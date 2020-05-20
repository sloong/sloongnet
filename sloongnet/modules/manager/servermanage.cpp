/*
 * @Author: WCB
 * @Date: 2020-04-29 09:27:21
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 19:19:53
 * @Description: file content
 */
#include "servermanage.h"

#include "utility.h"
#include "SendMessageEvent.hpp"


using namespace Sloong::Events;

CResult Sloong::CServerManage::Initialize(IControl *ic)
{
	if (ic)
	{
		m_pControl = ic;
		m_pLog = IData::GetLog();
	}

	m_mapFuncToHandler[Functions::PostLog] = std::bind(&CServerManage::EventRecorderHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::RegisteWorker] = std::bind(&CServerManage::RegisteWorkerHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::RegisteNode] = std::bind(&CServerManage::RegisteNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::AddTemplate] = std::bind(&CServerManage::AddTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::DeleteTemplate] = std::bind(&CServerManage::DeleteTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::SetTemplate] = std::bind(&CServerManage::SetTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::QueryTemplate] = std::bind(&CServerManage::QueryTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::QueryNode] = std::bind(&CServerManage::QueryNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::QueryReferenceInfo] = std::bind(&CServerManage::QueryReferenceInfoHandler, this, std::placeholders::_1, std::placeholders::_2);

	if (!CConfiguation::Instance->IsInituialized())
	{
		auto res = CConfiguation::Instance->Initialize("/data/configuation.db");
		if (res.IsFialed())
			return res;
	}

	// Initialize template list
	auto list = CConfiguation::Instance->GetTemplateList();
	for (auto item : list)
	{
		TemplateItem addItem(item);
		m_mapIDToTemplateItem[addItem.ID] = addItem;
		RefreshModuleReference(addItem.ID);
	}

	auto res = CConfiguation::Instance->GetTemplate(1);
	if (res.IsFialed())
		return CResult::Succeed();
	else
		return res;
}

CResult Sloong::CServerManage::ResetManagerTemplate(GLOBAL_CONFIG *config)
{
	string config_str;
	if (!config->SerializeToString(&config_str))
	{
		return CResult::Make_Error("Config SerializeToString error.");
	}
	TemplateItem item;
	item.ID = 1;
	item.Name = "Manager";
	item.Note = "This template just for the manager node.";
	item.Replicas = 1;
	item.Configuation = config_str;
	item.BuildCache();
	CResult res(ResultType::Succeed);
	auto info = item.ToTemplateInfo();
	if (CConfiguation::Instance->CheckTemplateExist(1))
		res = CConfiguation::Instance->SetTemplate(1, info);
	else
		res = CConfiguation::Instance->AddTemplate(info, nullptr);
	m_mapIDToTemplateItem[1] = item;
	return res;
}

int Sloong::CServerManage::SearchNeedCreateTemplate()
{
	// First time find the no created
	for (auto item : m_mapIDToTemplateItem)
	{
		if (item.second.Replicas == 0 || item.second.ID == 1)
			continue;

		if (item.second.Created.size() == item.second.Replicas)
			continue;

		if (item.second.Created.size() == 0)
			return item.first;
	}

	// Sencond time find the created < replicas
	for (auto item : m_mapIDToTemplateItem)
	{
		if (item.second.Replicas == 0 || item.second.ID == 1)
			continue;

		if (item.second.Created.size() == item.second.Replicas)
			continue;

		if (item.second.Created.size() < item.second.Replicas)
			return item.first;
	}
	return -1;
}


inline Functions ConvertStrToFunc(string str){
	return (Functions)stoi(str);
}

CResult Sloong::CServerManage::ProcessHandler(CDataTransPackage *pack)
{
	auto function = (Functions)pack->GetFunction();
	if (!Manager::Functions_IsValid(function))
	{
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("Parser request package function[%s] error.", pack->GetRecvMessage()));
		return CResult::Succeed();
	}
	
	auto req_obj = pack->GetRecvMessage();
	auto func_name = Functions_Name(function);
	m_pLog->Debug(CUniversal::Format("Request [%d][%s]:[%s]", function, func_name, req_obj));
	if (!m_mapFuncToHandler.exist(function))
	{
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("Function [%s] no handler.",func_name));
		return CResult::Succeed();
	}

	auto res = m_mapFuncToHandler[function](req_obj, pack);
	m_pLog->Debug(CUniversal::Format("Response [%s]:[%s][%s].", func_name, Core::ResultType_Name(res.Result()), res.Message()));
	pack->ResponsePackage(res);
	return CResult::Succeed();
}

CResult Sloong::CServerManage::EventRecorderHandler(const string& req_obj, CDataTransPackage *pack)
{
	return CResult::Succeed();
}

