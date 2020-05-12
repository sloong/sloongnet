/*
 * @Author: WCB
 * @Date: 2020-04-29 09:27:21
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-12 11:49:31
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

	m_listFuncHandler[FunctionType::PostLog] = std::bind(&CServerManage::EventRecorderHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_listFuncHandler[FunctionType::RegisteWorker] = std::bind(&CServerManage::RegisteWorkerHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_listFuncHandler[FunctionType::RegisteNode] = std::bind(&CServerManage::RegisteNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_listFuncHandler[FunctionType::AddTemplate] = std::bind(&CServerManage::AddTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_listFuncHandler[FunctionType::DeleteTemplate] = std::bind(&CServerManage::DeleteTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_listFuncHandler[FunctionType::SetTemplate] = std::bind(&CServerManage::SetTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_listFuncHandler[FunctionType::QueryTemplate] = std::bind(&CServerManage::QueryTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_listFuncHandler[FunctionType::QueryNode] = std::bind(&CServerManage::QueryNodeHandler, this, std::placeholders::_1, std::placeholders::_2);

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
		m_oTemplateList[addItem.ID] = addItem;
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
	TemplateInfo info;
	info.id = 1;
	info.name = "Manager";
	info.note = "This template just for the manager node.";
	info.replicas = 1;
	info.configuation = CBase64::Encode(config_str);
	CResult res(ResultType::Succeed);
	if (CConfiguation::Instance->CheckTemplateExist(1))
		res = CConfiguation::Instance->SetTemplate(1, info);
	else
		res = CConfiguation::Instance->AddTemplate(info, nullptr);
	m_oTemplateList[1] = info;
	return res;
}

int Sloong::CServerManage::SearchNeedCreateTemplate()
{
	// First time find the no created
	for (auto item : m_oTemplateList)
	{
		if (item.second.Replicas == 0 || item.second.ID == 1)
			continue;

		if (item.second.Created.size() == item.second.Replicas)
			continue;

		if (item.second.Created.size() == 0)
			return item.first;
	}

	// Sencond time find the created < replicas
	for (auto item : m_oTemplateList)
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

template<class T> inline
unique_ptr<T> ConvertStrToObj(string obj){
	unique_ptr<T> item = make_unique<T>();
	if(!item->ParseFromString(obj))
		return nullptr;
	return item;
}

inline string ConvertObjToStr(::google::protobuf::Message* obj){
	string str_res;
	if(obj->SerializeToString(&str_res))
		return "";
	return str_res;
}

CResult Sloong::CServerManage::ProcessHandler(CDataTransPackage *pack)
{
	auto str_req = pack->GetRecvMessage();
	auto req_pack = ConvertStrToObj<RequestPackage>(str_req);
	if (!req_pack)
	{
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("Parser request package [%s] error.", str_req));
		return CResult::Succeed();
	}
	auto function = req_pack->function();
	auto req_obj = req_pack->requestobject();
	auto func_name = FunctionType_Name(function);
	m_pLog->Verbos(CUniversal::Format("Request [%d][%s]:[%s]", function, func_name, str_req));
	if (!m_listFuncHandler.exist(function))
	{
		pack->ResponsePackage(ResultType::Error, CUniversal::Format("Function [%s] no handler.",func_name));
		return CResult::Succeed();
	}

	auto res = m_listFuncHandler[function](req_obj, pack);
	m_pLog->Verbos(CUniversal::Format("Response [%s]:[%s][%s].", func_name, Protocol::ResultType_Name(res.Result()), res.Message()));
	ResponsePackage res_pack;
	res_pack.set_function(function);
	res_pack.set_responseobject(res.Message());
	pack->ResponsePackage(res.Result(), ConvertObjToStr(&res_pack));
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
	auto sender_info = m_oWorkerList.try_get(sender);
	if (sender_info == nullptr)
	{
		ServerItem item;
		item.Address = pack->GetSocketIP();
		item.Port = pack->GetSocketPort();
		item.UUID = sender;
		m_oWorkerList[sender] = item;
		m_pLog->Verbos(CUniversal::Format("Module[%s:%d] regist to system. Allocating uuid [%s].", item.Address, item.Port, item.UUID));
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

	auto tpl = m_oTemplateList[index];
	RegisteWorkerResponse res;
	res.set_templateid(tpl.ID);
	res.set_configuation(m_oTemplateList[tpl.ID].Configuation);

	m_pLog->Verbos(CUniversal::Format("Allocating module[%s] Type to [%s]", sender_info->UUID, tpl.Name));
	return CResult::Make_OK( ConvertObjToStr(&res) );
}

void Sloong::CServerManage::RefreshModuleReference(int id)
{
	m_oTemplateList[id].Reference.clear();
	GLOBAL_CONFIG config;
	config.ParseFromString(CBase64::Decode(m_oTemplateList[id].Configuation));
	auto references = CUniversal::split(config.modulereference(), ';');
	for (auto item : references)
		m_oTemplateList[id].Reference.push_back(atoi(item.c_str()));
}

CResult Sloong::CServerManage::RegisteNodeHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto sender = pack->GetRecvPackage()->sender();
	auto req = ConvertStrToObj<RegisteNodeRequest>(req_obj);
	if (!req || sender.length() == 0)
		return CResult::Make_Error("The required parameter check error.");

	int id = req->templateid();
	if (!m_oTemplateList.exist(id))
		return CResult::Make_Error(CUniversal::Format("The template id [%d] is no exist.", id));

	if (!m_oWorkerList.exist(sender))
		return CResult::Make_Error(CUniversal::Format("The sender [%d] is no regitser.", sender));

	if (id == 1)
		return CResult::Make_Error("Template id error.");

	// Save node info.
	m_oWorkerList[sender].TemplateName = m_oTemplateList[id].Name;
	m_oWorkerList[sender].TemplateID = m_oTemplateList[id].ID;
	m_oWorkerList[sender].Connection = pack->GetConnection();
	m_oTemplateList[id].Created.unique_insert(sender);
	m_oSocketList[pack->GetConnection()->GetSocketID()] = sender;

	// Find reference node and notify them
	list<string> notifyList;
	for (auto item : m_oTemplateList)
	{
		if (item.second.Reference.exist(id))
		{
			for (auto i : item.second.Created)
				notifyList.push_back(i);
		}
	}

	if (notifyList.size() > 0)
	{
		GLOBAL_CONFIG config;
		config.ParseFromString(CBase64::Decode(m_oTemplateList[id].Configuation));
		EventReferenceModuleOnline online_event;
		online_event.set_address(pack->GetSocketIP());
		online_event.set_port(config.listenport());
		SendEvent(notifyList, Manager::Events::ReferenceModuleOnline, &online_event);
	}

	return CResult::Succeed();
}

CResult Sloong::CServerManage::AddTemplateHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<AddTemplateRequest>(req_obj);
	auto info = req->addinfo();
	TemplateItem item;
	item.Name = info.name();
	item.Note = info.note();;
	item.Replicas = info.replicas();
	item.Configuation = info.configuation();
	int id = 0;
	auto res = CConfiguation::Instance->AddTemplate(item.ToTemplateInfo(), &id);
	if (res.IsFialed())
	{
		return res;
	}
	item.ID = id;
	m_oTemplateList[id] = item;
	RefreshModuleReference(id);
	return CResult::Succeed();
}

CResult Sloong::CServerManage::DeleteTemplateHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<DeleteTemplateRequest>(req_obj);

	int id = req->templateid();
	if (!m_oTemplateList.exist(id))
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
	m_oTemplateList.erase(id);
	return CResult::Succeed();
}

CResult Sloong::CServerManage::SetTemplateHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<SetTemplateRequest>(req_obj);
	auto info = req->setinfo();
	if ( !m_oTemplateList.exist(info.id()))
	{
		return CResult::Make_Error("Check the templeate ID error, please check.");
	}

	auto tplInfo = m_oTemplateList[info.id()];
	if (info.name().size()>0)
		tplInfo.Name = info.name();

	if (info.note().size() > 0)
		tplInfo.Note = info.note();

	if (info.replicas() > 0 )
		tplInfo.Replicas = info.replicas();

	if (info.configuation().size() > 0)
		tplInfo.Configuation = info.configuation();
		
	auto res = CConfiguation::Instance->SetTemplate(tplInfo.ID, tplInfo.ToTemplateInfo());
	if (res.IsFialed())
	{
		return res;
	}

	m_oTemplateList[tplInfo.ID] = tplInfo;
	RefreshModuleReference(tplInfo.ID);
	return CResult::Succeed();
}

CResult Sloong::CServerManage::QueryTemplateHandler(const string& req_obj, CDataTransPackage *pack)
{
	auto req = ConvertStrToObj<QueryTemplateRequest>(req_obj);
	
	QueryTemplateResponse res;
	if (req->templateid_size()==0)
	{
		for (auto &i : m_oTemplateList)
		{
			i.second.ToProtobuf(res.add_templateinfos());
		}
	}
	else
	{
		auto ids = req->templateid();
		for( auto id : ids )
		{
			if (!m_oTemplateList.exist(id))
			{
				return CResult::Make_Error(CUniversal::Format("The template id [%d] is no exist.", id));
			}
			m_oTemplateList[id].ToProtobuf(res.add_templateinfos());
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
	auto id_list = req->templateid();
	for( auto id: id_list)
	{
		for( auto servID:m_oTemplateList[id].Created)
		{
			auto nodeItem = m_oWorkerList[servID];
			auto item = res.add_nodeinfos();
			item->set_address(nodeItem.Address);
			item->set_port(nodeItem.Port);
		}
	}

	/*for_each(id_list.begin(),id_list.end(),[](int id){
		for( auto servID:m_oTemplateList[id].Created)
		{
			auto nodeItem = m_oWorkerList[servID];
			NodeInfoItem item;
			item.set_address(nodeItem.Address);
			item.set_port(nodeItem.Port);
			res.add_nodeinfo(item);
		}
	});*/
	return CResult::Make_OK( ConvertObjToStr(&res) );
}

