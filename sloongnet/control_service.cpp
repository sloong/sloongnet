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
		CServerItem item;
		item.Address = pack->GetSocketIP();
		item.Port = pack->GetSocketPort();
		if (sender.size() == 0){
			sender = CUtility::GenUUID();
			m_pLog->Verbos(CUniversal::Format("New module[%s:%d] regist to system. Allocating uuid [%s].", item.Address,item.Port, sender));
		}
		else if (m_pAllConfig->GetConfig(sender).length() == 0){
			m_pLog->Verbos(CUniversal::Format("Server [%s:%d][%s] is online. but no configuation info.", item.Address, item.Port, sender));
		}
		else {
			item.Configured = true;
		}

		m_oServerList[sender] = item;
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
		auto item = m_oServerList.try_get(sender);
		if (item == nullptr) {
			string errmsg = CUniversal::Format("Module [%s] is no registe.", sender);
			m_pLog->Warn(errmsg);
			pack->ResponsePackage( ResultType::Error, errmsg);
		}
		else
		{
			string config = m_pAllConfig->GetConfig(sender);
			if (config == "")
			{
				m_pLog->Verbos(CUniversal::Format("Module[%s:%d|UUID:%s] is no configued. wait admin process.", item->Address, item->Port, sender));
				m_oWaitConfig.SerializeToString(&config);
			}
			else
			{
				item->Configured = true;
			}
			pack->ResponsePackage("", config);
		}
	}break;
	case Functions::GetWaitConfigList:
	{
		string list_str = "";
		Json::Value root;
		Json::Value list;
		for (auto& i : m_oServerList) {
			if (i.second.Configured)
				continue;
			Json::Value item;
			item["UUID"] = i.first;
			item["IP"] = i.second.Address;
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
