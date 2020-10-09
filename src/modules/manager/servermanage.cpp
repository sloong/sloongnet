/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-04-29 09:27:21
 * @LastEditTime: 2020-10-09 10:44:00
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/manager/servermanage.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#include "servermanage.h"

#include "utility.h"
#include "snowflake.h"

#include "events/SendPackage.hpp"
#include "events/GetConnectionInfo.hpp"
using namespace Sloong::Events;

CResult Sloong::CServerManage::Initialize(IControl *ic, const string &db_path)
{
	IObject::Initialize(ic);

	m_mapFuncToHandler[Functions::PostLog] = std::bind(&CServerManage::EventRecorderHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::RegisteWorker] = std::bind(&CServerManage::RegisteWorkerHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::RegisteNode] = std::bind(&CServerManage::RegisteNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::AddTemplate] = std::bind(&CServerManage::AddTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::DeleteTemplate] = std::bind(&CServerManage::DeleteTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::SetTemplate] = std::bind(&CServerManage::SetTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::QueryTemplate] = std::bind(&CServerManage::QueryTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::QueryNode] = std::bind(&CServerManage::QueryNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::QueryReferenceInfo] = std::bind(&CServerManage::QueryReferenceInfoHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::StopNode] = std::bind(&CServerManage::StopNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::RestartNode] = std::bind(&CServerManage::RestartNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
	m_mapFuncToHandler[Functions::ReportLoadStatus] = std::bind(&CServerManage::ReportLoadStatusHandler, this, std::placeholders::_1, std::placeholders::_2);

	if (!CConfiguation::Instance->IsInituialized())
	{
		auto res = CConfiguation::Instance->Initialize(db_path);
		if (res.IsFialed())
			return res;
	}

	// Initialize template list
	auto list = CConfiguation::Instance->GetTemplateList();
	for (auto &item : list)
	{
		TemplateItem addItem(item);
		m_mapIDToTemplateItem[addItem.ID] = addItem;
		RefreshModuleReference(addItem.ID);
	}
	m_mapIDToTemplateItem[1].Created.push_back(0);
	return CResult::Succeed;
}

CResult Sloong::CServerManage::LoadManagerConfig(const string &db_path)
{
	if (!CConfiguation::Instance->IsInituialized())
	{
		auto res = CConfiguation::Instance->Initialize(db_path);
		if (res.IsFialed())
			return res;
	}
	auto res = CConfiguation::Instance->GetTemplate(1);
	if (res.IsFialed())
		return CResult(ResultType::Warning);
	return CResult::Make_OK(string(res.GetResultObject().configuation.begin(), res.GetResultObject().configuation.end()));
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
	return res;
}

int Sloong::CServerManage::SearchNeedCreateTemplate()
{
	// First time find the no created
	for (auto item : m_mapIDToTemplateItem)
	{
		if (item.second.Replicas == 0 || item.second.ID == 1)
			continue;

		if ((int)item.second.Created.size() >= item.second.Replicas)
			continue;

		if (item.second.Created.size() == 0)
			return item.first;
	}

	// Sencond time find the created < replicas
	for (auto item : m_mapIDToTemplateItem)
	{
		if (item.second.Replicas == 0 || item.second.ID == 1)
			continue;

		if ((int)item.second.Created.size() >= item.second.Replicas)
			continue;

		if ((int)item.second.Created.size() < item.second.Replicas)
			return item.first;
	}
	return 0;
}

void Sloong::CServerManage::SendEvent(const list<uint64_t> &notifyList, int event, ::google::protobuf::Message *msg)
{
	for (auto item : notifyList)
	{
		string msg_str;
		if (msg)
			msg->SerializeToString(&msg_str);
		auto req = make_unique<SendPackageEvent>(m_mapUUIDToNodeItem[item].ConnectionHashCode);
		req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, event, msg_str, DataPackage_PackageType::DataPackage_PackageType_EventPackage);
		m_iC->SendMessage(std::move(req));
	}
}

