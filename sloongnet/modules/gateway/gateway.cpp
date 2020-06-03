/* File Name: server.c */
#include "gateway.h"
#include "utility.h"
#include "IData.h"
#include "SendMessageEvent.hpp"
using namespace Sloong;
using namespace Sloong::Events;

#include "protocol/manager.pb.h"
using namespace Manager;

#include "snowflake.h"


unique_ptr<SloongNetGateway> Sloong::SloongNetGateway::Instance = nullptr;

extern "C" CResult RequestPackageProcesser(void *env, CDataTransPackage *pack)
{
	auto pTranspond = TYPE_TRANS<GatewayTranspond*>(env);
	if( pTranspond)
		return pTranspond->RequestPackageProcesser(pack);
	else
		return CResult::Make_Error("RequestPackageProcesser error, Environment convert failed.");
}

extern "C" CResult ResponsePackageProcesser(void *env, CDataTransPackage *pack)
{
	auto num = pack->GetSerialNumber();
	if( SloongNetGateway::Instance->m_mapSerialToRequest.exist(num) )
	{
		auto pTranspond = TYPE_TRANS<GatewayTranspond*>(env);
		if( pTranspond)
			return pTranspond->ResponsePackageProcesser(&SloongNetGateway::Instance->m_mapSerialToRequest[num],pack);
		else
			return CResult::Make_Error("ResponsePackageProcesser error, Environment convert failed.");
	}

	return SloongNetGateway::Instance->ResponsePackageProcesser(pack);
}

extern "C" CResult EventPackageProcesser(CDataTransPackage *pack)
{
	SloongNetGateway::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
} 

extern "C" CResult NewConnectAcceptProcesser(CSockInfo *info)
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
		auto event = make_unique<CNormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableTimeoutCheck);
		event->SetMessage(Helper::Format("{\"TimeoutTime\":\"%d\", \"CheckInterval\":%d}", (*m_pModuleConfig)["TimeoutTime"].asInt(), (*m_pModuleConfig)["TimeoutCheckInterval"].asInt()));
		m_iC->SendMessage(std::move(event));

		event = make_unique<CNormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableClientCheck);
		event->SetMessage(Helper::Format("{\"ClientCheckKey\":\"%s\", \"ClientCheckTime\":%d}", (*m_pModuleConfig)["ClientCheckKey"].asString().c_str(), (*m_pModuleConfig)["ClientCheckKey"].asInt()));
		m_iC->SendMessage(std::move(event));
	}
	m_nManagerConnection = sock;
	m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStart, std::bind(&SloongNetGateway::OnStart, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::SocketClose, std::bind(&SloongNetGateway::OnSocketClose, this, std::placeholders::_1));
	m_iC->RegisterEventHook(EVENT_TYPE::SendPackage, std::bind(&SloongNetGateway::SendPackageHook, this, std::placeholders::_1));
	return CResult::Succeed();
}


CResult SloongNetGateway::ResponsePackageProcesser( CDataTransPackage *trans_pack)
{
	auto num = trans_pack->GetSerialNumber();
	if (!m_listSendEvent.exist(num))
	{
		m_pLog->Error("ResponsePackageProcesser no find the package.");
		return CResult::Make_Error("ResponsePackageProcesser no find the package.");
	}
		

	auto send_evt = TYPE_TRANS<CSendPackageEvent*>(m_listSendEvent[num].get());
	auto need_send_res = send_evt->CallCallbackFunc(trans_pack);
	m_listSendEvent.erase(num);
	return need_send_res;
}


