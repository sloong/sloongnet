/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2019-01-15 15:57:36
 * @LastEditTime: 2021-09-22 16:28:18
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/gateway/gateway.cpp
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

/* File Name: server.c */
#include "gateway.h"
#include "utility.h"
#include "IData.h"
#include "events/SendPackageToManager.hpp"
#include "events/RegisterConnection.hpp"
using namespace Sloong;
using namespace Sloong::Events;

#include "protocol/manager.pb.h"
using namespace Manager;

unique_ptr<SloongNetGateway> Sloong::SloongNetGateway::Instance = nullptr;

extern "C" PackageResult RequestPackageProcesser(void *env, Package *pack)
{
	auto pTranspond = STATIC_TRANS<GatewayTranspond *>(env);
	if (pTranspond)
		return pTranspond->RequestPackageProcesser(pack);
	else
		return PackageResult::Make_Error("RequestPackageProcesser error, Environment convert failed.");
}

extern "C" PackageResult ResponsePackageProcesser(void *env, Package *pack)
{
	auto num = pack->id();
	if (SloongNetGateway::Instance->m_mapSerialToRequest.exist(num))
	{
		auto pTranspond = STATIC_TRANS<GatewayTranspond *>(env);
		if (pTranspond)
		{
			auto info = SloongNetGateway::Instance->m_mapSerialToRequest.remove(num);
			return pTranspond->ResponsePackageProcesser(move(info), pack);
		}
		else
			return PackageResult::Make_Error("ResponsePackageProcesser error, Environment convert failed.");
	}

	return SloongNetGateway::Instance->ResponsePackageProcesser(pack);
}

extern "C" CResult EventPackageProcesser(Package *pack)
{
	SloongNetGateway::Instance->EventPackageProcesser(pack);
	return CResult::Succeed;
}

extern "C" CResult NewConnectAcceptProcesser(SOCKET sock)
{
	return CResult::Succeed;
}

extern "C" CResult ModuleInitialization(IControl *ic)
{
	SloongNetGateway::Instance = make_unique<SloongNetGateway>();
	return SloongNetGateway::Instance->Initialization(ic);
}

extern "C" CResult ModuleInitialized()
{
	return SloongNetGateway::Instance->Initialized();
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
	return SloongNetGateway::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult SloongNetGateway::Initialization(IControl *iC)
{
	IObject::Initialize(iC);
	IData::Initialize(iC);
	return CResult::Succeed;
}

CResult SloongNetGateway::Initialized()
{
	m_pConfig = IData::GetGlobalConfig();
	m_pModuleConfig = IData::GetModuleConfig();
	if (m_pModuleConfig)
	{
		/*auto event = make_shared<NormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableTimeoutCheck);
		event->SetMessage(format("{\"TimeoutTime\":\"{}\", \"CheckInterval\":{}}", (*m_pModuleConfig)["TimeoutTime"].asInt(), (*m_pModuleConfig)["TimeoutCheckInterval"].asInt()));
		m_iC->SendMessage(event);

		event = make_shared<NormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableClientCheck);
		event->SetMessage(format("{\"ClientCheckKey\":\"{}\", \"ClientCheckTime\":{}}", (*m_pModuleConfig)["ClientCheckKey"].asString(), (*m_pModuleConfig)["ClientCheckKey"].asInt()));
		m_iC->SendMessage(event);*/

		if (!m_pModuleConfig->isMember("forwards"))
		{
			return CResult::Make_Error("Configuration cannot empty");
		}

		auto forwards = m_pModuleConfig->operator[]("forwards");

		if (!(forwards.isMember("default") && forwards["default"].isInt()))
		{
			return CResult::Make_Error("Configuration forwards-default cannot empty");
		}

		_DefaultTemplateId = forwards["default"].asInt();

		if (forwards.isMember("range") && forwards["range"].isArray())
		{
			auto range = forwards["range"];
			for (int i = 0; i < range.size(); i++)
			{
				auto item = range[i];
				if (item["begin"].isInt() && item["end"].isInt() && item["forward_to"].isInt())
				{
					_FORWARD_RANE_INFO info;
					info.begin = item["begin"].asInt();
					info.end = item["end"].isInt();
					info.forward_to = item["forward_to"].asInt();
					m_mapforwardRangeInfo.emplace_back(info);
				}
				else
				{
					return CResult::Make_Error("Configuration error. the range must have begin/end/forward_to set. and type must be integer");
				}
			}
		}
		if (forwards.isMember("map") && forwards["map"].isArray())
		{
			auto map = forwards["map"];
			for (int i = 0; i < map.size(); i++)
			{
				auto item = map[i];
				if (item["source_function"].isInt() && item["map_function"].isInt() && item["forward_to"].isInt())
				{
					_FORWARD_MAP_INFO info;
					info.source_function = item["source_function"].asInt();
					info.map_function = item["map_function"].isInt();
					info.forward_to = item["forward_to"].asInt();
					m_mapForwardMapInfo.insert(info.source_function, move(info));
				}
				else
				{
					return CResult::Make_Error("Configuration error. the map item must have source_function/map_function/forward_to set. and type must be integer");
				}
			}
		}
	}
	m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStart, std::bind(&SloongNetGateway::OnStart, this, std::placeholders::_1));
	return CResult::Succeed;
}

