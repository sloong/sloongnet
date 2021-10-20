/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-04-29 09:27:21
 * @LastEditTime: 2021-10-20 13:53:28
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

#include "snowflake.h"
#include "utility.h"

#include "events/GetConnectionInfo.hpp"
#include "events/SendPackage.hpp"

using namespace Sloong::Events;

CResult Sloong::CServerManage::Initialize(IControl *ic, const string &db_path)
{
    IObject::Initialize(ic);

    m_mapFuncToHandler[Functions::PostLog] =
        std::bind(&CServerManage::EventRecorderHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::RegisterWorker] =
        std::bind(&CServerManage::RegisterWorkerHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::RegisterNode] =
        std::bind(&CServerManage::RegisterNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::AddTemplate] =
        std::bind(&CServerManage::AddTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::DeleteTemplate] =
        std::bind(&CServerManage::DeleteTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::SetTemplate] =
        std::bind(&CServerManage::SetTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::QueryTemplate] =
        std::bind(&CServerManage::QueryTemplateHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::QueryNode] =
        std::bind(&CServerManage::QueryNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::QueryReferenceInfo] =
        std::bind(&CServerManage::QueryReferenceInfoHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::StopNode] =
        std::bind(&CServerManage::StopNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::RestartNode] =
        std::bind(&CServerManage::RestartNodeHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::ReportLoadStatus] =
        std::bind(&CServerManage::ReportLoadStatusHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::ReconnectRegister] =
        std::bind(&CServerManage::ReconnectRegisterHandler, this, std::placeholders::_1, std::placeholders::_2);

    if (!CConfiguration::Instance->IsInituialized())
    {
        auto res = CConfiguration::Instance->Initialize(db_path);
        if (res.IsFialed())
            return res;
    }

    // Initialize template list
    auto list = CConfiguration::Instance->GetTemplateList();
    for (auto &item : list)
    {
        TemplateItem addItem(item);
        m_mapIDToTemplateItem.insert(addItem.ID, addItem);
        RefreshModuleReference(addItem.ID);
    }
    m_mapIDToTemplateItem.get(1).Created.push_back(0);
    return CResult::Succeed;
}

CResult Sloong::CServerManage::LoadManagerConfig(const string &db_path)
{
    if (!CConfiguration::Instance->IsInituialized())
    {
        auto res = CConfiguration::Instance->Initialize(db_path);
        if (res.IsFialed())
            return res;
    }
    auto res = CConfiguration::Instance->GetTemplate(1);
    if (res.IsFialed())
        return CResult(ResultType::Warning);
    return CResult::Make_OK(
        string(res.GetResultObject().configuration.begin(), res.GetResultObject().configuration.end()));
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
    item.Configuration = config_str;
    item.BuildCache();
    CResult res(ResultType::Succeed);
    auto info = item.ToTemplateInfo();
    if (CConfiguration::Instance->CheckTemplateExist(1))
        res = CConfiguration::Instance->SetTemplate(1, info);
    else
        res = CConfiguration::Instance->AddTemplate(info, nullptr);
    return res;
}

bool Sloong::CServerManage::CheckTemplateIsRegistered(int id)
{
    std::erase_if(m_mapRegisterdUUIDToInfo, [](auto &it) { return difftime(time(NULL), it.second.registedTime) > 1; });

    for (auto i = m_mapRegisterdUUIDToInfo.begin(); i != m_mapRegisterdUUIDToInfo.end(); i++)
    {
        if ((*i).second.templateID == id)
        {
            return true;
        }
    }
    return false;
}

int Sloong::CServerManage::SearchNeedCreateTemplate()
{
    vector<int> all_ids;
    std::for_each(m_mapIDToTemplateItem.begin(), m_mapIDToTemplateItem.end(),
                  [&all_ids](auto &item) { all_ids.push_back(item.first); });
    return SearchNeedCreateWithIDs(all_ids);
}

