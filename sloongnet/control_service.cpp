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
	if( true ) // TODO: Check and load uuid from file
	{
		uuid = CUtility::GenUUID();
		// TODO: Save uuid to file
	}

	m_pAllConfig->Initialize("configuation.db",uuid);
	if(!config->ParseFromString(m_pAllConfig->GetConfig(uuid)))
	{
		// If parse config error, run with default config.
		cout <<  "Parser server general config error. run with default setting." << endl;
		ResetControlConfig();
	}
	return CSloongBaseService::Initialize(config);
}

void Sloong::SloongControlService::AfterInit()
{
	m_pControl->Add(DATA_ITEM::ServerConfiguation, &m_oConfig);

	m_pNetwork->RegisterMessageProcesser(std::bind(&SloongControlService::MessagePackageProcesser, this, std::placeholders::_1));

}

void Sloong::SloongControlService::ResetControlConfig()
{
	
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