PackageResult SloongNetGateway::ResponsePackageProcesser(Package *trans_pack)
{
	m_pLog->error("ResponsePackageProcesser no find the package. Ignore package.");
	return PackageResult::Ignore();
}

void SloongNetGateway::QueryReferenceInfo()
{
	auto event = make_shared<SendPackageToManagerEvent>(Functions::QueryReferenceInfo, "");
	event->SetCallbackFunc(std::bind(&SloongNetGateway::QueryReferenceInfoResponseHandler, this, std::placeholders::_1, std::placeholders::_2));
	m_iC->SendMessage(event);
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
			int start, end;
			if (!ConvertStrToInt(range[0], &start) || !ConvertStrToInt(range[1], &end))
				return res_list;
			for (int i = start; i <= end; i++)
			{
				res_list.push_back(i);
			}
		}
		else
		{
			int nFunc;
			if (!ConvertStrToInt(func, &nFunc))
				return res_list;
			res_list.push_back(nFunc);
		}
	}
	return res_list;
}

void SloongNetGateway::QueryReferenceInfoResponseHandler(IEvent *send_pack, Package *res_pack)
{
	auto str_res = res_pack->content();
	auto res = ConvertStrToObj<QueryReferenceInfoResponse>(str_res);
	if (res == nullptr || res->templateinfos_size() == 0)
		return;

	auto templateInfos = res->templateinfos();
	for (auto info : templateInfos)
	{
		// if (info.providefunctions() == "*")
		// {
		// 	m_pLog->debug(format("Universal processer find: template id[{}]", info.templateid()));
		// 	m_mapFuncToTemplateIDs[-1].unique_insert(info.templateid());
		// }
		// else
		// {
		// 	for (auto i : ProcessProviedFunction(info.providefunctions()))
		// 		m_mapFuncToTemplateIDs[i].unique_insert(info.templateid());
		// }
		for (auto item : info.nodeinfos())
		{
			m_mapUUIDToNode[item.uuid()] = item;
			m_mapTempteIDToUUIDs.insert( info.templateid(), list_ex<uint64_t>() );
			m_mapTempteIDToUUIDs.get(info.templateid()).push_back(item.uuid());

			AddConnection(item.uuid(), item.address(), item.port());
		}
	}
}

void SloongNetGateway::AddConnection(uint64_t uuid, const string &addr, int port)
{
	auto event = make_shared<RegisterConnectionEvent>(addr, port);
	event->SetCallbackFunc([this, uuid](IEvent *e, uint64_t hashcode)
						   { m_mapUUIDToConnectionID[uuid] = hashcode; });
	m_iC->SendMessage(event);
}