CResult Sloong::CServerManage::RegisteWorkerHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto sender = pack->GetRecvPackage()->sender();
	if (sender.length() == 0)
	{
		sender = CUtility::GenUUID();
	}
	auto sender_info = m_mapUUIDToNodeItem.try_get(sender);
	if (sender_info == nullptr)
	{
		NodeItem item;
		item.Address = pack->GetSocketIP();
		item.UUID = sender;
		m_mapUUIDToNodeItem[sender] = item;
		m_pLog->Debug(CUniversal::Format("Module[%s:%d] regist to system. Allocating uuid [%s].", item.Address, item.Port, item.UUID));
		return CResult(ResultType::Retry, sender);
	}

	auto index = SearchNeedCreateTemplate();
	if (index == -1)
	{
		return CResult(ResultType::Retry, sender);
	}

	if (sender_info == nullptr)
	{
		return CResult::Make_Error("Add server info to ServerList fialed.");
	}

	auto tpl = m_mapIDToTemplateItem[index];
	RegisteWorkerResponse res;
	res.set_templateid(tpl.ID);
	res.set_configuation(m_mapIDToTemplateItem[tpl.ID].Configuation);

	m_pLog->Debug(CUniversal::Format("Allocating module[%s] Type to [%s]", sender_info->UUID, tpl.Name));
	return CResult::Make_OK( ConvertObjToStr(&res) );
}

void Sloong::CServerManage::RefreshModuleReference(int id)
{
	m_mapIDToTemplateItem[id].Reference.clear();
	auto references = CUniversal::split(m_mapIDToTemplateItem[id].ConfiguationObj->modulereference(), ';');
	for (auto item : references)
		m_mapIDToTemplateItem[id].Reference.push_back(stoi(item));
}

CResult Sloong::CServerManage::RegisteNodeHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto sender = pack->GetRecvPackage()->sender();
	auto req = ConvertStrToObj<RegisteNodeRequest>(req_obj);
	if (!req || sender.length() == 0)
		return CResult::Make_Error("The required parameter check error.");

	int id = req->templateid();
	if (!m_mapIDToTemplateItem.exist(id))
		return CResult::Make_Error(CUniversal::Format("The template id [%d] is no exist.", id));

	if (!m_mapUUIDToNodeItem.exist(sender))
		return CResult::Make_Error(CUniversal::Format("The sender [%d] is no regitser.", sender));

	if (id == 1)
		return CResult::Make_Error("Template id error.");

	// Save node info.
	auto& item = m_mapUUIDToNodeItem[sender];
	auto& tpl = m_mapIDToTemplateItem[id];
	item.TemplateName = m_mapIDToTemplateItem[id].Name;
	item.TemplateID = m_mapIDToTemplateItem[id].ID;
	item.Port = m_mapIDToTemplateItem[id].ConfiguationObj->listenport();
	item.Connection = pack->GetConnection();
	m_mapIDToTemplateItem[id].Created.unique_insert(sender);
	m_mapSocketToUUID[pack->GetConnection()->GetSocketID()] = sender;

	// Find reference node and notify them
	list<string> notifyList;
	for (auto item : m_mapIDToTemplateItem)
	{
		if (item.second.Reference.exist(id))
		{
			for (auto i : item.second.Created)
				notifyList.push_back(i);
		}
	}

	if (notifyList.size() > 0)
	{
		EventReferenceModuleOnline online_event;
		m_mapUUIDToNodeItem[sender].ToProtobuf(online_event.mutable_item());
		SendEvent(notifyList, Manager::Events::ReferenceModuleOnline, &online_event);
	}

	return CResult::Succeed();
}

CResult Sloong::CServerManage::AddTemplateHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<AddTemplateRequest>(req_obj);
	auto info = req->addinfo();
	TemplateItem item;
	item.ID = 0;
	item.Name = info.name();
	item.Note = info.note();;
	item.Replicas = info.replicas();
	item.Configuation = info.configuation();
	item.BuildCache();
	if( !item.IsValid() )
		return CResult::Make_Error("Param is valid.");

	int id = 0;
	auto res = CConfiguation::Instance->AddTemplate(item.ToTemplateInfo(), &id);
	if (res.IsFialed())
	{
		return res;
	}
	item.ID = id;
	m_mapIDToTemplateItem[id] = item;
	RefreshModuleReference(id);
	return CResult::Succeed();
}

CResult Sloong::CServerManage::DeleteTemplateHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<DeleteTemplateRequest>(req_obj);

	int id = req->templateid();
	if (!m_mapIDToTemplateItem.exist(id))
	{
		return CResult::Make_Error(CUniversal::Format("The template id [%d] is no exist.", id));
	}
	if (id == 1)
	{
		return CResult::Make_Error("Cannot delete this template.");
	}

	auto res = CConfiguation::Instance->DeleteTemplate(id);
	if (res.IsFialed())
	{
		return res;
	}
	m_mapIDToTemplateItem.erase(id);
	return CResult::Succeed();
}

