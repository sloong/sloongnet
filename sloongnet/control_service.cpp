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
#include <jsoncpp/json/json.h>
#include "univ/Base64.h"
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

	m_pAllConfig->Initialize("/data/configuation.db", uuid);
	InitWaitConfig();
	auto config_str = m_pAllConfig->GetConfig(uuid);
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

	m_pNetwork->RegisterMessageProcesser(std::bind(&SloongControlService::MessagePackageProcesser, this, std::placeholders::_1));

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

void Sloong::SloongControlService::InitWaitConfig()
{
	m_oWaitConfig.set_type(ModuleType::Unconfigured);
}

void Sloong::SloongControlService::MessagePackageProcesser(SmartPackage pack)
{
	auto msgPack = pack->GetRecvPackage();
	auto sender = msgPack->sender();
	switch( msgPack->function() )
	{
	case Functions::RegisteServer:
	{
		if (sender.size() == 0)
		{
			// TODO: 这里需要根据情况增加一个ip显示。
			string ip = "unspport";
			m_pLog->Verbos(CUniversal::Format("New module[IP:%s] add to system. wait configuation. ", ip));
			sender = CUtility::GenUUID();
			m_oWaitConfigList[sender] =ip;
		}
		// Add to server list
		m_oServerList[sender] = "";
		pack->ResponsePackage(sender);
	}break;
	case Functions::GetConfigTemplateList: 
	{
		auto tpl_list = m_pAllConfig->GetTemplateList();
		Json::Value list;
		for (auto& i : tpl_list)
		{
			Json::Value item;
			item["ID"] = i.first;
			item["Config"] = CBase64::Encode(i.second);
			list.append(item);
		}
		Json::Value root;
		root["ConfigTemplateList"] = list;
		pack->ResponsePackage(root.toStyledString());
	}break;
	case Functions::SetServerConfig:
	{
		auto target = msgPack->content();
		auto res = m_pAllConfig->SaveConfig(target, msgPack->extend());
		if (res.IsSucceed())
		{
			m_oWaitConfigList.erase(target);
			pack->ResponsePackage("Succeed", "");
		}
		else
		{
			pack->ResponsePackage(ResultType::Error, res.Message());
		}
	}break;
	case Functions::SetServerConfigTemplate:
	{
		auto target = msgPack->content();
		auto res = m_pAllConfig->SaveTemplate(target, msgPack->extend());
		if(res.IsSucceed())
		{
			pack->ResponsePackage("Succeed", "");
		}
		else
		{
			pack->ResponsePackage(ResultType::Error, res.Message());
		}
	}break;
	case Functions::GetServerConfig:
	{
		m_pLog->Verbos(CUniversal::Format("Porcess [GetServerConfig] request: sender[%d]", sender));
		string config = m_pAllConfig->GetConfig(sender);
		if (config == "")
		{
			// Error
			string ip = "unspport";
			m_pLog->Verbos(CUniversal::Format("Module[IP:%s|UUID:%s] is no registe in system. wait admin process.", ip,sender));
			m_oWaitConfig.SerializeToString(&config);
			if (m_oWaitConfigList.end() == m_oWaitConfigList.find(sender))
				m_oWaitConfigList[sender]= ip;
		}
		pack->ResponsePackage("",config);
	}break;
	case Functions::GetWaitConfigList:
	{
		string list_str = "";
		Json::Value root;
		Json::Value list;
		for (auto& i : m_oWaitConfigList) {
			Json::Value item;
			item["UUID"] = i.first;
			item["IP"] = i.second;
			list.append(item);
		}
		root["WaitConfigList"] = list;
		pack->ResponsePackage(root.toStyledString());
	}break;
	case Functions::RestartService:
	{
		m_oExitResult = ResultEnum::Retry;
		m_oExitSync.notify_all();
		return;
	}break;
	}
	
	auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(pack->GetSocketID());
	response_event->SetDataPackage(pack);
	m_pControl->SendMessage(response_event);
}
