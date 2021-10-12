/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2019-01-15 15:57:36
 * @LastEditTime: 2021-09-24 11:03:39
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
#include "IData.h"
#include "events/RegisterConnection.hpp"
#include "events/SendPackageToManager.hpp"
#include "utility.h"

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
        try
        {
            /*auto event = make_shared<NormalEvent>();
            event->SetEvent(EVENT_TYPE::EnableTimeoutCheck);
            event->SetMessage(format("{\"TimeoutTime\":\"{}\", \"CheckInterval\":{}}",
            (*m_pModuleConfig)["TimeoutTime"].asInt(), (*m_pModuleConfig)["TimeoutCheckInterval"].asInt()));
            m_iC->SendMessage(event);

            event = make_shared<NormalEvent>();
            event->SetEvent(EVENT_TYPE::EnableClientCheck);
            event->SetMessage(format("{\"ClientCheckKey\":\"{}\", \"ClientCheckTime\":{}}",
            (*m_pModuleConfig)["ClientCheckKey"].asString(), (*m_pModuleConfig)["ClientCheckKey"].asInt()));
            m_iC->SendMessage(event);*/

            if (!m_pModuleConfig->isMember("forwarders"))
            {
                return CResult::Make_Error("Configuration cannot empty");
            }

            auto forwarders = m_pModuleConfig->operator[]("forwarders");

            if (!CheckJsonInt(forwarders, "default"))
            {
                return CResult::Make_Error("Configuration forwards-default cannot empty");
            }

            _DefaultTemplateId = GetJsonInt(forwarders, "default");

            if (forwarders.isMember("range") && forwarders["range"].isArray())
            {
                auto range = forwarders["range"];
                for (int i = 0; i < range.size(); i++)
                {
                    auto item = range[i];
                    if (CheckJsonInt(item, "begin") && CheckJsonInt(item, "end") && CheckJsonInt(item, "forward_to"))
                    {
                        _FORWARD_RANE_INFO info;
                        info.begin = GetJsonInt(item, "begin");
                        info.end = GetJsonInt(item, "end");
                        info.forward_to = GetJsonInt(item, "forward_to");
                        if (item.isMember("convert_rule"))
                        {
                            auto convert_rule = item["convert_rule"];
                            if (!CheckJsonString(convert_rule, "direction") || !CheckJsonInt(convert_rule, "offset"))
                            {
                                return CResult::Make_Error("Configuration error. the convert_rule is enabled but the "
                                                           "direction/offset is not set or type error.");
                            }
                            auto direction = convert_rule["direction"].asString();
                            auto offset = GetJsonInt(convert_rule, "offset");
                            if (direction == "+")
                            {
                                info.converter = [direction, offset](int funcid) { return funcid + offset; };
                            }
                            else if (direction == "-")
                            {
                                info.converter = [direction, offset](int funcid) { return funcid - offset; };
                            }
                            else
                            {
                                return CResult::Make_Error(
                                    format("Configuration error. the direction only can be '-' or '+'"));
                            }
                        }

                        m_mapforwardRangeInfo.emplace_back(info);
                    }
                    else
                    {
                        return CResult::Make_Error("Configuration error. the range must have begin/end/forward_to set. "
                                                   "and type must be integer");
                    }
                }
            }
            if (forwarders.isMember("single") && forwarders["single"].isArray())
            {
                auto single = forwarders["single"];
                for (int i = 0; i < single.size(); i++)
                {
                    auto item = single[i];
                    if (CheckJsonInt(item, "forward_to") && item.isMember("maps") && item["maps"].isArray())
                    {
                        auto maps = item["maps"];
                        auto forward_to = GetJsonInt(item, "forward_to");
                        for (auto k : maps.getMemberNames())
                        {
                            int func = 0;
                            if (ConvertStrToInt(k, &func) || !CheckJsonInt(maps, k))
                                return CResult::Make_Error(
                                    "Configuration error. the single maps key and value must be a integers");

                            _FORWARD_MAP_INFO info;
                            info.from = func;
                            info.to = GetJsonInt(maps, k);
                            info.forward_to = forward_to;
                            m_mapForwardMapInfo.insert(func, move(info));
                        }
                    }
                    else
                    {
                        return CResult::Make_Error(
                            "Configuration error. the single element must have maps(array)/forward_to(int) set.");
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            return CResult::Make_Error(format("Process module configuration error. {}", e.what()));
        }
    }
    m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStart,
                               std::bind(&SloongNetGateway::OnStart, this, std::placeholders::_1));
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
    event->SetCallbackFunc(std::bind(&SloongNetGateway::QueryReferenceInfoResponseHandler, this, std::placeholders::_1,
                                     std::placeholders::_2));
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
            m_mapTempteIDToUUIDs.insert(info.templateid(), list_ex<uint64_t>());
            m_mapTempteIDToUUIDs.get(info.templateid()).push_back(item.uuid());

            AddConnection(item.uuid(), item.address(), item.port());
        }
    }
}