int Sloong::CServerManage::SearchNeedCreateWithIDs(const vector<int> &ids)
{
    if (ids.size() == 0)
        return 0;

    // First time find the no created
    for (auto index : ids)
    {
        if (CheckTemplateNeedCreate(index, true))
            return index;
        else
            continue;
    }

    // Sencond time find the created < replicas
    for (auto index : ids)
    {
        if (CheckTemplateNeedCreate(index, false))
            return index;
        else
            continue;
    }
    return 0;
}

bool Sloong::CServerManage::CheckTemplateNeedCreate(int index, bool onlyEmpty)
{
    auto item = m_mapIDToTemplateItem.get(index);
    if (item.Replicas == 0 || item.ID == 1)
        return false;

    if ((int)item.Created.size() >= item.Replicas)
        return false;

    if ((onlyEmpty && item.Created.size() == 0) || (!onlyEmpty && (int)item.Created.size() < item.Replicas))
        if (CheckTemplateIsRegistered(index))
            return false;

    return true;
}

/***
 * @description: Convert type to template id.
 */
int Sloong::CServerManage::SearchNeedCreateWithType(bool excludeMode, const vector<int> &type)
{
    if (type.size() == 0)
        return 0;

    auto ids = vector<int>();
    auto includeMode = !excludeMode;
    for (auto item : m_mapIDToTemplateItem)
    {
        int item_id = item.second.ConfigurationObj->moduletype();
        bool exist = std::find(type.begin(), type.end(), item_id) != type.end();
        // In exclude mode, the current item no exist in the type map. Or
        // in include mode, the current item is in the type map.
        if ((exist && includeMode) || (!exist && excludeMode))
        {
            ids.push_back(item.first);
        }
    }

    return SearchNeedCreateWithIDs(ids);
}

void Sloong::CServerManage::SendManagerEvent(const list<uint64_t> &notifyList, Manager::Events event,
                                             ::google::protobuf::Message *msg)
{
    for (auto item : notifyList)
    {
        string msg_str;
        if (msg)
            msg->SerializeToString(&msg_str);
        auto req = make_unique<SendPackageEvent>(m_mapUUIDToNodeItem[item].ConnectionHashCode);
        req->SetEvent(event, msg_str, DataPackage_PackageType::DataPackage_PackageType_ManagerEvent);
        m_iC->SendMessage(std::move(req));
    }
}

void Sloong::CServerManage::SendControlEvent(const list<uint64_t> &notifyList, Core::ControlEvent event,
                                             const string &msg)
{
    for (auto item : notifyList)
    {
        auto req = make_unique<SendPackageEvent>(m_mapUUIDToNodeItem[item].ConnectionHashCode);
        req->SetEvent(event, msg, DataPackage_PackageType::DataPackage_PackageType_ControlEvent);
        m_iC->SendMessage(std::move(req));
    }
}

void Sloong::CServerManage::OnSocketClosed(uint64_t con)
{
    if (!m_mapConnectionToUUID.exist(con))
        return;

    auto target = m_mapConnectionToUUID[con];
    auto tpl = m_mapUUIDToNodeItem.try_get(target);
    if (tpl == nullptr)
    {
        m_pLog->error("Connection is already closed, but not found the node infomation");
        return;
    }
    auto id = tpl->TemplateID;

    auto tplItem = m_mapIDToTemplateItem.try_get(id);
    if (tplItem == nullptr)
    {
        m_pLog->error(format("cannot find the template item with id {}", id));
        return;
    }

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
        SendManagerEvent(notifyList, Manager::Events::ReferenceModuleOffline, &offline_event);
    }
    m_mapUUIDToNodeItem.erase(target);
    tplItem->Created.remove(target);
}

