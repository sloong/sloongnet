/*
 * @Author: WCB
 * @LastEditors: WCB
 * @Description: Control center service 
 * @Date: 2019-04-14 14:41:59
 * @LastEditTime: 2019-11-06 16:38:28
 */

#include "control_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "fstream_ex.hpp"

using namespace Sloong::Events;

/**
 * @Remarks: If have errors, the error message will print to standard output device.
 * @Params: CMD line. from System.
 * @Return: return true if no error, service can continue to run.
 * 			return false if error. service must exit. 
 */
CResult SloongControlService::Initialize(unique_ptr<GLOBAL_CONFIG>& config)
{
	string uuid;
	if (!fstream_ex::read_all("uuid.dat",uuid) || uuid.length() == 0)
	{
		uuid = CUtility::GenUUID();
		fstream_ex::write_all("uuid.dat", uuid);
	}
	string config_str;
	auto res = m_pServer->Initialize(uuid, config_str);
	if (res.IsFialed())
	{
		cout << "Init server manage fialed. error message:" << res.Message() << endl;
		return res;
	}
	
	auto port = config->listenport();
	if (config_str.length() == 0 || !config->ParseFromString(config_str))
	{
		// If parse config error, run with default config.
		cout <<  "Parser server config error. run with default setting." << endl;
		ResetControlConfig(config.get());
	}
	config->set_listenport(port);
	return CSloongBaseService::Initialize(config);
}

void Sloong::SloongControlService::AfterInit()
{
	m_pControl->Add(DATA_ITEM::ServerConfiguation, &m_oConfig);
	m_pServer->SetLog(m_pLog.get());
	RegistFunctionHandler(Functions::RegisteServer, std::bind(&CServerManage::RegisterServerHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	RegistFunctionHandler(Functions::GetAllConfigTemplate, std::bind(&CServerManage::GetConfigTemplateListHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	RegistFunctionHandler(Functions::GetServerConfig , std::bind(&CServerManage::GetServerConfigHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	RegistFunctionHandler(Functions::SetConfigTemplate, std::bind(&CServerManage::SetServerConfigTemplateHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	RegistFunctionHandler(Functions::SetServerConfig, std::bind(&CServerManage::SetServerConfigHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	RegistFunctionHandler(Functions::SetServerToTemplate, std::bind(&CServerManage::SetServerToTemplate, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