CResult SloongNetGateway::CreateProcessEnvironmentHandler(void **out_env)
{
	auto item = make_unique<GatewayTranspond>();
	auto res = item->Initialize(m_iC);
	if (res.IsFialed())
		return res;
	(*out_env) = item.get();
	m_listTranspond.emplace_back(std::move(item));
	return CResult::Succeed;
}

void SloongNetGateway::OnStart(SharedEvent evt)
{
	QueryReferenceInfo();
}

void Sloong::SloongNetGateway::OnReferenceModuleOnlineEvent(const string &str_req, Package *trans_pack)
{
	auto req = ConvertStrToObj<Manager::EventReferenceModuleOnline>(str_req);
	auto item = req->item();
	m_mapUUIDToNode[item.uuid()] = item;
	auto uuids = m_mapTempteIDToUUIDs.try_get(item.templateid());
	if( uuids == nullptr ) 
	{
		m_pLog->warn(format("OnReferenceModuleOnlineEvent: not found info with template id: {}",item.templateid()));
	}
	uuids->push_back(item.uuid());
	m_pLog->info(format("New node[{}][{}:{}] is online:templateid[{}],list size[{}]", item.uuid(), item.address(), item.port(), item.templateid(), uuids->size()));

	AddConnection(item.uuid(), item.address(), item.port());
}

void Sloong::SloongNetGateway::OnReferenceModuleOfflineEvent(const string &str_req, Package *trans_pack)
{
	auto req = ConvertStrToObj<Manager::EventReferenceModuleOffline>(str_req);
	auto uuid = req->uuid();
	auto item = m_mapUUIDToNode[uuid];

	auto uuids = m_mapTempteIDToUUIDs.try_get(item.templateid());
	if( uuids == nullptr ) {
		m_pLog->warn(format("OnReferenceModuleOfflineEvent: not found info with template id: {}",item.templateid()));
	}

	uuids->erase(uuid);

	m_mapUUIDToConnectionID.erase(uuid);
	m_mapUUIDToNode.erase(uuid);
	m_pLog->info(format("Node is offline [{}], template id[{}],list size[{}]", item.uuid(), item.templateid(), uuids->size()));
}

void Sloong::SloongNetGateway::EventPackageProcesser(Package *pack)
{
	auto event = (Manager::Events)pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->error(format("EventPackageProcesser is called.but the fucntion[{}] check error.", event));
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
		m_pLog->error(format("Event is no processed. [{}][{}].", Manager::Events_Name(event), event));
	}
	break;
	}
}

U64Result Sloong::SloongNetGateway::GetPorcessConnection(int function)
{
	auto forward_to = -1;
	if (m_mapForwardMapInfo.exist(function))
	{
		forward_to = m_mapForwardMapInfo.get(function).forward_to;
	}
	else
	{
		for (auto &i : m_mapforwardRangeInfo)
		{
			if (i.InRange(function))
			{
				forward_to = i.forward_to;
				break;
			}
		}
	}

	if (forward_to == -1)
	{
		forward_to = _DefaultTemplateId;
	}

	if (m_mapTempteIDToUUIDs.exist(forward_to))
	{
		m_pLog->error(format("Function [{}] has matching for forward to template [{}], but not found infomation in the template info list"));
		return U64Result::Make_Error("Forward error: not found target infomation.");
	}

	if (m_mapTempteIDToUUIDs.get(forward_to).size() == 0)
	{
		m_pLog->error(format("Function [{}] has matching for forward to template [{}], but the template no has nodes."));
		return U64Result::Make_Error("Forward error: not node infomation.");
	}

	for (auto node : m_mapTempteIDToUUIDs.get(forward_to))
	{
		// TODO: should be check the node loading.
		return U64Result::Make_OKResult(m_mapUUIDToConnectionID[node]);
	}
}