PackageResult Sloong::CServerManage::ProcessHandler(Package *pack)
{
    auto function = (Functions)pack->function();
    if (!Manager::Functions_IsValid(function))
    {
        return PackageResult::Make_OKResult(
            Package::MakeErrorResponse(pack, format("Parser request package function[{}] error.", pack->content())));
    }

    auto req_str = pack->content();

    auto uuid = m_mapConnectionToUUID.try_get(pack->sessionid());
    if (uuid != nullptr)
    {
        auto node = m_mapUUIDToNodeItem.try_get(*uuid);
        if (node != nullptr)
        {
            node->Active();
        }
    }

    auto func_name = Functions_Name(function);
    m_pLog->info(format("Request [{}][{}]", function, func_name));
    if (!m_mapFuncToHandler.exist(function))
    {
        return PackageResult::Make_OKResult(
            Package::MakeErrorResponse(pack, format("Function [{}] no handler.", func_name)));
    }

    auto res = m_mapFuncToHandler[function](req_str, pack);
    if (res.IsError())
        m_pLog->warn(format("Response [{}]:[{}][{}].", func_name, ResultType_Name(res.GetResult()), res.GetMessage()));
    else
        m_pLog->info(format("Response [{}]:[{}]", func_name, ResultType_Name(res.GetResult())));
    if (res.GetResult() == ResultType::Ignore)
        return PackageResult::Ignore();

    return PackageResult::Make_OKResult(Package::MakeResponse(pack, res));
}

CResult Sloong::CServerManage::EventRecorderHandler(const string &req_str, Package *pack)
{

    return CResult::Succeed;
}

// TODO 在同时接收到多个请求时，会返回同一个templateid，即使其副本设置为1.
// 导致后面的RegisterNote请求必定只有一个可以成功。
CResult Sloong::CServerManage::RegisterWorkerHandler(const string &req_str, Package *pack)
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
        auto event = make_shared<GetConnectionInfoEvent>(pack->sessionid());
        event->SetCallbackFunc(
            [item = &m_mapUUIDToNodeItem[sender]](IEvent *e, ConnectionInfo info) { item->Address = info.Address; });
        m_iC->SendMessage(event);
        m_pLog->debug(format("New module register to system. Allocating uuid [{}].", item.UUID));
        char m_pMsgBuffer[8] = {0};
        char *pCpyPoint = m_pMsgBuffer;
        Helper::Int64ToBytes(sender, pCpyPoint);
        return CResult(ResultType::Retry, string(m_pMsgBuffer, 8));
    }

    int index = 0;
    if (req_str.length() > 0)
    {
        auto req = ConvertStrToObj<RegisterWorkerRequest>(req_str);
        switch (req->runmode())
        {
        case RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_AssignTemplate:
            index = SearchNeedCreateWithIDs(
                vector<int>(req->assigntargettemplateid().begin(), req->assigntargettemplateid().end()));
            break;
        case RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_IncludeType: {
            vector<int> l;
            for (auto i : req->includetargettype())
            {
                l.push_back(i);
            }
            index = SearchNeedCreateWithType(false, l);
        }
        break;
        case RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_ExcludeType: {
            vector<int> l;
            for (auto i : req->excludetargettype())
            {
                l.push_back(i);
            }
            index = SearchNeedCreateWithType(true, l);
        }
        break;
        default:
            index = SearchNeedCreateTemplate();
            break;
        }
    }
    else
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

    auto tpl = m_mapIDToTemplateItem.try_get(index);
    if (tpl == nullptr)
    {
        return CResult(ResultType::Retry, "Allocating type no exist.");
    }

    auto id = snowflake::Instance->nextid();

    RegisterNodeInfo info;
    info.registedTime = time(NULL);
    info.templateID = tpl->ID;

    m_mapRegisterdUUIDToInfo.insert(id, info);

    RegisterWorkerResponse res;
    res.set_registerid(id);
    res.set_templateid(tpl->ID);
    res.set_configuration(tpl->Configuration);

    m_pLog->debug(format("Allocating module[{}][{}] Type to [{}]", sender_info->UUID, id, tpl->Name));
    return CResult::Make_OK(ConvertObjToStr(&res));
}

void Sloong::CServerManage::RefreshModuleReference(int id)
{
    auto info = m_mapIDToTemplateItem.try_get(id);
    if (info == nullptr)
        return;
    info->Reference.clear();
    auto references = Helper::split(info->ConfigurationObj->modulereference(), ',');
    for (auto &item : references)
    {
        int id;
        if (ConvertStrToInt(item, &id))
            info->Reference.push_back(id);
    }
}

