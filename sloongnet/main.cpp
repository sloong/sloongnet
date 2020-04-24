#include "main.h"
#include "base_service.h"
#include "version.h"
#include "utility.h"
#include "fstream_ex.hpp"

void PrintVersion()
{
	cout << PRODUCT_TEXT << endl;
	cout << VERSION_TEXT << endl;
	cout << COPYRIGHT_TEXT << endl;
}


TResult<shared_ptr<DataPackage>> RegisteToControl(SmartConnect con, string uuid)
{
	auto req = make_shared<DataPackage>();
	req->set_function(Functions::ProcessMessage);
	req->set_sender(uuid);
	req->set_content("{\"Function\":\"RegisteWorkder\"}");

	CDataTransPackage dataPackage;
	dataPackage.Initialize(con);
	dataPackage.RequestPackage(req);
	ResultType result = dataPackage.SendPackage();
	if (result != ResultType::Succeed)
		return TResult<shared_ptr<DataPackage>>::Make_Error( "Send get config request error.");
	result = dataPackage.RecvPackage(0, 0);
	if (result != ResultType::Succeed)
		return TResult<shared_ptr<DataPackage>>::Make_Error("Receive get config result error.");
	auto get_config_response_buf = dataPackage.GetRecvPackage();
	if (!get_config_response_buf)
		return TResult<shared_ptr<DataPackage>>::Make_Error("Parse the get config response data error.");

	return TResult<shared_ptr<DataPackage>>::Make_OK(get_config_response_buf);
}


void PrientHelp()
{
	cout << "sloongnet [<type>] [<address:port>]" << endl;
	cout << "<type>: Manager|Worker" << endl;
}


unique_ptr<RunTimeData> Initialize(int argc, char** args)
{
	// 参数共有2个，类型和地址信息
	// 1 类型，为指定值
	// 2 地址信息：格式->地址：端口。如果类型为control，表示为监听地址
	if (argc != 3)
	{
		PrientHelp();
		return nullptr;
	}

	bool ManagerMode = true;
	if (strcasecmp(args[1], "Worker") == 0)
	{
		ManagerMode = false;
	}
	else if (strcasecmp(args[1], "Manager") == 0)
	{
		ManagerMode = true;
	}
	else
	{
		cout << "Parse module type error." << endl;
		PrientHelp();
		return nullptr;
	}

	
	vector<string> addr = CUniversal::split(args[2], ':');
	if (addr.size() != 2)
	{
		cout << "Address info format error. Format [addr]:[port]" << endl;
		return nullptr;
	}

	unique_ptr<RunTimeData> data = make_unique<RunTimeData>();
	if( ManagerMode )
	{
		int port = atoi(addr[1].c_str());
		if (port == 0)
		{
			cout << "Convert [" << args[1] << "] to int port fialed." << endl;
			return nullptr;
		}
		data->TemplateConfig.set_listenport(port);
		data->TemplateConfig.set_modulepath("./modules/");
		data->TemplateConfig.set_modulename("libmanager.so");
	}
	else
	{
		data->ManagerAddressPort = args[2];
		data->ManagerConnect = make_shared<EasyConnect>();
		data->ManagerConnect->Initialize(args[2], nullptr);
		if (!data->ManagerConnect->Connect())
		{
			cout << "Connect to control fialed." << endl;
			return nullptr;
		}
		cout << "Connect to control succeed. Start registe and get configuation." << endl;
		
		string uuid;
		string serverConfig;
		while(true)
		{
			auto res = RegisteToControl(data->ManagerConnect,uuid);
			if (res.IsFialed())
			{
				cout << res.Message() << endl;
				return nullptr;
			}
			

			auto response = res.ResultObject();
			if( response->result() == Protocol::ResultType::Retry ){
				cout << "Control return retry package. wait 500ms and retry." << endl;
				uuid = response->content();
				SLEEP(500);
				continue;
			}else if(response->result() == Protocol::ResultType::Succeed){
				serverConfig = response->content();
			}else{
				cout << "Control return an unexpected result." << Protocol::ResultType_Name(response->result()) << ".Message: " << response->content() << endl;
				return nullptr;
			}

			if (serverConfig.size() == 0)
			{
				cout << "Control no return config infomation. wait 500ms and retry." << endl;
				SLEEP(500);
				continue;
			}

			data->NodeUUID = uuid;
			Json::Reader reader;
			Json::Value jConfig;
			if( !reader.parse(serverConfig, jConfig) || !jConfig["TemplateID"].isInt() || !jConfig["Configuation"].isString())
			{
				cout << "Parse the config error. response data:" << serverConfig << endl;
				return nullptr;
			}
			
			if (!data->TemplateConfig.ParseFromString(CBase64::Decode(jConfig["Configuation"].asString()) ))
			{
				cout << "Parse the config struct error. please check." << endl;
				return nullptr;
			}
			data->TemplateID = jConfig["TemplateID"].asInt();
			break;
		};
		cout << "Get configuation succeed." << endl;
	}
	return data;
}


int main(int argc, char** args)
{
	try
	{
		CResult code = CResult::Succeed();
		do
		{
			auto data = Initialize(argc, args);
			if (!data) {
				cout << "Initialize error." << endl;
				return -1;
			}

			Sloong::CSloongBaseService::Instance = make_unique<Sloong::CSloongBaseService>();
			if (!Sloong::CSloongBaseService::Instance) {
				cout << "Create service object error." << endl;
				return -2;
			}

			auto res = Sloong::CSloongBaseService::Instance->Initialize(data);
			if (!res.IsSucceed()) {
				cout << "Initialize server error. Message: " << res.Message() << endl;
				return -3;
			}

			code = Sloong::CSloongBaseService::Instance->Run();
		} while (code.Result() == ResultType::Retry);
		return 0;
	}
	catch (string & msg)
	{
		cout << "exception happened, message:" << msg << endl;
	}
	catch (exception & exc)
	{
		cout << "exception happened, message:" << exc.what() << endl;
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. " << endl;
		CUtility::write_call_stack();
	}
}
