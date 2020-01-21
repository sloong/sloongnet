#include "servermanage.h"

#include "utility.h"
#include "NetworkEvent.hpp"
#include "univ/Base64.h"
#include <jsoncpp/json/json.h>
using namespace Sloong::Events;


CResult Sloong::CServerManage::Initialize(string uuid, string& out_config)
{
	m_pAllConfig->Initialize("/data/configuation.db", uuid);
	auto config_str = m_pAllConfig->GetConfig(uuid);
	out_config = config_str;
	return CResult::Succeed;
}

bool Sloong::CServerManage::RegisterServerHandler(Functions func, string sender, SmartPackage pack)
{
		CServerItem item;
		item.Address = pack->GetSocketIP();
		item.Port = pack->GetSocketPort();
		auto content = pack->GetRecvPackage()->content();
		ModuleType type = Unconfigured;
		/// check module type first.
		if (!ModuleType_Parse(content, &type))
		{
			m_pLog->Error(CUniversal::Format("Parser module with Server [%s:%d] error.", item.Address, item.Port));
			pack->ResponsePackage(ResultType::Error, CUniversal::Format("Parser module error [%s].", content));
			return true;
		}

		// check uuid
		if (sender.size() == 0) {
			sender = CUtility::GenUUID();
			m_pLog->Verbos(CUniversal::Format("New module[%s:%d] regist to system. Allocating uuid [%s].", item.Address, item.Port, sender));
		}
		item.Type = type;
		m_oServerTypeList[type].unique_insert(sender);
		m_oServerList[sender] = item;
		pack->ResponsePackage(sender,"");

		return true;
}


bool Sloong::CServerManage::GetConfigTemplateListHandler(Functions func, string sender, SmartPackage pack)
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
	pack->ResponsePackage(root.toStyledString(),"");

	return true;
}


bool Sloong::CServerManage::SetServerConfigHandler(Functions func, string sender, SmartPackage pack)
{
	auto msgPack = pack->GetRecvPackage();
	auto target = msgPack->content();
	auto res = m_pAllConfig->SetConfig(target, msgPack->extend());
	if (res.IsSucceed())
	{
		pack->ResponsePackage("Succeed", "");
	}
	else
	{
		pack->ResponsePackage(ResultType::Error, res.Message());
	}

	return true;
}

bool Sloong::CServerManage::SetServerConfigTemplateHandler(Functions func, string sender, SmartPackage pack)
{
	auto msgPack = pack->GetRecvPackage();
	auto target = msgPack->content();
	auto res = m_pAllConfig->SetTemplate(atoi(target.c_str()), msgPack->extend());
	if (res.IsSucceed())
	{
		pack->ResponsePackage("Succeed", "");
	}
	else
	{
		pack->ResponsePackage(ResultType::Error, res.Message());
	}

	return true;
}

bool Sloong::CServerManage::GetServerConfigHandler(Functions func, string sender, SmartPackage pack)
{
	auto item = m_oServerList.try_get(sender);
	if (item == nullptr) {
		string errmsg = CUniversal::Format("Module [%s] is no registe.", sender);
		m_pLog->Warn(errmsg);
		pack->ResponsePackage(ResultType::Error, errmsg);
		return true;
	}
	else
	{
		// Try get server config with uuid first
		string config = m_pAllConfig->GetConfig(sender);
		if (config == "")
		{
			m_pLog->Verbos(CUniversal::Format("Module[%s:%d|UUID:%s] is no special configued. return global module config.", item->Address, item->Port, sender));
			config = m_pAllConfig->GetTemplate(item->Type);
			if (config == "")
			{
				string errmsg = CUniversal::Format("Get global module[%s:%d] config error.", ModuleType_Name(item->Type), item->Type);
				m_pLog->Error(errmsg);
				pack->ResponsePackage(ResultType::Error, errmsg);
				return true;
			}
		}
		pack->ResponsePackage("", config);

		return true;
	}

}

/*
Format:
{
	"ServerUUID":"",
	"TemplateID" : "",
}
*/
bool Sloong::CServerManage::SetServerToTemplate(Functions func, string sender, SmartPackage pack)
{
	auto msgPack = pack->GetRecvPackage();
	auto jreq = msgPack->content();
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(jreq, root))
	{
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("Parser json[% s] error.", jreq));
		return true;
	}
	if (root["TemplateID"].isNull())
	{
		pack->ResponsePackage(ResultType::Error, "JSON no have 'TemplateID' element.");
		return true;
	}

	
	auto res = m_pAllConfig->SetConfigToTemplate(sender, root["key"].asInt());
	if( res.IsFialed())
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("Parser json[% s] error.", jreq));
	pack->ResponsePackage(root.toStyledString(),"");

	return true;
}