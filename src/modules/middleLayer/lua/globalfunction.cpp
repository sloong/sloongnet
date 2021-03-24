/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-12-11 15:05:40
 * @LastEditTime: 2020-08-12 20:23:53
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/middleLayer/lua/globalfunction.cpp
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

#include "globalfunction.h"
#include "utility.h"
#include "version.h"
#include "EpollEx.h"
#include "IData.h"
#include "snowflake.h"
#include "luaMiddleLayer.h"

#include "protocol/datacenter.pb.h"
#include "protocol/filecenter.pb.h"
#include "protocol/manager.pb.h"

#include "events/SendPackageToManager.hpp"
#include "events/RegisteConnection.hpp"
#include "events/SendPackage.hpp"
#include "events/ModuleOnOff.hpp"
#include "events/LuaEvent.hpp"
using namespace Sloong::Events;

unique_ptr<CGlobalFunction> Sloong::CGlobalFunction::Instance = make_unique<CGlobalFunction>();

LuaFunctionRegistr g_LuaFunc[] =
    {
        {"ShowLog", CGlobalFunction::Lua_ShowLog},
        {"GetEngineVer", CGlobalFunction::Lua_GetEngineVer},
        {"Base64Encode", CGlobalFunction::Lua_Base64_Encode},
        {"Base64Decode", CGlobalFunction::Lua_Base64_Decode},
        {"HashEncode", CGlobalFunction::Lua_Hash_Encode},
        {"ReloadScript", CGlobalFunction::Lua_ReloadScript},
        {"GetConfig", CGlobalFunction::Lua_GetConfig},
        {"GenUUID", CGlobalFunction::Lua_GenUUID},
        {"SetCommData", CGlobalFunction::Lua_SetCommData},
        {"GetCommData", CGlobalFunction::Lua_GetCommData},
        {"SetExtendData", CGlobalFunction::Lua_SetExtendData},
        {"SetExtendDataByFile", CGlobalFunction::Lua_SetExtendDataByFile},
        {"ConnectToDBCenter", CGlobalFunction::Lua_ConnectToDBCenter},
        {"SQLQueryToDBCenter", CGlobalFunction::Lua_SQLQueryToDBCenter},
        {"SQLInsertToDBCenter", CGlobalFunction::Lua_SQLInsertToDBCenter},
        {"SQLDeleteToDBCenter", CGlobalFunction::Lua_SQLDeleteToDBCenter},
        {"SQLUpdateToDBCenter", CGlobalFunction::Lua_SQLUpdateToDBCenter},
        {"PrepareUpload", CGlobalFunction::Lua_PrepareUpload},
        {"UploadEnd", CGlobalFunction::Lua_UploadEnd},
        {"GetThumbnail", CGlobalFunction::Lua_GetThumbnail},
        {"ConvertImageFormat", CGlobalFunction::Lua_ConvertImageFormat},
        {"SetTimeout", CGlobalFunction::Lua_SetTimeout},
        // 这个功能的实现仅是为了达到可用的程度，其实现存在一些性能问题。
        {"PushEvent", CGlobalFunction::Lua_PushEvent},
};

