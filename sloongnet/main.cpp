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


bool ReadAllText(string path, string& out)
{
	FILE* fp = NULL;

	fp = fopen(path.c_str(), "r");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long len = ftell(fp);
		rewind(fp);    //指针复位
		char* buf = new char[len];
		memset(buf, 0, len);
		fread(buf, 1, len, fp);
		fclose(fp);
		out = string(buf, len);
		SAFE_DELETE_ARR(buf);
		return true;
	}
	else
	{
		return false;
	}
}



string GetConfigFromControl(SmartConnect con)
{
	auto get_config_request_buf = make_shared<MessagePackage>();
	get_config_request_buf->set_function(MessageFunction::GetServerConfig);
	get_config_request_buf->set_receiver(Protocol::ModuleType::Control);
	get_config_request_buf->set_type(Protocol::MsgTypes::Request);
	string uuid;
	if (fstream_ex::read_all("uuid.dat", uuid))
		get_config_request_buf->set_senderuuid(uuid.c_str());

	CDataTransPackage dataPackage;
	dataPackage.Initialize(con);
	dataPackage.RequestPackage(get_config_request_buf);
	NetworkResult result = dataPackage.SendPackage();
	if (result != NetworkResult::Succeed)
		throw string("Send get config request error.");
	result = dataPackage.RecvPackage(0);
	if (result != NetworkResult::Succeed)
		throw string("Receive get config result error.");
	auto get_config_response_buf = dataPackage.GetRecvPackage();
	if (!get_config_response_buf)
		throw string("Parse the get config response data error.");

	return get_config_response_buf->extenddata();
}

void PrientHelp()
{
	cout << "param: address:port" << endl;
}


unique_ptr<GLOBAL_CONFIG> Initialize(int argc, char** args)
{
	// 这里参数有两种形式。
	// 1 只有端口。 此时为控制中心模式
	// 2 地址：端口。此时为非控制中心模式
	if (argc != 2)
	{
		PrientHelp();
		return nullptr;
	}

	vector<string> addr = CUniversal::split(args[1], ":");
	
	auto config = make_unique<GLOBAL_CONFIG>();
	config->set_type(ModuleType::Unconfigured);

	if (addr.size() == 1)
	{
		int port = atoi(args[1]);
		if (port == 0)
		{
			cout << "Convert [" << args[1] << "] to int port fialed." << endl;
			return nullptr;
		}
		config->set_type(ModuleType::Control);
		config->set_listenport(port);
	}
	else
	{
		auto con = make_shared<EasyConnect>();
		con->Initialize(args[1], nullptr);
		if (!con->Connect())
		{
			cout << "Connect to control fialed." << endl;
			return nullptr;
		}
		cout << "Connect to control succeed." << endl;
		do
		{
			cout << "Start get configuation." << endl;
			auto serverConfig = GetConfigFromControl(con);
			if (serverConfig.size() == 0)
			{
				cout << "Control no return config infomation. wait 500ms and retry." << endl;
				sleep(500);
				continue;
			}
			if (!config->ParseFromString(serverConfig))
			{
				cout << "Parse the config struct error. please check." << endl;
				return nullptr;
			}
		} while (config->type() != ModuleType::Unconfigured);
		cout << "Get configuation succeed." << endl;
	}
	return config;
}


int main(int argc, char** args)
{
	try
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

		Sloong::CSloongBaseService::g_pAppService->Run();
		return 0;
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. " << endl;
		CUtility::write_call_stack();
	}
}
