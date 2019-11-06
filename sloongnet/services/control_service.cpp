/*
 * @Author: WCB
 * @LastEditors: WCB
 * @Description: Control center service 
 * @Date: 2019-04-14 14:41:59
 * @LastEditTime: 2019-11-06 16:38:28
 */

#include "control_service.h"
#include "NetworkHub.h"
#include "ControlHub.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "DataTransPackage.h"
#include "configuation.h"
#include "SQLiteEx.h"
#include "version.h"
using namespace Sloong::Events;

int main(int argc, char **args)
{
	try
	{
		Sloong::CSloongBaseService::g_pAppService = make_unique<SloongControlService>();

		auto res = Sloong::CSloongBaseService::g_pAppService->Initialize(argc, args);
		if (res.IsSucceed()){
			Sloong::CSloongBaseService::g_pAppService->Run();
			return 0;
		}
		else{
			cout << "Initialize error. message: " << res.Message() << endl;
			return -1;
		}
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. " << endl;
		CUtility::write_call_stack();
	}
}


void PrintVersion()
{
	cout << PRODUCT_TEXT << endl;
	cout << VERSION_TEXT << endl;
	cout << COPYRIGHT_TEXT << endl;
}

void SloongControlService::PrientHelp()
{
	cout << "param: listen port" << endl;
}

/**
 * @Remarks: If have errors, the error message will print to standard output device.
 * @Params: CMD line. from System.
 * @Return: return true if no error, service can continue to run.
 * 			return false if error. service must exit. 
 */
CResult SloongControlService::Initialize(int argc, char **args)
{
	int port = atoi(args[1]);
	if (port == 0)
	{
		cout << "Convert [" << args[1] << "] to int port fialed." << endl;
		return CResult(false);
	}
	string uuid;
	if( true ) // TODO: Check and load uuid from file
	{
		uuid = CUtility::GenUUID();
		// TODO: Save uuid to file
	}

	m_pAllConfig->Initialize("configuation.db",uuid);
	if(!m_oServerConfig.ParseFromString(m_pAllConfig->GetConfig(uuid)))
	{
		// If parse config error, run with default config.
		cout <<  "Parser server general config error. run with default setting." << endl;
		ResetControlConfig();
	}
		
	auto res = CSloongBaseService::Initialize(argc,args);
	if( !res.IsSucceed())
		return res;

	m_pControl->Add(DATA_ITEM::ServerConfiguation , &m_oConfig);
	
	m_pNetwork->RegisterMessageProcesser(std::bind(&SloongControlService::MessagePackageProcesser, this, std::placeholders::_1));
	
	return CResult::Succeed;
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