CResult Sloong::CServerManage::RegisterNodeHandler(const string &req_str, Package *pack)
{
    auto sender = pack->sender();
    auto req = ConvertStrToObj<RegisterNodeRequest>(req_str);
    if (!req || sender == 0)
        return CResult::Make_Error("The required parameter check error.");

    if (!m_mapUUIDToNodeItem.exist(sender))
        return CResult::Make_Error(format("The sender [{}] is no regitser.", sender));

    auto id = req->registerid();

    auto info = m_mapRegisterdUUIDToInfo.try_get(id);
    if (info == nullptr)
        return CResult::Make_Error(format("The register id [{}] is invalid.", id));

    if (info->templateID == 1)
        return CResult::Make_Error("Template id error.");

    auto tpl = m_mapIDToTemplateItem.try_get(info->templateID);
    if (tpl == nullptr)
        return CResult::Make_Error(format("The template id [{}] is no exist.", id));

    if (tpl->Created.size() >= tpl->Replicas)
    {
        return CResult(ResultType::Retry,
                       format("Target template is no need a new node. Retry with [RegisterWorker] request."));
    }

    // Save node info.
    auto &item = m_mapUUIDToNodeItem[sender];
    item.TemplateName = tpl->Name;
    item.TemplateID = tpl->ID;
    item.Port = tpl->ConfigurationObj->listenport();
    item.ConnectionHashCode = pack->sessionid();
    tpl->Created.unique_insert(sender);
    m_mapConnectionToUUID[pack->sessionid()] = sender;

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
        SendManagerEvent(notifyList, Manager::Events::ReferenceModuleOnline, &online_event);
    }

    return CResult::Succeed;
}

CResult Sloong::CServerManage::AddTemplateHandler(const string &req_str, Package *pack)
{
    auto req = ConvertStrToObj<AddTemplateRequest>(req_str);
    auto info = req->addinfo();
    TemplateItem item;
    item.ID = 0;
    item.Name = info.name();
    item.Note = info.note();
    item.Replicas = info.replicas();
    item.Configuration = info.configuration();
    item.BuildCache();
    if (!item.IsValid())
        return CResult::Make_Error("Param is valid.");

    int id = 0;
    auto res = CConfiguration::Instance->AddTemplate(item.ToTemplateInfo(), &id);
    if (res.IsFialed())
    {
        return res;
    }
    item.ID = id;

    m_mapIDToTemplateItem.insert(id, item);
    RefreshModuleReference(id);
    return CResult::Succeed;
}

CResult Sloong::CServerManage::DeleteTemplateHandler(const string &req_str, Package *pack)
{
    auto req = ConvertStrToObj<DeleteTemplateRequest>(req_str);

    int id = req->templateid();
    if (!m_mapIDToTemplateItem.exist(id))
    {
        return CResult::Make_Error(format("The template id [{}] is no exist.", id));
    }
    if (id == 1)
    {
        return CResult::Make_Error("Cannot delete this template.");
    }

    auto res = CConfiguration::Instance->DeleteTemplate(id);
    if (res.IsFialed())
    {
        return res;
    }
    m_mapIDToTemplateItem.erase(id);
    return CResult::Succeed;
}

CResult Sloong::CServerManage::SetTemplateHandler(const string &req_str, Package *pack)
{
    auto req = ConvertStrToObj<SetTemplateRequest>(req_str);
    auto info = req->setinfo();

    auto tplInfo = m_mapIDToTemplateItem.try_get(info.id());
    if (tplInfo == nullptr)
    {
        return CResult::Make_Error("Check the templeate ID error, please check.");
    }

    if (info.name().size() > 0)
        tplInfo->Name = info.name();

    if (info.note().size() > 0)
        tplInfo->Note = info.note();

    if (info.replicas() > 0)
        tplInfo->Replicas = info.replicas();

    if (info.configuration().size() > 0)
        tplInfo->Configuration = info.configuration();

    tplInfo->BuildCache();
    auto res = CConfiguration::Instance->SetTemplate(tplInfo->ID, tplInfo->ToTemplateInfo());
    if (res.IsFialed())
        return res;

    RefreshModuleReference(info.id());
    SendControlEvent(m_mapIDToTemplateItem.get(info.id()).Created, Core::ControlEvent::Restart, string());

    return CResult::Succeed;
}

