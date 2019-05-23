/*
 * @Author: WCB
 * @LastEditors: WCB
 * @Description: Control center service 
 * @Date: 2019-04-14 14:41:59
 * @LastEditTime: 2019-05-23 10:25:15
 */

#include "control_service.h"
#include "NetworkHub.h"
#include "ControlHub.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "DataTransPackage.h"
#include "configuation.h"
#include "SQLiteEx.h"
#include "config.pb.h"
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
	m_pAllConfig->Initialize("system");
	m_pAllConfig->LoadAll();
	string config;
	m_pAllConfig->m_oServerConfigList[ModuleType::ControlCenter].SerializeToString(&config);
	if(!m_oServerConfig.ParseFromString(config))
		return CResult(false, "Parser server general config error.");
	
	auto res = CSloongBaseService::Initialize(argc,args);
	if( !res.IsSucceed())
		return res;
	
	m_pNetwork->RegisterMessageProcesser(std::bind(&SloongControlService::MessagePackageProcesser, this, std::placeholders::_1));
	
	return CResult::Succeed;
}

void Sloong::SloongControlService::MessagePackageProcesser(SmartPackage pack)
{
	auto msgPack = pack->GetRecvPackage();
	string config;
	switch( msgPack->function() )
	{
		case MessageFunction::GetGeneralConfig:
		{
			m_pLog->Verbos(CUniversal::Format("Porcess [GetGerenalConfig] request: sender[%d]",msgPack->sender()));
			auto item = m_pAllConfig->m_oServerConfigList.find(msgPack->sender());
			if( item == m_pAllConfig->m_oServerConfigList.end() )
			{
				// Error
			}
			(*item).second.SerializeToString(&config);
		}break;
		case MessageFunction::GetSpecialConfig:
		{
			m_pLog->Verbos(CUniversal::Format("Porcess [GetSpecialConfig] request: sender[%d]",msgPack->sender()));
			switch(msgPack->sender())
			{
				case ModuleType::Proxy:
					m_pAllConfig->m_oProxyConfig.SerializeToString(&config);
					break;
				case ModuleType::Process:
					m_pAllConfig->m_oProcessConfig.SerializeToString(&config);
					break;
				case ModuleType::Firewall:
					m_pAllConfig->m_oFirewallConfig.SerializeToString(&config);
					break;
				case ModuleType::DataCenter:
					m_pAllConfig->m_oDataConfig.SerializeToString(&config);
					break;
				case ModuleType::DBCenter:
					m_pAllConfig->m_oDBConfig.SerializeToString(&config);
					break;
			}
		}break;
	}
	pack->ResponsePackage("",config.data(),config.length());

	auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(pack->GetSocketID());
	response_event->SetDataPackage(pack);
	m_pControl->CallMessage(response_event);
}
