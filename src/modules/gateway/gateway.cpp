/* File Name: server.c */
#include "gateway.h"
#include "utility.h"
#include "IData.h"
#include "events/SendPackageToManager.hpp"
#include "events/RegisteConnection.hpp"
using namespace Sloong;
using namespace Sloong::Events;

#include "protocol/manager.pb.h"
using namespace Manager;
#include "snowflake.h"


unique_ptr<SloongNetGateway> Sloong::SloongNetGateway::Instance = nullptr;

extern "C" PackageResult RequestPackageProcesser(void *env, DataPackage *pack)
{
	auto pTranspond = STATIC_TRANS<GatewayTranspond*>(env);
	if( pTranspond)
		return pTranspond->RequestPackageProcesser(pack);
	else
		return PackageResult::Make_Error("RequestPackageProcesser error, Environment convert failed.");
}

extern "C" PackageResult ResponsePackageProcesser(void *env, DataPackage *pack)
{
	auto num = pack->id();
	if( SloongNetGateway::Instance->m_mapSerialToRequest.exist(num) )
	{
		auto pTranspond = STATIC_TRANS<GatewayTranspond*>(env);
		if( pTranspond)
		{
			auto info = move(SloongNetGateway::Instance->m_mapSerialToRequest[num]);
			SloongNetGateway::Instance->m_mapSerialToRequest.erase(num);
			return pTranspond->ResponsePackageProcesser(move(info),pack);
		}
		else
			return PackageResult::Make_Error("ResponsePackageProcesser error, Environment convert failed.");
	}

	return SloongNetGateway::Instance->ResponsePackageProcesser(pack);
}

extern "C" CResult EventPackageProcesser(DataPackage *pack)
{
	SloongNetGateway::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
} 

extern "C" CResult NewConnectAcceptProcesser(SOCKET sock)
{
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialization(GLOBAL_CONFIG *confiog)
{
	SloongNetGateway::Instance = make_unique<SloongNetGateway>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(SOCKET sock, IControl *iC)
{
	return SloongNetGateway::Instance->Initialized(sock, iC);
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
	return SloongNetGateway::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult SloongNetGateway::Initialized(SOCKET sock, IControl *iC)
{
	IObject::Initialize(iC);
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	m_pModuleConfig = IData::GetModuleConfig();
	m_pRuntimeData = IData::GetRuntimeData();
	if (m_pModuleConfig)
	{
		auto event = make_shared<NormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableTimeoutCheck);
		event->SetMessage(Helper::Format("{\"TimeoutTime\":\"%d\", \"CheckInterval\":%d}", (*m_pModuleConfig)["TimeoutTime"].asInt(), (*m_pModuleConfig)["TimeoutCheckInterval"].asInt()));
		m_iC->SendMessage(event);

		event = make_shared<NormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableClientCheck);
		event->SetMessage(Helper::Format("{\"ClientCheckKey\":\"%s\", \"ClientCheckTime\":%d}", (*m_pModuleConfig)["ClientCheckKey"].asString().c_str(), (*m_pModuleConfig)["ClientCheckKey"].asInt()));
		m_iC->SendMessage(event);
	}
	m_nManagerConnection = sock;
	m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStart, std::bind(&SloongNetGateway::OnStart, this, std::placeholders::_1));
	return CResult::Succeed();
}


PackageResult SloongNetGateway::ResponsePackageProcesser( DataPackage *trans_pack)
{
	m_pLog->Error("ResponsePackageProcesser no find the package.");
	return PackageResult::Make_Error("ResponsePackageProcesser no find the package.");
}


void SloongNetGateway::QueryReferenceInfo()
{
	auto event = make_shared<SendPackageToManagerEvent>(Functions::QueryReferenceInfo, "");
	event->SetCallbackFunc(std::bind(&SloongNetGateway::QueryReferenceInfoResponseHandler, this, std::placeholders::_1, std::placeholders::_2));
	m_iC->CallMessage(event);
}

// process the provied function string to list.
list<int> SloongNetGateway::ProcessProviedFunction(const string &prov_func)
{
	list<int> res_list;
	auto funcs = Helper::split(prov_func, ',');
	for (auto func : funcs)
	{
		if (func.find("-") != string::npos)
		{
			auto range = Helper::split(func, '-');
			int start,end;
			if(!ConvertStrToInt(range[0],&start)||!ConvertStrToInt(range[1],&end) )
				return res_list;
			for (int i = start; i <= end; i++)
			{
				res_list.push_back(i);
			}
		}
		else
		{
			int nFunc;
			if (!ConvertStrToInt(func,&nFunc))
				return res_list;
			res_list.push_back(nFunc);
		}
	}
	return res_list;
}