void SloongNetGateway::AddConnection(uint64_t uuid, const string &addr, int port)
{
    auto event = make_shared<RegisterConnectionEvent>(addr, port);
    event->SetCallbackFunc([this, uuid](IEvent *e, uint64_t hashcode) { m_mapUUIDToConnectionID[uuid] = hashcode; });
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
    if (uuids == nullptr)
    {
        m_pLog->warn(format("OnReferenceModuleOnlineEvent: not found info with template id: {}", item.templateid()));
    }
    uuids->push_back(item.uuid());
    m_pLog->info(format("New node[{}][{}:{}] is online:templateid[{}],list size[{}]", item.uuid(), item.address(),
                        item.port(), item.templateid(), uuids->size()));

    AddConnection(item.uuid(), item.address(), item.port());
}

void Sloong::SloongNetGateway::OnReferenceModuleOfflineEvent(const string &str_req, Package *trans_pack)
{
    auto req = ConvertStrToObj<Manager::EventReferenceModuleOffline>(str_req);
    auto uuid = req->uuid();
    auto item = m_mapUUIDToNode[uuid];

    auto uuids = m_mapTempteIDToUUIDs.try_get(item.templateid());
    if (uuids == nullptr)
    {
        m_pLog->warn(format("OnReferenceModuleOfflineEvent: not found info with template id: {}", item.templateid()));
    }

    uuids->erase(uuid);

    m_mapUUIDToConnectionID.erase(uuid);
    m_mapUUIDToNode.erase(uuid);
    m_pLog->info(
        format("Node is offline [{}], template id[{}],list size[{}]", item.uuid(), item.templateid(), uuids->size()));
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
    case Manager::Events::ReferenceModuleOnline: {
        OnReferenceModuleOnlineEvent(pack->content(), pack);
    }
    break;
    case Manager::Events::ReferenceModuleOffline: {
        OnReferenceModuleOfflineEvent(pack->content(), pack);
    }
    break;
    default: {
        m_pLog->error(format("Event is no processed. [{}][{}].", Manager::Events_Name(event), event));
    }
    break;
    }
}

FWResult Sloong::SloongNetGateway::GetForwardInfo(int function)
{
    auto forward_to = -1;
    auto new_function = function;
    if (m_mapForwardMapInfo.exist(function))
    {
        auto item = m_mapForwardMapInfo.get(function);
        forward_to = item.forward_to;
        new_function = item.to;
    }
    else
    {
        for (auto &i : m_mapforwardRangeInfo)
        {
            if (i.InRange(function))
            {
                forward_to = i.forward_to;
                if (i.converter != nullptr)
                    new_function = i.converter(function);
                break;
            }
        }
    }

    if (forward_to == -1)
    {
        forward_to = _DefaultTemplateId;
    }

    if (!m_mapTempteIDToUUIDs.exist(forward_to))
    {
        m_pLog->error(format("Function [{}] has matching for forward to template [{}], but not found infomation in the "
                             "template info list"));
        return FWResult::Make_Error("Forward error: not found target infomation.");
    }

    if (m_mapTempteIDToUUIDs.get(forward_to).size() == 0)
    {
        m_pLog->error(
            format("Function [{}] has matching for forward to template [{}], but the template no has nodes."));
        return FWResult::Make_Error("Forward error: not node infomation.");
    }

    for (auto node : m_mapTempteIDToUUIDs.get(forward_to))
    {
        // TODO: should be check the node loading.
        return FWResult::Make_OKResult(
            _FORWARD_INFO{.connection_id = m_mapUUIDToConnectionID[node], .function_id = new_function});
    }

    auto node = m_mapTempteIDToUUIDs.get(forward_to).begin();
    return FWResult::Make_OKResult(
        _FORWARD_INFO{.connection_id = m_mapUUIDToConnectionID[*node], .function_id = new_function});
}