void SloongNetGateway::QueryReferenceInfo()
{
	auto event = make_unique<CSendPackageEvent>();
	event->SetCallbackFunc(std::bind(&SloongNetGateway::QueryReferenceInfoResponseHandler, this, std::placeholders::_1, std::placeholders::_2));
	event->SetRequest(m_nManagerConnection, m_pRuntimeData->nodeuuid(), snowflake::Instance->nextid(), Core::HEIGHT_LEVEL, (int)Functions::QueryReferenceInfo, "");
	m_iC->CallMessage(std::move(event));
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

CResult SloongNetGateway::QueryReferenceInfoResponseHandler(IEvent *send_pack, CDataTransPackage *res_pack)
{
	auto str_res = res_pack->GetRecvMessage();
	auto res = ConvertStrToObj<QueryReferenceInfoResponse>(str_res);
	if (res == nullptr || res->templateinfos_size() == 0)
		return CResult::Invalid();

	auto templateInfos = res->templateinfos();
	for (auto info : templateInfos)
	{
		if (info.providefunctions() == "*")
			m_mapFuncToTemplateIDs[-1].unique_insert(info.templateid());
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
	return CResult::Invalid();
}

void SloongNetGateway::AddConnection( uint64_t uuid, const string &addr, int port)
{
	EasyConnect conn;
	conn.Initialize(addr, port);
	conn.Connect();
	auto event = make_unique<CNetworkEvent>(EVENT_TYPE::RegisteConnection);
	event->SetSocketID(conn.GetSocketID());
	m_iC->SendMessage(std::move(event));
	m_mapUUIDToConnect[uuid] = conn.GetSocketID();
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

void SloongNetGateway::SendPackageHook(UniqueEvent event)
{
	auto send_evt = TYPE_TRANS<CSendPackageEvent*>(event.get());
	auto pack = send_evt->GetDataPackage();
	m_listSendEvent[pack->id()] = std::move(event);
}

void SloongNetGateway::OnStart(IEvent* evt)
{
	QueryReferenceInfo();
}

void Sloong::SloongNetGateway::OnSocketClose(IEvent* event)
{

}

void Sloong::SloongNetGateway::OnReferenceModuleOnlineEvent(const string &str_req, CDataTransPackage *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOnline event");
	auto req = ConvertStrToObj<Manager::EventReferenceModuleOnline>(str_req);
	auto item = req->item();
	m_mapUUIDToNode[item.uuid()] = item;
	m_mapTempteIDToUUIDs[item.templateid()].push_back(item.uuid());
	m_pLog->Debug(Helper::Format("New module is online:[%llu][%s:%d]", item.uuid(), item.address().c_str(), item.port()));

	AddConnection(item.uuid(), item.address(), item.port());
}

void Sloong::SloongNetGateway::OnReferenceModuleOfflineEvent(const string &str_req, CDataTransPackage *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOffline event");
	auto req = ConvertStrToObj<Manager::EventReferenceModuleOffline>(str_req);
	auto uuid = req->uuid();
	auto item = m_mapUUIDToNode[uuid];
	m_mapTempteIDToUUIDs[item.templateid()].erase(item.uuid());
	auto event = make_unique<CNetworkEvent>(EVENT_TYPE::SocketClose);
	event->SetSocketID(m_mapUUIDToConnect[uuid]);
	m_iC->SendMessage(std::move(event));
	m_mapUUIDToConnect.erase(uuid);
	m_mapUUIDToNode.erase(uuid);
}

void Sloong::SloongNetGateway::EventPackageProcesser(CDataTransPackage *trans_pack)
{
	auto data_pack = trans_pack->GetDataPackage();
	auto event = (Manager::Events)data_pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->Error(Helper::Format("EventPackageProcesser is called.but the fucntion[%d] check error.", event));
		return;
	}

	switch (event)
	{
	case Manager::Events::ReferenceModuleOnline:
	{
		OnReferenceModuleOnlineEvent(data_pack->content(), trans_pack);
	}
	break;
	case Manager::Events::ReferenceModuleOffline:
	{
		OnReferenceModuleOfflineEvent(data_pack->content(), trans_pack);
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
		return INVALID_SOCKET;
	}

	for( auto tpl : m_mapFuncToTemplateIDs[function])
	{
		if (m_mapTempteIDToUUIDs[tpl].size() == 0)
			continue;

		for (auto node : m_mapTempteIDToUUIDs[tpl])
		{
			return m_mapUUIDToConnect[node];
		}
	}

	for( auto tpl : m_mapFuncToTemplateIDs[-1])
	{
		if (m_mapTempteIDToUUIDs[tpl].size() == 0)
			continue;

		for (auto node : m_mapTempteIDToUUIDs[tpl])
		{
			return m_mapUUIDToConnect[node];
		}
	}

	return INVALID_SOCKET;
}