CResult Sloong::CServerManage::QueryTemplateHandler(const string &req_str, Package *pack)
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
                    m_mapIDToTemplateItem.get(id).ToProtobuf(res.add_templateinfos());
                else
                    return CResult::Make_Error(format("The template id [{}] is no exist.", id));
            }
        }
        else if (req->templatetype_size() > 0)
        {
            auto types = req->templatetype();
            for (auto t : types)
            {
                for (auto &item : m_mapIDToTemplateItem)
                {
                    if (item.second.ConfigurationObj->moduletype() == t)
                        item.second.ToProtobuf(res.add_templateinfos());
                }
            }
        }
    }

    auto str_res = ConvertObjToStr(&res);
    m_pLog->debug(format("Query Template Succeed: Count[{}];[{}]", res.templateinfos_size(), CBase64::Encode(str_res)));
    return CResult::Make_OK(str_res);
}

CResult Sloong::CServerManage::QueryNodeHandler(const string &req_str, Package *pack)
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
            for (auto servID : m_mapIDToTemplateItem.get(id).Created)
            {
                m_mapUUIDToNodeItem[servID].ToProtobuf(res.add_nodeinfos());
            }
        }
    }

    return CResult::Make_OK(ConvertObjToStr(&res));
}

CResult Sloong::CServerManage::StopNodeHandler(const string &req_str, Package *pack)
{
    auto req = ConvertStrToObj<StopNodeRequest>(req_str);
    if (!req)
        return CResult::Make_Error("Parser message object fialed.");

    auto id = req->nodeid();
    if (!m_mapUUIDToNodeItem.exist(id))
        return CResult::Make_Error("NodeID error, the node no exit.");

    list<uint64_t> l;
    l.push_back(id);
    SendControlEvent(l, Core::ControlEvent::Stop, nullptr);

    return CResult::Succeed;
}

CResult Sloong::CServerManage::RestartNodeHandler(const string &req_str, Package *pack)
{
    auto req = ConvertStrToObj<RestartNodeRequest>(req_str);
    if (!req)
        return CResult::Make_Error("Parser message object fialed.");

    auto id = req->nodeid();
    if (!m_mapUUIDToNodeItem.exist(id))
        return CResult::Make_Error("NodeID error, the node no exit.");

    list<uint64_t> l;
    l.push_back(id);
    SendControlEvent(l, Core::ControlEvent::Restart, nullptr);

    return CResult::Succeed;
}

CResult Sloong::CServerManage::QueryReferenceInfoHandler(const string &req_str, Package *pack)
{
    m_pLog->debug("QueryReferenceInfoHandler <<< ");
    auto uuid = pack->sender();
    if (!m_mapUUIDToNodeItem.exist(uuid))
        return CResult::Make_Error(format("The node is no registed. [{}]", uuid));

    auto id = m_mapUUIDToNodeItem[uuid].TemplateID;
    auto item = m_mapIDToTemplateItem.try_get(id);
    if (item == nullptr)
        return CResult::Make_Error(format("The template id error. UUID[{}];ID[{}]", uuid, id));

    QueryReferenceInfoResponse res;

    auto references = Helper::split(item->ConfigurationObj->modulereference(), ',');
    for (auto ref : references)
    {
        auto ref_id = 0;
        if (!ConvertStrToInt(ref, &ref_id))
            continue;
        auto item = res.add_templateinfos();
        auto tpl = m_mapIDToTemplateItem.try_get(ref_id);
        if (tpl == nullptr)
        {
            m_pLog->warn(format("Reference template item [id:{}] no exist. please check. ", ref_id));
            continue;
        }
        item->set_templateid(tpl->ID);
        item->set_type(tpl->ConfigurationObj->moduletype());
        item->set_providefunctions(tpl->ConfigurationObj->modulefunctoins());
        for (auto node : tpl->Created)
        {
            m_mapUUIDToNodeItem[node].ToProtobuf(item->add_nodeinfos());
        }
    }
    m_pLog->debug("QueryReferenceInfoHandler response >>> " + res.ShortDebugString());
    return CResult::Make_OK(ConvertObjToStr(&res));
}