void SloongNetGateway::QueryReferenceInfoResponseHandler(IEvent* send_pack, DataPackage *res_pack)
{
	auto str_res = res_pack->content();
	auto res = ConvertStrToObj<QueryReferenceInfoResponse>(str_res);
	if (res == nullptr || res->templateinfos_size() == 0)
		return;

	auto templateInfos = res->templateinfos();
	for (auto info : templateInfos)
	{
		if (info.providefunctions() == "*")
		{
			m_pLog->Verbos(Helper::Format("Universal processer find: template id[%d]", info.templateid()));
			m_mapFuncToTemplateIDs[-1].unique_insert(info.templateid());
		}
		else
		{
			for (auto i : ProcessProviedFunction(info.providefunctions()))
				m_mapFuncToTemplateIDs[i].unique_insert(info.templateid());
		}
		for (auto item : info.nodeinfos())
		{
			m_mapUUIDToNode[item.uuid()] = item;
			m_mapTempteIDToUUIDs[info.templateid()].push_back(item.uuid());

			AddConnection(item.uuid(), item.address(), item.port());
		}
	}
}

void SloongNetGateway::AddConnection( uint64_t uuid, const string &addr, int port)
{
	auto event = make_shared<RegisteConnectionEvent>(addr, port);
	event->SetCallbackFunc([this,uuid](IEvent* e, int64_t hashcode){
		m_mapUUIDToConnectionID[uuid] = hashcode;
	});
	m_iC->SendMessage(event);
}

CResult SloongNetGateway::CreateProcessEnvironmentHandler(void **out_env)
{
	auto item = make_unique<GatewayTranspond>();
	auto res = item->Initialize(m_iC);
	if (res.IsFialed())
		return res;
	(*out_env) = item.get();
	m_listTranspond.push_back(std::move(item));
	return CResult::Succeed();
}

void SloongNetGateway::OnStart(SharedEvent evt)
{
	QueryReferenceInfo();
}

void Sloong::SloongNetGateway::OnReferenceModuleOnlineEvent(const string &str_req, DataPackage *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOnline event");
	auto req = ConvertStrToObj<Manager::EventReferenceModuleOnline>(str_req);
	auto item = req->item();
	m_mapUUIDToNode[item.uuid()] = item;
	m_mapTempteIDToUUIDs[item.templateid()].push_back(item.uuid());
	m_pLog->Debug(Helper::Format("New module is online:templateid[%d]->node id[%llu][%s:%d],node list size[%d]", item.templateid(), item.uuid(), item.address().c_str(), item.port(), m_mapTempteIDToUUIDs.size()));

	AddConnection(item.uuid(), item.address(), item.port());
}

void Sloong::SloongNetGateway::OnReferenceModuleOfflineEvent(const string &str_req, DataPackage *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOffline event");
	auto req = ConvertStrToObj<Manager::EventReferenceModuleOffline>(str_req);
	auto uuid = req->uuid();
	auto item = m_mapUUIDToNode[uuid];
	m_mapTempteIDToUUIDs[item.templateid()].erase(item.uuid());
	m_mapUUIDToConnectionID.erase(uuid);
	m_mapUUIDToNode.erase(uuid);
}

void Sloong::SloongNetGateway::EventPackageProcesser(DataPackage *pack)
{
	auto event = (Manager::Events)pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->Error(Helper::Format("EventPackageProcesser is called.but the fucntion[%d] check error.", event));
		return;
	}

	switch (event)
	{
	case Manager::Events::ReferenceModuleOnline:
	{
		OnReferenceModuleOnlineEvent(pack->content(), pack);
	}
	break;
	case Manager::Events::ReferenceModuleOffline:
	{
		OnReferenceModuleOfflineEvent(pack->content(), pack);
	}
	break;
	default:
	{
		m_pLog->Error(Helper::Format("Event is no processed. [%s][%d].", Manager::Events_Name(event).c_str(), event));
	}
	break;
	}
}


SOCKET Sloong::SloongNetGateway::GetPorcessConnect(int function)
{
	if (!m_mapFuncToTemplateIDs.exist(function) && !m_mapFuncToTemplateIDs.exist(-1))
	{
		m_pLog->Warn(Helper::Format("Function to template map list no have function [%d] and universal processer. the map list size [%d]", function, m_mapFuncToTemplateIDs.size()));
		return INVALID_SOCKET;
	}

	for( auto tpl : m_mapFuncToTemplateIDs[function])
	{
		if (m_mapTempteIDToUUIDs[tpl].size() == 0)
			continue;

		for (auto node : m_mapTempteIDToUUIDs[tpl])
		{
			return m_mapUUIDToConnectionID[node];
		}
	}

	for( auto tpl : m_mapFuncToTemplateIDs[-1])
	{
		if (m_mapTempteIDToUUIDs[tpl].size() == 0)
			continue;

		for (auto node : m_mapTempteIDToUUIDs[tpl])
		{
			return m_mapUUIDToConnectionID[node];
		}
	}

	return INVALID_SOCKET;
}

