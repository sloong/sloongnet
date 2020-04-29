/*
 * @Author: WCB
 * @LastEditors: WCB
 * @Description: Control center service 
 * @Date: 2019-04-14 14:41:59
 * @LastEditTime: 2020-04-29 10:32:49
 */

#include "control_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "fstream_ex.hpp"


using namespace Sloong::Events;

unique_ptr<SloongControlService> Sloong::SloongControlService::Instance = nullptr;

extern "C" CResult MessagePackageProcesser(void* pEnv, CDataTransPackage* pack)
{
	auto pServer = TYPE_TRANS<CServerManage*>(pEnv);
	if( pServer)
		return pServer->ProcessHandler(pack);
	else
		return CResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongControlService::Instance = make_unique<SloongControlService>();
	return SloongControlService::Instance->Initialization(confiog);
}

extern "C" CResult ModuleInitialized(IControl* iC){
	return SloongControlService::Instance->Initialized(iC);
}

extern "C" CResult CreateProcessEnvironment(void** out_env)
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
CResult SloongControlService::Initialization(GLOBAL_CONFIG* config)
{
	if( config == nullptr )
	{
		return CResult::Make_Error("Config object is nullptr.");
	}
	auto res = m_pServer->Initialize(nullptr);
	if (res.IsFialed())
	{
		return CResult::Make_Error(CUniversal::Format("Init server manage fialed. error message:%s",res.Message()));
	}

	auto config_str = res.Message();
	// Here, this port is came from COMMAND LINE. 
	// So we need save it before parse other setting.
	auto port = config->listenport();
	if (config_str.length() == 0 || !config->ParseFromString(config_str))
	{
		// If parse config error, run with default config.
		cout <<  "Parser server config error. run with default setting." << endl;
		ResetControlConfig(config);
		res = m_pServer->ResetManagerTemplate(config);
		if( res.IsFialed())
		{
			cout << "Save defualt template error. message:" << res.Message() << endl;
			return res;
		}
	}
	// When config parse done, revert the port setting. Because we always use the command line port.
	config->set_listenport(port);
	
	return CResult::Succeed();
}

CResult SloongControlService::Initialized(IControl* iC)
{
	m_pControl = iC;
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	m_pLog = IData::GetLog();
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongControlService::OnSocketClose, this, std::placeholders::_1));
	return CResult::Succeed();
}

void Sloong::SloongControlService::ResetControlConfig(GLOBAL_CONFIG* config)
{
	config->set_logpath("/data/log");
	config->set_loglevel(Protocol::LogLevel::Info);
	config->set_debugmode(false);
	config->set_mqthreadquantity(1);
	config->set_enablessl(false);
	config->set_epollthreadquantity(1);
	config->set_processthreadquantity(1);
	config->set_receivetime(3);
}


void Sloong::SloongControlService::OnSocketClose(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto sock = net_evt->GetSocketID();
	for( auto item : m_listServerManage ) item->OnSocketClosed(sock);
}

inline CResult Sloong::SloongControlService::CreateProcessEnvironmentHandler(void** out_env)
{
	auto item = make_shared<CServerManage>();
	auto res = item->Initialize(m_pControl);
	if (res.IsFialed())
		return res;
	m_listServerManage.push_back(item);
	(*out_env) = item.get();
	return CResult::Succeed();
}