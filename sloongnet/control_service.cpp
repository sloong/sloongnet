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
#include "SQLiteEx.h"
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

	m_pAllConfig->Initialize("configuation.db", uuid);
	auto config_str = m_pAllConfig->GetConfig(uuid);
	if (config_str.length() == 0 || !config->ParseFromString(config_str))
	{
		// If parse config error, run with default config.
		cout <<  "Parser server config error. run with default setting." << endl;
		ResetControlConfig(config.get());
	}
	return CSloongBaseService::Initialize(config);
}

void Sloong::SloongControlService::AfterInit()
{
	m_pControl->Add(DATA_ITEM::ServerConfiguation, &m_oConfig);

	m_pNetwork->RegisterMessageProcesser(std::bind(&SloongControlService::MessagePackageProcesser, this, std::placeholders::_1));

}

void Sloong::SloongControlService::ResetControlConfig(GLOBAL_CONFIG* config)
{
	config->set_logpath("/var/log/sloong");
	config->set_loglevel(LOGLEVEL::Info);
	config->set_debugmode(false);
	config->set_mqthreadquantity(1);
	config->set_enablessl(false);
	config->set_epollthreadquantity(1);
	config->set_listenport(8002);
}

void Sloong::SloongControlService::MessagePackageProcesser(SmartPackage pack)
{
	auto msgPack = pack->GetRecvPackage();
	switch( msgPack->function() )
	{
		case MessageFunction::GetServerConfig:
		{
			auto sender = msgPack->senderuuid();
			string config = "";
			if( sender.size() == 0 )
			{
				// TODO: 这里需要根据情况增加一个ip显示。
				m_pLog->Verbos(CUniversal::Format("New module[IP:%s] add to system. wait configuation. ","unspport"));
			}
			else
			{
				m_pLog->Verbos(CUniversal::Format("Porcess [GetServerConfig] request: sender[%d]",sender));
				config = m_pAllConfig->GetConfig(sender);
				if(config == "" )
				{
					// Error
					m_pLog->Verbos(CUniversal::Format("Module[IP:%s|UUID:%s] is no registe in system. wait admin process.","unspport",sender));
					
				}
			}
			pack->ResponsePackage("",config);
		}break;
	}
	
	auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(pack->GetSocketID());
	response_event->SetDataPackage(pack);
	m_pControl->CallMessage(response_event);
}
