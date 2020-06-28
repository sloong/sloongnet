/*
 * @Author: WCB
 * @LastEditors: WCB
 * @Description: Control center service 
 * @Date: 2019-04-14 14:41:59
 * @LastEditTime: 2020-05-14 14:14:01
 */

#include "manager.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "fstream_ex.hpp"

using namespace Sloong::Events;

unique_ptr<SloongControlService> Sloong::SloongControlService::Instance = nullptr;

extern "C" CResult RequestPackageProcesser(void *pEnv, CDataTransPackage *pack)
{
	auto pServer = TYPE_TRANS<CServerManage *>(pEnv);
	if (pServer)
		return pServer->ProcessHandler(pack);
	else
		return CResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" CResult ResponsePackageProcesser(void *pEnv, CDataTransPackage *pack)
{
	auto pServer = TYPE_TRANS<CServerManage *>(pEnv);
	if (pServer)
		return pServer->ProcessHandler(pack);
	else
		return CResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" CResult EventPackageProcesser(CDataTransPackage *pack)
{
	SloongControlService::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialization(GLOBAL_CONFIG *config)
{
	SloongControlService::Instance = make_unique<SloongControlService>();
	return SloongControlService::Instance->Initialization(config);
}

extern "C" CResult ModuleInitialized(SOCKET sock, IControl *iC)
{
	return SloongControlService::Instance->Initialized(iC);
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
	return SloongControlService::Instance->CreateProcessEnvironmentHandler(out_env);
}

/**
 * @Remarks: If have errors, the error message will print to standard output device.
 * @Params: CMD line. from System.
 * @Return: return true if no error, service can continue to run.
 * 			return false if error. service must exit. 
 * 
 * 
 */
CResult SloongControlService::Initialization(GLOBAL_CONFIG *config)
{
	if (config == nullptr)
	{
		return CResult::Make_Error("Config object is nullptr.");
	}

	auto path = std::getenv("ManagerConfiguationDBFilePath");
	if(path != nullptr )
	{
		m_strDBFilePath = path;
	}
	auto res = CServerManage::LoadManagerConfig(m_strDBFilePath);
	if( res.GetResult() != ResultType::Warning && res.GetResult() != ResultType::Succeed )
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

	return CResult::Succeed();
}

CResult SloongControlService::Initialized(IControl *iC)
{
	m_pControl = iC;
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	m_pLog = IData::GetLog();
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongControlService::OnSocketClose, this, std::placeholders::_1));
	return CResult::Succeed();
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

void Sloong::SloongControlService::OnSocketClose(IEvent *event)
{
	auto net_evt = TYPE_TRANS<CNetworkEvent *>(event);
	auto sock = net_evt->GetSocketID();
	for (auto &item : m_listServerManage)
		item->OnSocketClosed(sock);
}

inline CResult Sloong::SloongControlService::CreateProcessEnvironmentHandler(void **out_env)
{
	auto item = make_unique<CServerManage>();
	auto res = item->Initialize(m_pControl,m_strDBFilePath);
	if (res.IsFialed())
		return res;
	(*out_env) = item.get();
	m_listServerManage.push_back(std::move(item));
	return CResult::Succeed();
}

void Sloong::SloongControlService::EventPackageProcesser(CDataTransPackage *pack)
{
	m_pLog->Info("EventPackageProcesser is called.");
}