CResult Sloong::CServerManage::SetTemplateHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<SetTemplateRequest>(req_obj);
	auto info = req->setinfo();
	if ( !m_mapIDToTemplateItem.exist(info.id()))
	{
		return CResult::Make_Error("Check the templeate ID error, please check.");
	}

	auto tplInfo = m_mapIDToTemplateItem[info.id()];
	if (info.name().size()>0)
		tplInfo.Name = info.name();

	if (info.note().size() > 0)
		tplInfo.Note = info.note();

	if (info.replicas() > 0 )
		tplInfo.Replicas = info.replicas();

	if (info.configuation().size() > 0)
		tplInfo.Configuation = info.configuation();

	tplInfo.BuildCache();		
	auto res = CConfiguation::Instance->SetTemplate(tplInfo.ID, tplInfo.ToTemplateInfo());
	if (res.IsFialed())
	{
		return res;
	}

	m_mapIDToTemplateItem[tplInfo.ID] = tplInfo;
	RefreshModuleReference(tplInfo.ID);
	return CResult::Succeed();
}

CResult Sloong::CServerManage::QueryTemplateHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<QueryTemplateRequest>(req_obj);
	
	QueryTemplateResponse res;
	if (req->templateid_size()==0)
	{
		for (auto &i : m_mapIDToTemplateItem)
		{
			i.second.ToProtobuf(res.add_templateinfos());
		}
	}
	else
	{
		auto ids = req->templateid();
		for( auto id : ids )
		{
			if (!m_mapIDToTemplateItem.exist(id))
			{
				return CResult::Make_Error(CUniversal::Format("The template id [%d] is no exist.", id));
			}
			m_mapIDToTemplateItem[id].ToProtobuf(res.add_templateinfos());
		}
	}

	return CResult::Make_OK( ConvertObjToStr(&res) );
}

CResult Sloong::CServerManage::QueryNodeHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<QueryNodeRequest>(req_obj);
	if(!req)
		return CResult::Make_Error("Parser message object fialed.");

	QueryNodeResponse res;
	if( req->templateid_size() == 0 )
	{
		for( auto node:m_mapUUIDToNodeItem)
		{
			node.second.ToProtobuf(res.add_nodeinfos());
		}
	}
	else
	{
		auto id_list = req->templateid();
		for( auto id: id_list)
		{
			for( auto servID : m_mapIDToTemplateItem[id].Created)
			{
				m_mapUUIDToNodeItem[servID].ToProtobuf(res.add_nodeinfos());
			}
		}
	}
	
	return CResult::Make_OK( ConvertObjToStr(&res) );
}


CResult Sloong::CServerManage::QueryReferenceInfoHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto data_pack = pack->GetRecvPackage();
	auto uuid = data_pack->sender();
	if( !m_mapUUIDToNodeItem.exist(uuid))
		return CResult::Make_Error(CUniversal::Format("The node is no registed. [%s]",uuid));

	auto id = m_mapUUIDToNodeItem[uuid].TemplateID;

	QueryReferenceInfoResponse res;
	auto references = CUniversal::split(m_mapIDToTemplateItem[id].ConfiguationObj->modulereference(),',');
	for (  auto ref: references )
	{
		auto item = res.add_templateinfos();
		auto tpl = m_mapIDToTemplateItem[stoi(ref)];
		item->set_templateid(tpl.ID);
		item->set_providefunctions(tpl.ConfiguationObj->modulefunctoins());
		for( auto node: tpl.Created)
		{
			m_mapUUIDToNodeItem[node].ToProtobuf(item->add_nodeinfos());
		}
	}
	
	return CResult::Make_OK( ConvertObjToStr(&res) );
}

void Sloong::CServerManage::SendEvent(list<string> notifyList, int event, ::google::protobuf::Message *msg)
{
	for (auto item : notifyList)
	{
		string msg_str;
		msg->SerializeToString(&msg_str);
		auto req = make_shared<CSendPackageEvent>();
		req->SetRequest(m_mapUUIDToNodeItem[item].Connection->GetSocketID(),"0" ,m_nSerialNumber, 1, event, msg_str, "", DataPackage_PackageType::DataPackage_PackageType_EventPackage);
		m_pControl->SendMessage(req);
	}
}

void Sloong::CServerManage::OnSocketClosed(SOCKET sock)
{
	if (!m_mapSocketToUUID.exist(sock))
		return;

	auto target = m_mapSocketToUUID[sock];
	auto id = m_mapUUIDToNodeItem[target].TemplateID;

	// Find reference node and notify them
	list<string> notifyList;
	for (auto item : m_mapIDToTemplateItem)
	{
		if (item.second.Reference.exist(id))
		{
			for (auto i : item.second.Created)
				notifyList.push_back(i);
		}
	}

	if (notifyList.size() > 0)
	{
		EventReferenceModuleOffline offline_event;
		offline_event.set_uuid(target);
		SendEvent(notifyList, Manager::Events::ReferenceModuleOffline, &offline_event);
	}
	m_mapUUIDToNodeItem.erase(target);
	m_mapIDToTemplateItem[id].Created.remove(target);
}