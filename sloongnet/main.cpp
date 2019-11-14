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

string RegisteToControl(SmartConnect con, string uuid)
{
	auto req = make_shared<DataPackage>();
	req->set_function(Functions::RegisteServer);
	req->set_receiver(Protocol::ModuleType::Control);
	req->set_sender(uuid);

	CDataTransPackage dataPackage;
	dataPackage.Initialize(con);
	dataPackage.RequestPackage(req);
	NetworkResult result = dataPackage.SendPackage();
	if (result != NetworkResult::Succeed)
		throw string("Send get config request error.");
	result = dataPackage.RecvPackage(0);
	if (result != NetworkResult::Succeed)
		throw string("Receive get config result error.");
	auto get_config_response_buf = dataPackage.GetRecvPackage();
	if (!get_config_response_buf)
		throw string("Parse the get config response data error.");

	return get_config_response_buf->content();
}

string GetConfigFromControl(SmartConnect con,string uuid)
{
	auto get_config_request_buf = make_shared<DataPackage>();
	get_config_request_buf->set_function(Functions::GetServerConfig);
	get_config_request_buf->set_receiver(Protocol::ModuleType::Control);
	get_config_request_buf->set_sender(uuid);

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

	return get_config_response_buf->extend();
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
		string uuid;
		fstream_ex::read_all("uuid.dat", uuid);
		string server_uuid = RegisteToControl(con, uuid);
		if (uuid != server_uuid)
		{
			fstream_ex::write_all("uuid.dat", server_uuid);
			uuid = server_uuid;
		}

		do
		{
			cout << "Start get configuation." << endl;
			auto serverConfig = GetConfigFromControl(con,uuid);
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