CResult Sloong::CGlobalFunction::Initialize(IControl *ic)
{
    IObject::Initialize(ic);
    IData::Initialize(ic);
    m_pModuleConfig = IData::GetModuleConfig();
    m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStart, std::bind(&CGlobalFunction::OnStart, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(LUA_EVENT_TYPE::OnReferenceModuleOnline, std::bind(&CGlobalFunction::OnReferenceModuleOnline, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(LUA_EVENT_TYPE::OnReferenceModuleOffline, std::bind(&CGlobalFunction::OnReferenceModuleOffline, this, std::placeholders::_1));
    return CResult::Succeed;
}

void Sloong::CGlobalFunction::OnStart(SharedEvent e)
{
    auto event = make_shared<SendPackageToManagerEvent>(Functions::QueryReferenceInfo, "");
    event->SetCallbackFunc(std::bind(&CGlobalFunction::QueryReferenceInfoResponseHandler, this, std::placeholders::_1, std::placeholders::_2));
    m_iC->SendMessage(event);
}

void Sloong::CGlobalFunction::QueryReferenceInfoResponseHandler(IEvent *send_pack, Package *res_pack)
{
    auto str_res = res_pack->content();
    auto res = ConvertStrToObj<QueryReferenceInfoResponse>(str_res);
    if (res == nullptr || res->templateinfos_size() == 0)
        return;

    auto templateInfos = res->templateinfos();
    for (auto info : templateInfos)
    {
        if (info.type() == MODULE_TYPE::DataCenter)
        {
            m_DataCenterTemplateID.store(info.templateid());
        }
        else if (info.type() == MODULE_TYPE::FileCenter)
        {
            m_FileCenterTemplateID.store(info.templateid());
        }
        else
        {
            continue;
        }
        for (auto item : info.nodeinfos())
        {
            m_mapUUIDToNode[item.uuid()] = item;
            m_mapTemplateIDToUUIDs[info.templateid()].push_back(item.uuid());

            AddConnection(item.uuid(), item.address(), item.port());
        }
    }
}

void Sloong::CGlobalFunction::AddConnection(uint64_t uuid, const string &addr, int port)
{
    auto event = make_shared<RegisteConnectionEvent>(addr, port);
    event->SetCallbackFunc([this, uuid](IEvent *e, uint64_t hashcode) {
        m_mapUUIDToConnectionID[uuid] = hashcode;
    });
    m_iC->SendMessage(event);
}

void Sloong::CGlobalFunction::OnReferenceModuleOnline(SharedEvent e)
{
    auto event = EVENT_TRANS<ModuleOnlineEvent>(e);
    auto info = event->GetInfos();
    auto item = info->item();
    m_mapUUIDToNode[item.uuid()] = item;
    m_mapTemplateIDToUUIDs[item.templateid()].push_back(item.uuid());
    m_pLog->Info(Helper::Format("New node[%lld][%s:%d] is online:templateid[%d],list size[%d]", item.uuid(), item.address().c_str(), item.port(), item.templateid(), m_mapTemplateIDToUUIDs[item.templateid()].size()));

    AddConnection(item.uuid(), item.address(), item.port());
}

void Sloong::CGlobalFunction::OnReferenceModuleOffline(SharedEvent e)
{
    auto event = EVENT_TRANS<ModuleOfflineEvent>(e);
    auto info = event->GetInfos();
    auto uuid = info->uuid();
    auto item = m_mapUUIDToNode[uuid];

    m_mapTemplateIDToUUIDs[item.templateid()].erase(item.uuid());
    m_mapUUIDToConnectionID.erase(uuid);
    m_mapUUIDToNode.erase(uuid);
    m_pLog->Info(Helper::Format("Node is offline [%lld], template id[%d],list size[%d]", item.uuid(), item.templateid(), m_mapTemplateIDToUUIDs[item.templateid()].size()));
}

void Sloong::CGlobalFunction::RegistFuncToLua(CLua *pLua)
{
    vector<LuaFunctionRegistr> funcList(g_LuaFunc, g_LuaFunc + ARRAYSIZE(g_LuaFunc));
    pLua->AddFunctions(&funcList);
}

int Sloong::CGlobalFunction::Lua_GetEngineVer(lua_State *l)
{
    CLua::PushString(l, VERSION_TEXT);
    return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Encode(lua_State *l)
{
    string req = CLua::GetString(l, 1, "");
    string res = CBase64::Encode(req);
    CLua::PushString(l, res);
    return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Decode(lua_State *l)
{
    string req = CLua::GetString(l, 1, "");
    string res = CBase64::Decode(req);
    CLua::PushString(l, res);
    return 1;
}

int Sloong::CGlobalFunction::Lua_SetTimeout(lua_State *l)
{
    auto time = CLua::GetInteger(l, 1, -1);
    if (time != -1)
    {
        CGlobalFunction::Instance->SetTimeout(time);
    }
    return 0;
}

int CGlobalFunction::Lua_PushEvent(lua_State *l)
{
    auto e = CLua::GetInteger(l, 1, -1);
    auto p = CLua::GetString(l, 2);

    auto event = make_shared<LuaEvent>();
    event->SetLuaEvent(e);
    event->SetLuaEventParam(move(p));

    CGlobalFunction::Instance->m_iC->SendMessage(event);
    return 0;
}

/**
 * @Description:
 * @Params: 1 string > string data. if file mode, is the file path
 * 			2 string > hash type. support : 0(MD5), 1(SHA-1),2(SHA-256), 3(SHA-512). default by SHA-1
 * 			3 boolen > file mode. default by false
 * @Return: Hash result
 */
int Sloong::CGlobalFunction::Lua_Hash_Encode(lua_State *l)
{
    string data = CLua::GetString(l, 1, "");
    int hash_type = CLua::GetInteger(l, 2, 1);
    bool file_mode = CLua::GetBoolen(l, 3);
    string result("");

    switch (hash_type)
    {
    case HashType::MD5:
        result = CMD5::Encode(data, file_mode);
        break;
    case HashType::SHA_1:
        result = CSHA1::Encode(data, file_mode);
        break;
    case HashType::SHA_256:
        result = CSHA256::Encode(data, file_mode);
        break;
    case HashType::SHA_512:
        result = CSHA512::Encode(data, file_mode);
        break;
    default:
        result = "Hash type error. support : 0(MD5), 1(SHA-1),2(SHA-256), 3(SHA-512).";
        break;
    }
    CLua::PushString(l, result);
    return 1;
}

int Sloong::CGlobalFunction::Lua_ReloadScript(lua_State *l)
{
    LuaMiddleLayer::Instance->SetReloadScriptFlag();
    return 0;
}

int Sloong::CGlobalFunction::Lua_GetConfig(lua_State *l)
{
    string section = CLua::GetString(l, 1);
    string key = CLua::GetString(l, 2);
    string def = CLua::GetString(l, 3);
    auto config = CGlobalFunction::Instance->m_pModuleConfig;
    if (config->isMember(section) && (*config)[section].isMember(key))
    {
        CLua::PushString(l, (*config)[section][key].asString());
    }
    else
    {
        CLua::PushString(l, def);
    }
    return 1;
}

int CGlobalFunction::Lua_GenUUID(lua_State *l)
{
    CLua::PushString(l, CUtility::GenUUID());
    return 1;
}

int CGlobalFunction::Lua_ShowLog(lua_State *l)
{
    string luaTitle = CLua::GetString(l, 2, "Info");
    string msg = CLua::GetString(l, 1);
    if (msg == "")
        return 0;
    else
        msg = Helper::Format("[Script]:[%s]", msg.c_str());

    auto log = CGlobalFunction::Instance->m_pLog;
    if (luaTitle == "Info")
        log->Info(msg);
    else if (luaTitle == "Warn")
        log->Warn(msg);
    else if (luaTitle == "Debug")
        log->Debug(msg);
    else if (luaTitle == "Error")
        log->Error(msg);
    else if (luaTitle == "Verbos")
        log->Verbos(msg);
    else if (luaTitle == "Assert")
        log->Assert(msg);
    else if (luaTitle == "Fatal")
        log->Fatal(msg);
    else
        log->Log(msg, luaTitle);
    return 0;
}

int CGlobalFunction::Lua_SetCommData(lua_State *l)
{
    auto key = CLua::GetString(l, 1, "");
    auto value = CLua::GetString(l, 2, "");
    if (!key.empty())
    {
        if (value.empty())
            Instance->m_mapCommData.erase(key);
        else
            Instance->m_mapCommData[key] = value;
    }
    return 0;
}

int CGlobalFunction::Lua_GetCommData(lua_State *l)
{
    auto key = CLua::GetString(l, 1, "");
    if (key.length() > 0 && Instance->m_mapCommData.exist(key))
        CLua::PushString(l, Instance->m_mapCommData[key]);
    else
        CLua::PushString(l, "");
    return 1;
}

int CGlobalFunction::Lua_GetLogObject(lua_State *l)
{
    CLua::PushPointer(l, CGlobalFunction::Instance->m_pLog);
    return 1;
}

int CGlobalFunction::Lua_SetExtendData(lua_State *l)
{
    auto data = CLua::GetString(l, 1, "");
    auto uuid = "";
    if (data.length() > 0)
    {
        auto uuid = CUtility::GenUUID();
        Instance->m_iC->AddTempString(uuid, data);
    }

    CLua::PushString(l, uuid);
    return 1;
}

int CGlobalFunction::Lua_SetExtendDataByFile(lua_State *l)
{
    auto file = CLua::GetString(l, 1, "");
    auto uuid = "";
    int size = 0;
    auto pBuf = CUtility::ReadFile(file, &size);
    if (size > 0)
    {
        auto uuid = CUtility::GenUUID();
        Instance->m_iC->AddTempBytes(uuid, pBuf, size);
    }
    CLua::PushInteger(l, size);
    CLua::PushString(l, uuid);
    return 2;
}

U64Result CGlobalFunction::GetConnectionID(int templateid)
{
    if (CGlobalFunction::Instance->m_mapTemplateIDToUUIDs[templateid].size() == 0)
    {
        return U64Result::Make_Error(Helper::Format("Template[%d] no node online.", templateid));
    }

    auto uuid = CGlobalFunction::Instance->m_mapTemplateIDToUUIDs[templateid].front();
    if (!CGlobalFunction::Instance->m_mapUUIDToConnectionID.exist(uuid))
    {
        auto item = CGlobalFunction::Instance->m_mapUUIDToNode[uuid];
        CGlobalFunction::Instance->AddConnection(uuid, item.address(), item.port());

        return U64Result::Make_Error(Helper::Format("Try connect to [%d][%lld][%s:%d], please wait and retry.", templateid, uuid, item.address().c_str(), item.port()));
    }

    return U64Result::Make_OKResult(CGlobalFunction::Instance->m_mapUUIDToConnectionID[uuid]);
}

int CGlobalFunction::Lua_ConnectToDBCenter(lua_State *l)
{
    auto DBName = CLua::GetString(l, 1, "");
    if (DBName.empty())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "Database is empty");
        return 2;
    }

    if (CGlobalFunction::Instance->m_mapDBNameToSessionID.exist(DBName))
    {
        CLua::PushInteger(l, Base::ResultType::Succeed);
        CLua::PushInteger(l, CGlobalFunction::Instance->m_mapDBNameToSessionID[DBName]);
        return 2;
    }

    auto templateid = CGlobalFunction::Instance->m_DataCenterTemplateID.load();
    if (templateid == 0)
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "No enable datacenter, please check the configuation.");
        return 2;
    }

    auto conn = CGlobalFunction::Instance->GetConnectionID(templateid);
    if (conn.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, conn.GetMessage());
        return 2;
    }
    auto session = conn.GetResultObject();

    DataCenter::ConnectDatabaseRequest request;
    request.set_database(DBName);

    auto req = make_shared<SendPackageEvent>(session);
    req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, DataCenter::Functions::ConnectDatabase, ConvertObjToStr(&request));
    auto res = req->SyncCall(CGlobalFunction::Instance->m_iC, CGlobalFunction::Instance->m_nTimeout);
    if (res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, res.GetMessage());
    }
    auto response = ConvertStrToObj<DataCenter::ConnectDatabaseResponse>(res.GetMessage());
    if (response == nullptr)
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "Parse result error.");
        return 2;
    }
    CGlobalFunction::Instance->m_mapDBNameToSessionID[DBName] = response->session();

    CLua::PushInteger(l, Base::ResultType::Succeed);
    CLua::PushInteger(l, response->session());

    return 2;
}