CResult Sloong::CServerManage::ReportLoadStatusHandler(const string &req_str, Package *pack)
{
    auto req = ConvertStrToObj<ReportLoadStatusRequest>(req_str);
    if (!req)
        return CResult::Make_Error("Parser message object fialed.");

    m_pLog->info(format("Node[{}] load status :CPU[{}]Mem[{}]", pack->sender(), req->cpuload(), req->memroyused()));

    return CResult(ResultType::Ignore);
}

CResult Sloong::CServerManage::ReconnectRegisterHandler(const string &req_str, Package *pack)
{
    auto req = ConvertStrToObj<ReconnectRegisterRequest>(req_str);
    if (!req)
        return CResult::Make_Error("Parser message object fialed.");

    if (m_mapUUIDToNodeItem.exist(req->nodeuuid()))
    {
        return CResult::Make_Error("Node is regitstered");
    }

    auto tpl = m_mapIDToTemplateItem.try_get(req->templateid());
    if (tpl == nullptr)
    {
        return CResult::Make_Error("Template not exist.");
    }

    if (tpl->Created.size() >= tpl->Replicas)
    {
        return CResult::Make_Error("Target template worker node is full.");
    }

    NodeItem item;
    item.UUID = req->nodeuuid();
    m_mapUUIDToNodeItem[item.UUID] = item;
    auto event = make_shared<GetConnectionInfoEvent>(pack->sessionid());
    event->SetCallbackFunc(
        [item = &m_mapUUIDToNodeItem[item.UUID]](IEvent *e, ConnectionInfo info) { item->Address = info.Address; });
    m_iC->CallMessage(event);

    item.TemplateName = tpl->Name;
    item.TemplateID = tpl->ID;
    item.Port = tpl->ConfigurationObj->listenport();
    item.ConnectionHashCode = pack->sessionid();
    tpl->Created.unique_insert(item.UUID);
    m_mapConnectionToUUID[pack->sessionid()] = item.UUID;

    // Find reference node and notify them
    list<uint64_t> notifyList;
    for (auto &item : m_mapIDToTemplateItem)
    {
        if (item.second.Reference.exist(req->templateid()))
        {
            for (auto i : item.second.Created)
                notifyList.push_back(i);
        }
    }

    if (notifyList.size() > 0)
    {
        EventReferenceModuleOnline online_event;
        m_mapUUIDToNodeItem[item.UUID].ToProtobuf(online_event.mutable_item());
        SendManagerEvent(notifyList, Manager::Events::ReferenceModuleOnline, &online_event);
    }

    return CResult::Succeed;
}

CResult Sloong::CServerManage::SetNodeLogLevelHandler(const string &req_str, Package *pack)
{
    auto req = ConvertStrToObj<SetNodeLogLevelRequest>(req_str);
    if (!req)
        return CResult::Make_Error("Parser message object fialed.");

    auto new_level = Helper::ntos(req->loglevel());
    list<uint64_t> notifyList;
    for (auto &id : req->nodes())
    {
        auto node = m_mapUUIDToNodeItem.try_get(id);
        if (node == nullptr)
        {
            m_pLog->warn(format("SetNodeLogLevelHandler: no found node with id [{}]. ignore.", id));
            continue;
        }
        notifyList.push_back(id);
        m_pLog->info(format("Send UpdateLogLevel event with new level [{}] for node [{}]", new_level, id));
    }

    if (notifyList.size() > 0)
    {
        SendControlEvent(notifyList, Core::ControlEvent::SetLogLevel, new_level);
    }

    return CResult::Succeed;
}