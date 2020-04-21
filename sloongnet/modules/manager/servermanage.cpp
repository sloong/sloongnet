#include "servermanage.h"

#include "utility.h"
#include "NetworkEvent.hpp"
#include "univ/Base64.h"
#include <jsoncpp/json/json.h>
using namespace Sloong::Events;


CResult Sloong::CServerManage::Initialize( int template_id )
{
	m_listFuncHandler["AddTemplate"]=std::bind(&CServerManage::AddTemplateHandler, this, std::placeholders::_1,std::placeholders::_2);
	m_listFuncHandler["DeleteTemplate"]=std::bind(&CServerManage::DeleteTemplateHandler, this, std::placeholders::_1,std::placeholders::_2);
	m_listFuncHandler["SetTemplate"]=std::bind(&CServerManage::SetTemplateHandler, this, std::placeholders::_1,std::placeholders::_2);
	m_listFuncHandler["QueryTemplate"]=std::bind(&CServerManage::QueryTemplateHandler, this, std::placeholders::_1,std::placeholders::_2);

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
	auto index = SearchNeedCreateTemplate();
	if (index == -1)
	{
		pack->ResponsePackage(ResultType::Retry,"Retry");
		return true;
	}

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
	
	auto tpl = m_oTemplateList[index];
	sender_info->TemplateName = tpl.Name;
	sender_info->TemplateID = tpl.ID;
	m_pLog->Verbos(CUniversal::Format("Allocating module[%s] Type to [%s]", sender_info->UUID, sender_info->TemplateName));
	pack->ResponsePackage(sender_info->UUID,sender_info->TemplateID==-1?"": CBase64::Decode(m_oTemplateList[sender_info->TemplateID].Configuation));
	
	return true;
}


bool Sloong::CServerManage::ProcessHandler(Functions func, string sender, CDataTransPackage* pack)
{
	Json::Reader reader;
	Json::Value root;
	auto jreq = pack->GetRecvMessage();
	if (!reader.parse(jreq, root))
	{
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("Parser json[% s] error.", jreq));
		return true;
	}
	if (root["Function"].isNull())
	{
		pack->ResponsePackage(ResultType::Error, "Request no set [Function] node.");
		return true;
	}
	auto function = root["Function"].asString();
	if(!m_listFuncHandler.exist(function))
	{
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("Function [%s] no handler.",function));
		return true;
	}

	return m_listFuncHandler[function](sender,pack);
}


bool Sloong::CServerManage::AddTemplateHandler(const Json::Value& jRequest,CDataTransPackage* pack)
{
	auto jReq = jRequest["Template"];
	if(jReq["Name"].isNull()||jReq["Replicas"].isNull()||jReq["Configuation"].isNull())
	{
		pack->ResponsePackage(ResultType::Error, "The required parameter is null.");
		return true;
	}

	TemplateItem item;
	item.Name = jReq["Name"].asString();
	item.Note = jReq["Note"].asString();
	item.Replicas = jReq["Replicas"].asInt();
	item.Configuation = jReq["Configuation"].asString();
	int id = 0;
	auto res = m_pAllConfig->AddTemplate(item.ToTemplateInfo(),&id);
	if( res.IsFialed() )
	{
		pack->ResponsePackage(ResultType::Error, res.Message());
		return true;
	}
	item.ID = id;
	m_oTemplateList[id] = item;
	pack->ResponsePackage(ResultType::Succeed,CUniversal::ntos(id));
	return true;
}


bool Sloong::CServerManage::DeleteTemplateHandler(const Json::Value& jRequest,CDataTransPackage* pack)
{
	if(jRequest["ID"].isNull())
	{
		pack->ResponsePackage(ResultType::Error, "The required parameter is null.");
		return true;
	}

	int id = jRequest["ID"].asInt();
	if( !m_oTemplateList.exist(id))
	{
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("The template id [%d] is no exist.",id));
		return true;
	}

	auto res = m_pAllConfig->DeleteTemplate(id);
	if( res.IsFialed() )
	{
		pack->ResponsePackage(ResultType::Error, res.Message());
		return true;
	}
	m_oTemplateList.erase(id);
	pack->ResponsePackage(CResult::Succeed());
	return true;
}


bool Sloong::CServerManage::SetTemplateHandler(const Json::Value& jRequest,CDataTransPackage* pack)
{
	auto jReq = jRequest["Template"];
	if (jReq["ID"].isNull() || !m_oTemplateList.exist(jReq["ID"].asInt()))
	{
		pack->ResponsePackage(ResultType::Error, "Check the templeate ID error, please check.");
		return true;
	}
	
	auto info = m_oTemplateList[jReq["ID"].asInt()];
	if (!jReq["Name"].isNull())
		info.Name = jReq["Name"].asString();

	if (!jReq["Note"].isNull())
		info.Note = jReq["Note"].asString();

	if (!jReq["Replcas"].isNull())
		info.Replicas = jReq["Replcas"].asInt();

	if (!jReq["Configuation"].isNull())
		info.Configuation = jReq["Configuation"].asString();

	auto res = m_pAllConfig->SetTemplate(info.ID, info.ToTemplateInfo());
	if (res.IsFialed())
	{
		pack->ResponsePackage(ResultType::Error, res.Message());
		return true;
	}

	pack->ResponsePackage(ResultType::Succeed);
	m_oTemplateList[info.ID] =  info;
	return true;
}

bool Sloong::CServerManage::QueryTemplateHandler(const Json::Value& jRequest,CDataTransPackage* pack)
{
	Json::Value list;
	if( !jRequest["ID"].isNull() )
	{
		int id = jRequest["ID"].asInt();
		if(!m_oTemplateList.exist(id))
		{
			pack->ResponsePackage(ResultType::Error, CUniversal::Format("The template id [%d] is no exist.",id));
			return true;
		}

		list.append(m_oTemplateList[id].ToJson());
	}
	else
	{
		for (auto& i : m_oTemplateList)
		{
			list.append(i.second.ToJson());
		}
	}
	
	Json::Value root;
	root["TemplateList"] = list;
	pack->ResponsePackage(root.toStyledString(),"");

	return true;
}

bool Sloong::CServerManage::GetServerListHandler(const Json::Value& jRequest,CDataTransPackage* pack)
{
	Json::Value list;
	for (auto& i : m_oServerList)
	{
		Json::Value item;
		item["UUID"] = i.first;
		item["TemplateID"] = i.second.TemplateID;
		list.append(item);
	}
	Json::Value root;
	root["ServerList"] = list;
	pack->ResponsePackage(root.toStyledString(), "");
	return true;
}