U64Result CGlobalFunction::SQLFunctionPrepareCheck(lua_State *l, int sessionid, const string &sql)
{
    if (sessionid == -1)
    {
        return U64Result::Make_Error("Database session id is invalid, call ConnectDBCenter first.");
    }

    if (sql.empty())
    {
        return U64Result::Make_Error(l, "request data is empty");
    }

    auto res = CGlobalFunction::Instance->GetConnectionID(CGlobalFunction::Instance->m_DataCenterTemplateID.load());
    if (res.IsFialed())
    {
        return res;
    }

    return res.GetResultObject();
}

CResult CGlobalFunction::RunSQLFunction(uint64_t session, const string &request_str, int func)
{
    auto req = make_shared<SendPackageEvent>(session);
    req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, func, request_str);
    return req->SyncCall(CGlobalFunction::Instance->m_iC, CGlobalFunction::Instance->m_nTimeout);
}

// Request sql query cmd to dbcenter.
// Params:
//     1> SessionID
//     2> Cmd
int CGlobalFunction::Lua_SQLQueryToDBCenter(lua_State *l)
{
    auto SessionID = CLua::GetInteger(l, 1, -1);
    auto query_cmd = CLua::GetString(l, 2, "");
    auto session_res = SQLFunctionPrepareCheck(l, SessionID, query_cmd);
    if (session_res.IsFialed())
    {
        CLua::PushInteger(l,Base::ResultType::Error );
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
    auto session = session_res.GetResultObject();

    DataCenter::QuerySQLCmdRequest request;
    request.set_session(SessionID);
    request.set_sqlcmd(query_cmd);

    auto res = RunSQLFunction(session, ConvertObjToStr(&request), (int)DataCenter::Functions::QuerySQLCmd);
    if (res.IsSucceed())
    {
        auto response = ConvertStrToObj<DataCenter::QuerySQLCmdResponse>(res.GetMessage());
        if (response->lines_size() == 0)
        {
            CLua::PushInteger(l, Base::ResultType::Succeed );
            CLua::PushInteger(l, 0);
            CLua::PushNil(l);
            return 3;
        }
        else
        {
            CLua::PushInteger(l, Base::ResultType::Succeed );
            CLua::PushInteger(l, response->lines_size());
            list<list<string>> res;
            for (auto &item : response->lines())
            {
                list<string> row;
                for (auto &j : item.rawdataitem())
                    row.push_back(j);
                res.push_back(row);
            }
            CLua::Push2DTable(l, res);
            return 3;
        }
    }
    else
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
}

// Request sql query cmd to dbcenter.
// Params:
//     1> SessionID
//     2> Cmd
int CGlobalFunction::Lua_SQLInsertToDBCenter(lua_State *l)
{
    auto SessionID = CLua::GetInteger(l, 1, -1);
    auto sql_cmd = CLua::GetString(l, 2, "");
    auto session_res = SQLFunctionPrepareCheck(l, SessionID, query_cmd);
    if (session_res.IsFialed())
    {
        CLua::PushInteger(l,Base::ResultType::Error );
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
    auto session = session_res.GetResultObject();

    DataCenter::InsertSQLCmdRequest request;
    request.set_session(SessionID);
    request.set_sqlcmd(sql_cmd);
    request.set_getidentity(true);

    auto res = RunSQLFunction(session, ConvertObjToStr(&request), (int)DataCenter::Functions::InsertSQLCmd);
    if (res.IsSucceed())
    {
        auto response = ConvertStrToObj<DataCenter::InsertSQLCmdResponse>(res.GetMessage());
        if (response->affectedrows() > 0)
        {
            CLua::PushInteger(l, Base::ResultType::Succeed);
            CLua::PushInteger(l, response->identity());
        }
        else
        {
            CLua::PushInteger(l, Base::ResultType::Error);
            CLua::PushString(l, "SQL Run succeed, but affectedrows is 0");
        }
        return 2;
    }
    else
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
}

int CGlobalFunction::Lua_SQLDeleteToDBCenter(lua_State *l)
{
    auto SessionID = CLua::GetInteger(l, 1, -1);
    auto sql_cmd = CLua::GetString(l, 2, "");
    auto session_res = SQLFunctionPrepareCheck(l, SessionID, query_cmd);
    if (session_res.IsFialed())
    {
        CLua::PushInteger(l,Base::ResultType::Error );
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
    auto session = session_res.GetResultObject();

    DataCenter::DeleteSQLCmdRequest request;
    request.set_session(SessionID);
    request.set_sqlcmd(sql_cmd);

    auto res = RunSQLFunction(session, ConvertObjToStr(&request), (int)DataCenter::Functions::DeleteSQLCmd);
    if (res.IsSucceed())
    {
        auto response = ConvertStrToObj<DataCenter::DeleteSQLCmdResponse>(res.GetMessage());
        CLua::PushInteger(l,Base::ResultType::Succeed );
        CLua::PushInteger(l, response->affectedrows());
        return 2;
    }
    else
    {
        CLua::PushInteger(l, -1);
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
}

int CGlobalFunction::Lua_SQLUpdateToDBCenter(lua_State *l)
{
    auto SessionID = CLua::GetInteger(l, 1, -1);
    auto sql_cmd = CLua::GetString(l, 2, "");
    auto session_res = SQLFunctionPrepareCheck(l, SessionID, query_cmd);
    if (session_res.IsFialed())
    {
        CLua::PushInteger(l,Base::ResultType::Error );
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
    auto session = session_res.GetResultObject();


    DataCenter::UpdateSQLCmdRequest request;
    request.set_session(SessionID);
    request.set_sqlcmd(sql_cmd);

    auto res = RunSQLFunction(session, ConvertObjToStr(&request), (int)DataCenter::Functions::UpdateSQLCmd);
    if (res.IsSucceed())
    {
        auto response = ConvertStrToObj<DataCenter::UpdateSQLCmdResponse>(res.GetMessage());
        CLua::PushInteger(l,Base::ResultType::Succeed );
        CLua::PushInteger(l, response->affectedrows());
        return 2;
    }
    else
    {
        CLua::PushInteger(l, Base::ResultType::Error );
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
}

int CGlobalFunction::Lua_PrepareUpload(lua_State *l)
{
    auto file_sha256 = CLua::GetString(l, 1, "");
    auto file_size = CLua::GetInteger(l, 2, 0);
    if (file_sha256.empty() || file_size == 0)
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "request data is empty");
        return 2;
    }

    auto conn = CGlobalFunction::Instance->GetConnectionID(CGlobalFunction::Instance->m_FileCenterTemplateID.load());
    if (conn.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, conn.GetMessage());
        return 2;
    }
    auto session = conn.GetResultObject();

    FileCenter::PrepareUploadRequest request;
    request.set_sha256(file_sha256);
    request.set_filesize(file_size);

    auto req = make_shared<SendPackageEvent>(session);
    req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, FileCenter::Functions::PrepareUpload, ConvertObjToStr(&request));
    auto res = req->SyncCall(CGlobalFunction::Instance->m_iC, CGlobalFunction::Instance->m_nTimeout);
    if (res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, res.GetMessage());
        return 2;
    }

    auto response = ConvertStrToObj<FileCenter::PrepareUploadResponse>(res.GetMessage());
    CLua::PushInteger(l, Base::ResultType::Succeed);
    CLua::PushString(l, response->token());
    return 2;
}

int CGlobalFunction::Lua_UploadEnd(lua_State *l)
{
    auto token = CLua::GetString(l, 1, "");
    if (token.empty())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "request data is empty");
        return 2;
    }

    auto conn = CGlobalFunction::Instance->GetConnectionID(CGlobalFunction::Instance->m_FileCenterTemplateID.load());
    if (conn.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, conn.GetMessage());
        return 2;
    }
    auto session = conn.GetResultObject();

    FileCenter::UploadedRequest request;
    request.set_token(token);

    auto req = make_shared<SendPackageEvent>(session);
    req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, FileCenter::Functions::Uploaded, ConvertObjToStr(&request));
    auto res = req->SyncCall(CGlobalFunction::Instance->m_iC, CGlobalFunction::Instance->m_nTimeout);
    if (res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, res.GetMessage());
        return 2;
    }
    CLua::PushInteger(l, Base::ResultType::Succeed);
    CLua::PushString(l, "succeed");
    return 2;
}