void Sloong::CServerManage::OnSocketClosed(uint64_t con)
{
	if (!m_mapConnectionToUUID.exist(con))
		return;

	auto target = m_mapConnectionToUUID[con];
	auto id = m_mapUUIDToNodeItem[target].TemplateID;

	// Find reference node and notify them
	list<uint64_t> notifyList;
	for (auto &item : m_mapIDToTemplateItem)
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

PackageResult Sloong::CServerManage::ProcessHandler(DataPackage *pack)
{
	auto function = (Functions)pack->function();
	if (!Manager::Functions_IsValid(function))
	{
		return PackageResult::Make_OKResult(Package::MakeErrorResponse(pack, Helper::Format("Parser request package function[%s] error.", pack->content().data().c_str())));
	}

	auto req_str = pack->content().data();
	auto func_name = Functions_Name(function);
	m_pLog->Debug(Helper::Format("Request [%d][%s]", function, func_name.c_str()));
	if (!m_mapFuncToHandler.exist(function))
	{
		return PackageResult::Make_OKResult(Package::MakeErrorResponse(pack, Helper::Format("Function [%s] no handler.", func_name.c_str())));
	}

	auto res = m_mapFuncToHandler[function](req_str, pack);
	if (res.IsError())
		m_pLog->Debug(Helper::Format("Response [%s]:[%s][%s].", func_name.c_str(), ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str()));
	else
		m_pLog->Verbos(Helper::Format("Response [%s]:[%s]", func_name.c_str(), ResultType_Name(res.GetResult()).c_str()));
	if (res.GetResult() == ResultType::Ignore)
		return PackageResult::Ignore();

	return PackageResult::Make_OKResult(Package::MakeResponse(pack, res));
}

CResult Sloong::CServerManage::EventRecorderHandler(const string &req_str, DataPackage *pack)
{
	return CResult::Succeed;
}

CResult Sloong::CServerManage::RegisteWorkerHandler(const string &req_str, DataPackage *pack)
{
	auto sender = pack->sender();
	if (sender == 0)
	{
		sender = snowflake::Instance->nextid();
	}
	auto sender_info = m_mapUUIDToNodeItem.try_get(sender);
	if (sender_info == nullptr)
	{
		NodeItem item;
		item.UUID = sender;
		m_mapUUIDToNodeItem[sender] = item;
		auto event = make_shared<GetConnectionInfoEvent>(pack->reserved().sessionid());
		event->SetCallbackFunc([item = &m_mapUUIDToNodeItem[sender]](IEvent *e, ConnectionInfo info) {
			item->Address = info.Address;
		});
		m_iC->SendMessage(event);
		m_pLog->Debug(Helper::Format("Module[%s:%d] regist to system. Allocating uuid [%lld].", item.Address.c_str(), item.Port, item.UUID));
		char m_pMsgBuffer[8] = {0};
		char *pCpyPoint = m_pMsgBuffer;
		Helper::Int64ToBytes(sender, pCpyPoint);
		return CResult(ResultType::Retry, string(m_pMsgBuffer, 8));
	}

	int index = 0;
	if (req_str.length() > 0)
	{
		auto req = ConvertStrToObj<RegisteWorkerRequest>(req_str);
		index = req->forcetargettemplateid();
	}
	if (index == 0)
	{
		index = SearchNeedCreateTemplate();
	}

	if (index == 0)
	{
		return CResult(ResultType::Retry, "Wait");
	}

	if (sender_info == nullptr)
	{
		return CResult::Make_Error("Add server info to ServerList fialed.");
	}

	if (!m_mapIDToTemplateItem.exist(index))
	{
		return CResult(ResultType::Retry, "Allocating type no exist.");
	}

	auto tpl = m_mapIDToTemplateItem[index];
	RegisteWorkerResponse res;
	res.set_templateid(tpl.ID);
	res.set_configuation(m_mapIDToTemplateItem[tpl.ID].Configuation);

	m_pLog->Debug(Helper::Format("Allocating module[%lld] Type to [%s]", sender_info->UUID, tpl.Name.c_str()));
	return CResult::Make_OK(ConvertObjToStr(&res));
}

void Sloong::CServerManage::RefreshModuleReference(int id)
{
	auto info = m_mapIDToTemplateItem.try_get(id);
	if (info == nullptr)
		return;
	info->Reference.clear();
	auto references = Helper::split(info->ConfiguationObj->modulereference(), ',');
	for (auto &item : references)
	{
		int id;
		if (ConvertStrToInt(item, &id))
			info->Reference.push_back(id);
	}
}

CResult Sloong::CServerManage::RegisteNodeHandler(const string &req_str, DataPackage *pack)
{
	auto sender = pack->sender();
	auto req = ConvertStrToObj<RegisteNodeRequest>(req_str);
	if (!req || sender == 0)
		return CResult::Make_Error("The required parameter check error.");

	int id = req->templateid();
	if (!m_mapIDToTemplateItem.exist(id))
		return CResult::Make_Error(Helper::Format("The template id [%d] is no exist.", id));

	if (!m_mapUUIDToNodeItem.exist(sender))
		return CResult::Make_Error(Helper::Format("The sender [%lld] is no regitser.", sender));

	if (id == 1)
		return CResult::Make_Error("Template id error.");

	// Save node info.
	auto &item = m_mapUUIDToNodeItem[sender];
	auto &tpl = m_mapIDToTemplateItem[id];
	item.TemplateName = tpl.Name;
	item.TemplateID = tpl.ID;
	item.Port = tpl.ConfiguationObj->listenport();
	item.ConnectionHashCode = pack->reserved().sessionid();
	tpl.Created.unique_insert(sender);
	m_mapConnectionToUUID[pack->reserved().sessionid()] = sender;

	// Find reference node and notify them
	list<uint64_t> notifyList;
	for (auto &item : m_mapIDToTemplateItem)
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

	return CResult::Succeed;
}

CResult Sloong::CServerManage::AddTemplateHandler(const string &req_str, DataPackage *pack)
{
	auto req = ConvertStrToObj<AddTemplateRequest>(req_str);
	auto info = req->addinfo();
	TemplateItem item;
	item.ID = 0;
	item.Name = info.name();
	item.Note = info.note();
	;
	item.Replicas = info.replicas();
	item.Configuation = info.configuation();
	item.BuildCache();
	if (!item.IsValid())
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
	return CResult::Succeed;
}

CResult Sloong::CServerManage::DeleteTemplateHandler(const string &req_str, DataPackage *pack)
{
	auto req = ConvertStrToObj<DeleteTemplateRequest>(req_str);

	int id = req->templateid();
	if (!m_mapIDToTemplateItem.exist(id))
	{
		return CResult::Make_Error(Helper::Format("The template id [%d] is no exist.", id));
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
	return CResult::Succeed;
}

CResult Sloong::CServerManage::SetTemplateHandler(const string &req_str, DataPackage *pack)
{
	auto req = ConvertStrToObj<SetTemplateRequest>(req_str);
	auto info = req->setinfo();
	if (!m_mapIDToTemplateItem.exist(info.id()))
	{
		return CResult::Make_Error("Check the templeate ID error, please check.");
	}

	auto tplInfo = m_mapIDToTemplateItem[info.id()];
	if (info.name().size() > 0)
		tplInfo.Name = info.name();

	if (info.note().size() > 0)
		tplInfo.Note = info.note();

	if (info.replicas() > 0)
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
	return CResult::Succeed;
}

CResult Sloong::CServerManage::QueryTemplateHandler(const string &req_str, DataPackage *pack)
{
	auto req = ConvertStrToObj<QueryTemplateRequest>(req_str);

	QueryTemplateResponse res;
	if (req->queryall())
	{
		for (auto &i : m_mapIDToTemplateItem)
		{
			i.second.ToProtobuf(res.add_templateinfos());
		}
	}
	else
	{
		if (req->templateid_size() > 0)
		{
			auto ids = req->templateid();
			for (auto id : ids)
			{
				if (m_mapIDToTemplateItem.exist(id))
					m_mapIDToTemplateItem[id].ToProtobuf(res.add_templateinfos());
				else
					return CResult::Make_Error(Helper::Format("The template id [%d] is no exist.", id));
			}
		}
		else if (req->templatetype_size() > 0)
		{
			auto types = req->templatetype();
			for (auto t : types)
			{
				for (auto &item : m_mapIDToTemplateItem)
				{
					if (item.second.ConfiguationObj->moduletype() == t)
						item.second.ToProtobuf(res.add_templateinfos());
				}
			}
		}
	}

	auto str_res = ConvertObjToStr(&res);
	m_pLog->Debug(Helper::Format("Query Template Succeed: Count[%d];[%s]", res.templateinfos_size(), CBase64::Encode(str_res).c_str()));
	return CResult::Make_OK(str_res);
}

CResult Sloong::CServerManage::QueryNodeHandler(const string &req_str, DataPackage *pack)
{
	auto req = ConvertStrToObj<QueryNodeRequest>(req_str);
	if (!req)
		return CResult::Make_Error("Parser message object fialed.");

	QueryNodeResponse res;
	if (req->templateid_size() == 0)
	{
		for (auto node : m_mapUUIDToNodeItem)
		{
			node.second.ToProtobuf(res.add_nodeinfos());
		}
	}
	else
	{
		auto id_list = req->templateid();
		for (auto id : id_list)
		{
			for (auto servID : m_mapIDToTemplateItem[id].Created)
			{
				m_mapUUIDToNodeItem[servID].ToProtobuf(res.add_nodeinfos());
			}
		}
	}

	return CResult::Make_OK(ConvertObjToStr(&res));
}

CResult Sloong::CServerManage::StopNodeHandler(const string &req_str, DataPackage *pack)
{
	auto req = ConvertStrToObj<StopNodeRequest>(req_str);
	if (!req)
		return CResult::Make_Error("Parser message object fialed.");

	auto id = req->nodeid();
	if (!m_mapUUIDToNodeItem.exist(id))
		return CResult::Make_Error("NodeID error, the node no exit.");

	list<uint64_t> l;
	l.push_back(id);
	SendEvent(l, Core::ControlEvent::Stop, nullptr);

	return CResult::Succeed;
}

CResult Sloong::CServerManage::RestartNodeHandler(const string &req_str, DataPackage *pack)
{
	auto req = ConvertStrToObj<RestartNodeRequest>(req_str);
	if (!req)
		return CResult::Make_Error("Parser message object fialed.");

	auto id = req->nodeid();
	if (!m_mapUUIDToNodeItem.exist(id))
		return CResult::Make_Error("NodeID error, the node no exit.");

	list<uint64_t> l;
	l.push_back(id);
	SendEvent(l, Core::ControlEvent::Restart, nullptr);

	return CResult::Succeed;
}

CResult Sloong::CServerManage::QueryReferenceInfoHandler(const string &req_str, DataPackage *pack)
{
	m_pLog->Verbos("QueryReferenceInfoHandler <<< ");
	auto uuid = pack->sender();
	if (!m_mapUUIDToNodeItem.exist(uuid))
		return CResult::Make_Error(Helper::Format("The node is no registed. [%lld]", uuid));

	auto id = m_mapUUIDToNodeItem[uuid].TemplateID;
	if (!m_mapIDToTemplateItem.exist(id))
		return CResult::Make_Error(Helper::Format("The template id error. UUID[%lld];ID[%d]", uuid, id));

	QueryReferenceInfoResponse res;
	auto references = Helper::split(m_mapIDToTemplateItem[id].ConfiguationObj->modulereference(), ',');
	for (auto ref : references)
	{
		auto ref_id = 0;
		if (!ConvertStrToInt(ref, &ref_id))
			continue;
		auto item = res.add_templateinfos();
		auto tpl = m_mapIDToTemplateItem[ref_id];
		item->set_templateid(tpl.ID);
		item->set_type(tpl.ConfiguationObj->moduletype() );
		item->set_providefunctions(tpl.ConfiguationObj->modulefunctoins());
		for (auto node : tpl.Created)
		{
			m_mapUUIDToNodeItem[node].ToProtobuf(item->add_nodeinfos());
		}
	}
	m_pLog->Verbos("QueryReferenceInfoHandler response >>> " + res.ShortDebugString());
	return CResult::Make_OK(ConvertObjToStr(&res));
}

CResult Sloong::CServerManage::ReportLoadStatusHandler(const string &req_str, DataPackage *pack)
{
	auto req = ConvertStrToObj<ReportLoadStatusRequest>(req_str);

	m_pLog->Info(Helper::Format("Node[%lld] load status :CPU[%lf]Mem[%lf]", pack->sender(), req->cpuload(), req->memroyused()));

	return CResult(ResultType::Ignore);
}
