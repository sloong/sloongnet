#include "main.h"
#include "version.h"
#include "ServiceBuilder.h"
#include "utility.h"
#include "fstream_ex.hpp"

void PrintVersion()
{
	cout << PRODUCT_TEXT << endl;
	cout << VERSION_TEXT << endl;
	cout << COPYRIGHT_TEXT << endl;
}


TResult<shared_ptr<DataPackage>> SendPackage(SmartConnect con, shared_ptr<DataPackage> pack)
{
	CDataTransPackage dataPackage;
	dataPackage.Initialize(con);
	dataPackage.RequestPackage(pack);
	ResultType result = dataPackage.SendPackage();
	if (result != ResultType::Succeed)
		return TResult<shared_ptr<DataPackage>>::Make_Error( "Send get config request error.");
	result = dataPackage.RecvPackage(0, 0);
	if (result != ResultType::Succeed)
		return TResult<shared_ptr<DataPackage>>::Make_Error("Receive get config result error.");
	auto get_config_response_buf = dataPackage.GetRecvPackage();
	if (!get_config_response_buf)
		return TResult<shared_ptr<DataPackage>>::Make_Error("Parse the get config response data error.");
	if (get_config_response_buf->result() != ResultType::Succeed)
		return TResult<shared_ptr<DataPackage>>::Make_Error(get_config_response_buf->content());

	return TResult<shared_ptr<DataPackage>>::Make_OK(get_config_response_buf);
}

TResult<tuple<string,string>> RegisteToControl(SmartConnect con, const string& uuid)
{
	auto req = make_shared<DataPackage>();
	req->set_function(Functions::RegisteServer);
	req->set_receiver(Protocol::ModuleType::Control);
	req->set_sender(uuid);

	auto res = SendPackage(con, req);
	if (res.IsFialed())
		return TResult<tuple<string, string>>::Make_Error(res.Message());

	return TResult<tuple<string, string>>::Make_OK( tuple<string,string>(res.ResultObject()->content(), res.ResultObject()->extend()));
}


void PrientHelp()
{
	cout << "sloongnet [<type>] [<address:port>]" << endl;
	cout << "<type>: Manager|Worker" << endl;
}


unique_ptr<GLOBAL_CONFIG> Initialize(int argc, char** args)
{
	// 参数共有2个，类型和地址信息
	// 1 类型，为指定值
	// 2 地址信息：格式->地址：端口。如果类型为control，表示为监听地址
	if (argc != 3)
	{
		PrientHelp();
		return nullptr;
	}

	ModuleType type = Unconfigured;
	if (strcasecmp(args[1], "Worker") == 0)
	{
		type = ModuleType::Unconfigured;
	}
	else if (strcasecmp(args[1], "Manager") == 0)
	{
		type = ModuleType::Control;
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

	auto config = make_unique<GLOBAL_CONFIG>();
	config->set_type(type);

	if( type == ModuleType::Control)
	{
		int port = atoi(addr[1].c_str());
		if (port == 0)
		{
			cout << "Convert [" << args[1] << "] to int port fialed." << endl;
			return nullptr;
		}
		config->set_listenport(port);
	}
	else
	{
		auto con = make_shared<EasyConnect>();
		con->Initialize(args[2], nullptr);
		if (!con->Connect())
		{
			cout << "Connect to control fialed." << endl;
			return nullptr;
		}
		cout << "Connect to control succeed. Start registe and get configuation." << endl;

		string uuid;
		do
		{
			auto res = RegisteToControl(con,uuid);
			if (res.IsFialed())
			{
				cout << res.Message() << endl;
				return nullptr;
			}

			if( uuid.length() == 0 )
				uuid = std::get<0>(res.ResultObject());

			string serverConfig = std::get<1>(res.ResultObject());

			if (serverConfig.size() == 0)
			{
				cout << "Control no return config infomation. wait 500ms and retry." << endl;
				SLEEP(500);
				continue;
			}
			if (!config->ParseFromString(serverConfig))
			{
				cout << "Parse the config struct error. please check." << endl;
				return nullptr;
			}
		} while (config->type() == ModuleType::Unconfigured);
		cout << "Get configuation succeed." << endl;
	}
	return config;
}


int main(int argc, char** args)
{
	try
	{
		CResult code = CResult::Succeed();
		do
		{
			auto config = Initialize(argc, args);
			if (!config) {
				cout << "Initialize error." << endl;
				return -1;
			}

			Sloong::CSloongBaseService::g_pAppService = ServiceBuilder::Build(config.get());
			if (!Sloong::CSloongBaseService::g_pAppService) {
				cout << "Create service object error." << endl;
				return -2;
			}

			auto res = Sloong::CSloongBaseService::g_pAppService->Initialize(config);
			if (!res.IsSucceed()) {
				cout << "Initialize server error. Message: " << res.Message() << endl;
				return -3;
			}

			code = Sloong::CSloongBaseService::g_pAppService->Run();
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