int CGlobalFunction::Lua_GetThumbnail(lua_State *l)
{
    auto file_index = CLua::GetString(l, 1, "");
    auto height = CLua::GetInteger(l, 2, 0);
    auto width = CLua::GetInteger(l, 3, 0);
    auto quality = CLua::GetInteger(l, 4, 0);
    if (file_index.empty() || height == 0 || width == 0 || quality == 0)
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "Param error.");
        return 2;
    }

    auto conn = CGlobalFunction::Instance->GetConnectionID(CGlobalFunction::Instance->m_FileCenterTemplateID.load());
    if (conn.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, conn.GetMessage());
        return 2;
    }

    auto session = conn.GetResultObject();

    FileCenter::GetThumbnailRequest request;
    request.set_index(file_index);
    request.set_height(height);
    request.set_width(width);
    request.set_quality(quality);

    auto req = make_shared<SendPackageEvent>(session);
    req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, FileCenter::Functions::GetThumbnail, ConvertObjToStr(&request));
    auto res = req->SyncCall(CGlobalFunction::Instance->m_iC, CGlobalFunction::Instance->m_nTimeout);
    if (res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, res.GetMessage());
        return 2;
    }

    auto response = ConvertStrToObj<FileCenter::GetThumbnailResponse>(res.GetMessage());

    auto uuid = CUtility::GenUUID();
    CGlobalFunction::Instance->m_iC->AddTempString(uuid, response->data());
    CLua::PushInteger(l, Base::ResultType::Succeed);
    CLua::PushString(l, uuid);
    return 2;
}

