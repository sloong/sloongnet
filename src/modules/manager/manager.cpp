/*
 * @Author: WCB
 * @LastEditors: WCB
 * @Description: Control center service 
 * @Date: 2019-04-14 14:41:59
 * @LastEditTime: 2020-05-14 14:14:01
 */

#include "manager.h"
#include "utility.h"
#include "events/ConnectionBreak.hpp"
#include "fstream_ex.hpp"

using namespace Sloong::Events;

unique_ptr<SloongControlService> Sloong::SloongControlService::Instance = nullptr;
string Sloong::SloongControlService::g_strDBFilePath = "./configuation.db";

extern "C" PackageResult RequestPackageProcesser(void *pEnv, Package *pack)
{
	auto pServer = STATIC_TRANS<CServerManage *>(pEnv);
	if (pServer)
		return pServer->ProcessHandler(pack);
	else
		return PackageResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" PackageResult ResponsePackageProcesser(void *pEnv, Package *pack)
{
	auto pServer = STATIC_TRANS<CServerManage *>(pEnv);
	if (pServer)
		return pServer->ProcessHandler(pack);
	else
		return PackageResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" CResult EventPackageProcesser(Package *pack)
{
	SloongControlService::Instance->EventPackageProcesser(pack);
	return CResult::Succeed;
}

extern "C" CResult PrepareInitialize(GLOBAL_CONFIG *config)
{
	return SloongControlService::PrepareInitialize(config);
}

extern "C" CResult ModuleInitialization(IControl *ic)
{
	SloongControlService::Instance = make_unique<SloongControlService>();
	return SloongControlService::Instance->Initialization(ic);
}

extern "C" CResult ModuleInitialized()
{
	return SloongControlService::Instance->Initialized();
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
	return SloongControlService::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult SloongControlService::PrepareInitialize(GLOBAL_CONFIG *config)
{
	auto path = std::getenv("DB_FILE_PATH");
	if (path != nullptr)
	{
		g_strDBFilePath = path;
	}
	auto res = CServerManage::LoadManagerConfig(g_strDBFilePath);
	if (res.GetResult() != ResultType::Warning && res.GetResult() != ResultType::Succeed)
		return res;

	auto config_str = res.GetMessage();

	// Here, this port is came from COMMAND LINE.
	// So we need save it before parse other setting.
	auto port = config->listenport();
	if (config_str.length() == 0 || !config->ParseFromString(config_str))
	{
		// If parse config error, run with default config.
		cout << "Parser server config error. run with default setting." << endl;
		ResetControlConfig(config);
		res = CServerManage::ResetManagerTemplate(config);
		if (res.IsFialed())
		{
			cout << "Save defualt template error. message:" << res.GetMessage() << endl;
			return res;
		}
	}
	// When config parse done, revert the port setting. Because we always use the command line port.
	config->set_listenport(port);
	

	return CResult::Succeed;
}

/**
 * @Description: If have errors, the error message will print to standard output device.
 * @Params: CMD line. from System.
 * @Return: return true if no error, service can continue to run.
 * 			return false if error. service must exit. 
 * 
 * 
 */
CResult SloongControlService::Initialization(IControl *iC)
{
	if (iC == nullptr)
	{
		return CResult::Make_Error("Config object is nullptr.");
	}

	IObject::Initialize(iC);
	IData::Initialize(iC);

	m_pConfig = IData::GetGlobalConfig();

	return CResult::Succeed;
}

CResult SloongControlService::Initialized()
{
	m_iC->RegisterEventHandler(EVENT_TYPE::ConnectionBreaked, std::bind(&SloongControlService::OnConnectionBreaked, this, std::placeholders::_1));
	return CResult::Succeed;
}

void Sloong::SloongControlService::ResetControlConfig(GLOBAL_CONFIG *config)
{
	config->set_logpath("./log");
	config->set_loglevel(Core::LogLevel::Info);
	config->set_mqthreadquantity(1);
	config->set_enablessl(false);
	config->set_epollthreadquantity(1);
	config->set_processthreadquantity(1);
	config->set_receivetime(3);
}

void Sloong::SloongControlService::OnConnectionBreaked(SharedEvent e)
{
	auto event = DYNAMIC_TRANS<ConnectionBreakedEvent *>(e.get());
	auto id = event->GetSessionID();
	for (auto &item : m_listServerManage)
		item->OnSocketClosed(id);
}

inline CResult Sloong::SloongControlService::CreateProcessEnvironmentHandler(void **out_env)
{
	auto item = make_unique<CServerManage>();
	auto res = item->Initialize(m_iC, g_strDBFilePath);
	if (res.IsFialed())
		return res;
	(*out_env) = item.get();
	m_listServerManage.push_back(std::move(item));
	return CResult::Succeed;
}

void Sloong::SloongControlService::EventPackageProcesser(Package *pack)
{
	m_pLog->Info("EventPackageProcesser is called.");
}