void Sloong::CServerManage::SendEvent(list<string> notifyList, int event, ::google::protobuf::Message *msg)
{
	for (auto item : notifyList)
	{
		string msg_str;
		msg->SerializeToString(&msg_str);
		auto event = make_shared<CSendMessageEvent>();
		event->SetRequest(m_oWorkerList[item].Connection->GetSocketID(), CUniversal::ntos(event), msg_str, m_nSerialNumber, 1, Functions::ProcessEvent);
		m_pControl->SendMessage(event);
	}
}

void Sloong::CServerManage::OnSocketClosed(SOCKET sock)
{
	if (!m_oSocketList.exist(sock))
		return;

	auto target = m_oSocketList[sock];
	auto id = m_oWorkerList[target].TemplateID;
	m_oWorkerList.erase(target);
	m_oTemplateList[id].Created.remove(target);

	// Find reference node and notify them
	list<string> notifyList;
	for (auto item : m_oTemplateList)
	{
		if (item.second.Reference.exist(id))
		{
			for (auto i : item.second.Created)
				notifyList.push_back(i);
		}
	}

	if (notifyList.size() > 0)
	{
		GLOBAL_CONFIG config;
		config.ParseFromString(CBase64::Decode(m_oTemplateList[id].Configuation));
		EventReferenceModuleOffline offline_event;
		offline_event.set_address("");
		offline_event.set_port(0);
		SendEvent(notifyList, Manager::Events::ReferenceModuleOffline, &offline_event);
	}
}