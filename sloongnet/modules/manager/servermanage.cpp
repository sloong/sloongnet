#include "servermanage.h"

#include "utility.h"
#include "NetworkEvent.hpp"
#include "univ/Base64.h"
#include <jsoncpp/json/json.h>
using namespace Sloong::Events;


CResult Sloong::CServerManage::Initialize( int template_id )
{
	auto res = m_pAllConfig->Initialize("/data/configuation.db");
	if (res.IsFialed()) return res;

	// Initialize template list
	auto list = m_pAllConfig->GetTemplateList();
	for (auto item : list)
	{
		TemplateItem addItem(item);
		m_oTemplateList[addItem.ID] = addItem;
	}

	res = m_pAllConfig->GetTemplate(template_id);
	if (res.IsFialed())
		return CResult::Succeed();
	else
		return res;
}


int Sloong::CServerManage::SearchNeedCreateTemplate()
{
	for (auto item : m_oTemplateList)
	{
		if (item.second.Replicas == 0)
			continue;

		if (item.second.Created.size() == item.second.Replicas)
			continue;

		if (item.second.Created.size() == 0)
			return item.first;
	}

	for (auto item : m_oTemplateList)
	{
		if (item.second.Replicas == 0)
			continue;

		if (item.second.Created.size() == item.second.Replicas)
			continue;

		if (item.second.Created.size() < item.second.Replicas)
			return item.first;
	}
	return -1;
}


bool Sloong::CServerManage::RegisterServerHandler(Functions func, string sender, CDataTransPackage* pack)
{
	if( sender.length() == 0 )
	{
		sender = CUtility::GenUUID();
	}

	auto sender_info = m_oServerList.try_get(sender);
	if( sender_info == nullptr)
	{
		ServerItem item;
		item.Address = pack->GetSocketIP();
		item.Port = pack->GetSocketPort();
		item.UUID = sender;
		m_oServerList[sender] = item;
		m_pLog->Verbos(CUniversal::Format("Module[%s:%d] regist to system. Allocating uuid [%s].", item.Address, item.Port, item.UUID));
		sender_info = m_oServerList.try_get(sender);
	}

	if( sender_info == nullptr)
	{
		pack->ResponsePackage(ResultType::Error,"Add server info to ServerList fialed.");
		return true;
	}
	
	// Get tamplate list
	auto index = SearchNeedCreateTemplate();
	if (index == -1)
	{
		sender_info->Type = ModuleType::Unconfigured;
		sender_info->Template_ID = -1;
	}
	else
	{
		auto tpl = m_oTemplateList[index];
		sender_info->Type = tpl.Type;
		sender_info->Template_ID = tpl.ID;
		
		m_pLog->Verbos(CUniversal::Format("Allocating module[%s] Type to [%s]", sender_info->UUID, ModuleType_Name(sender_info->Type)));
	}

	pack->ResponsePackage(sender_info->UUID,sender_info->Template_ID==-1?"": CBase64::Decode(m_oTemplateList[sender_info->Template_ID].Configuation));

	return true;
}

/*
Response Format:
{
  "ConfigTemplateList": [
	{
	  "ID": "",
	  "Name": ""
	}
  ]
}
*/
bool Sloong::CServerManage::GetTemplateListHandler(Functions func, string sender, CDataTransPackage* pack)
{
	Json::Value list;
	for (auto& i : m_oTemplateList)
	{
		Json::Value item;
		item["ID"] = i.second.ID;
		item["Name"] = i.second.Name;
		item["Replicas"] = i.second.Replicas;
		item["Created"] = (int)i.second.Created.size();
		item["Note"] = i.second.Note;
		item["Configuation"] = i.second.Configuation;
		list.append(item);
	}
	Json::Value root;
	root["ConfigTemplateList"] = list;
	pack->ResponsePackage(root.toStyledString(),"");

	return true;
}


bool Sloong::CServerManage::SetTemplateConfigHandler(Functions func, string sender, CDataTransPackage* pack)
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
	if (root["ID"].isNull() || !m_oTemplateList.exist(root["ID"].asInt()))
	{
		pack->ResponsePackage(ResultType::Error, "Check the templeate ID error, please check.");
		return true;
	}
	
	auto info = m_oTemplateList[root["ID"].asInt()].ToTemplateInfo();
	if (!root["Name"].isNull())
		info.name = root["Name"].asString();

	if (!root["Note"].isNull())
		info.note = root["Note"].asString();

	if (!root["Replcas"].isNull())
		info.replicas = root["Replcas"].asInt();

	if (!root["Configuation"].isNull())
		info.configuation = root["Configuation"].asString();

	auto res = m_pAllConfig->SetTemplate(info.id, info);
	if (res.IsSucceed())
	{
		pack->ResponsePackage(ResultType::Succeed, "success");
	}
	else
	{
		pack->ResponsePackage(ResultType::Error, res.Message());
	}

	return true;
}

/*
Response Format: 
{
  "ServerList": [
    {
      "UUID": "",
	  "Type":"",
      "TemplateID": ""
    }
  ]
}
*/
bool Sloong::CServerManage::GetServerListHandler(Functions func, string sender, CDataTransPackage* pack)
{
	Json::Value list;
	for (auto& i : m_oServerList)
	{
		Json::Value item;
		item["UUID"] = i.first;
		item['Type'] = i.second.Type;
		item["TemplateID"] = i.second.Template_ID;
		list.append(item);
	}
	Json::Value root;
	root["ServerList"] = list;
	pack->ResponsePackage(root.toStyledString(), "");
}