/// Send the convert image format request to FileCenter.
/// Request params:
///  1(str) > the file index.
///  2(int) > the target image format (FileCenter.SupportFormat) in filecenter proto file.
///  3(int) > convert quality.
///  4(bool) > retain old file.
/// Return:
///  1(Base::ResultType) > the resule.
/// (Succeed):
///  2(str) > new file index.
///  3(str) > new file sha256.
///  4(str) > new file md5.
///  5(int) > new file size.
/// (Error):
///  2(str) > the error message.
int CGlobalFunction::Lua_ConvertImageFormat(lua_State *l)
{
    auto file_index = CLua::GetString(l, 1, "");
    auto target = CLua::GetInteger(l, 2, 0);
    auto quality = CLua::GetInteger(l, 3, 0);
    auto retain = CLua::GetBoolen(l, 4);
    if (file_index.empty() || quality <= 0 )
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "Param error.");
        return 2;
    }

    if (!FileCenter::SupportFormat_IsValid(target))
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, Helper::Format("Convert [%d] to FileCenter::SupportFormat enum fialed.", target));
        return 2;
    }

    auto conn = CGlobalFunction::Instance->GetConnectionID(CGlobalFunction::Instance->m_FileCenterTemplateID.load());
    if (conn.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, conn.GetMessage());
        return 2;
    }

    auto session = conn.GetResultObject();

    FileCenter::ConvertImageFileRequest request;
    request.set_index(file_index);
    request.set_targetformat((FileCenter::SupportFormat)target);
    request.set_quality(quality);
    request.set_retainsourcefile(retain);

    auto req = make_shared<SendPackageEvent>(session);
    req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, FileCenter::Functions::ConvertImageFile, ConvertObjToStr(&request));
    auto res = req->SyncCall(CGlobalFunction::Instance->m_iC, CGlobalFunction::Instance->m_nTimeout);
    if (res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, res.GetMessage());
        return 2;
    }

    auto response = ConvertStrToObj<FileCenter::ConvertImageFileResponse>(res.GetMessage());
    CLua::PushInteger(l, Base::ResultType::Succeed);
    CLua::PushString(l, response->newfileindexcode());
    CLua::PushString(l, response->newfilesha256());
    CLua::PushString(l, response->newfilemd5());
    CLua::PushInteger(l, response->newfilesize());
    return 5;
}