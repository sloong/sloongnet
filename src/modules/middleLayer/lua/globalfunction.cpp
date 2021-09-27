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
#include "luaMiddleLayer.h"

#include "protocol/datacenter.pb.h"
#include "protocol/filecenter.pb.h"
#include "protocol/manager.pb.h"

#include "events/SendPackageToManager.hpp"
#include "events/RegisterConnection.hpp"
#include "events/SendPackage.hpp"
#include "events/ModuleOnOff.hpp"
#include "events/LuaEvent.hpp"
using namespace Sloong::Events;

unique_ptr<CGlobalFunction> Sloong::CGlobalFunction::Instance = make_unique<CGlobalFunction>();
const string g_temp_file_path = "/tmp/sloongnet/receivefile/temp.tmp";

LuaFunctionRegisterr g_LuaFunc[] =
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
        {"MoveFile", CGlobalFunction::Lua_MoveFile},
        {"SendFile", CGlobalFunction::Lua_SendFile},
        {"ReceiveFile", CGlobalFunction::Lua_ReceiveFile},
        {"CheckRecvStatus", CGlobalFunction::Lua_CheckRecvStatus},
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
    m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStop, std::bind(&CGlobalFunction::OnStop, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(LUA_EVENT_TYPE::OnReferenceModuleOnline, std::bind(&CGlobalFunction::OnReferenceModuleOnline, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(LUA_EVENT_TYPE::OnReferenceModuleOffline, std::bind(&CGlobalFunction::OnReferenceModuleOffline, this, std::placeholders::_1));
    return CResult::Succeed;
}

CResult CGlobalFunction::EnableDataReceive(int port, int timtout)
{
    if (port < 0)
        return CResult::Make_Error("Listen port error.");

    m_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
    int sock_op = 1;
    // SOL_SOCKET:在socket层面设置
    // SO_REUSEADDR:允许套接字和一个已在使用中的地址捆绑
    setsockopt(m_ListenSock, SOL_SOCKET, SO_REUSEADDR, &sock_op, sizeof(sock_op));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons((uint16_t)port);

    errno = bind(m_ListenSock, (struct sockaddr *)&address, sizeof(address));

    if (errno == -1)
        return CResult::Make_Error(format("bind to {} field. errno = {}", port, errno));

    errno = listen(m_ListenSock, 1024);

    m_nRecvDataTimeoutTime = timtout;
    ThreadPool::AddWorkThread(std::bind(&CGlobalFunction::RecvDataConnFunc, this), 1);
    return CResult::Succeed;
}

void CGlobalFunction::ClearReceiveInfoByUUID(string uuid)
{
    m_pLog->debug(format("Clean receive info from g_RecvDataInfoList by :[{}]", uuid));
    auto p_item_list = g_RecvDataInfoList.find(uuid);
    if (p_item_list == g_RecvDataInfoList.end())
    {
        return;
    }
    auto item_list = p_item_list->second;
    for (auto item = item_list.begin(); item != item_list.end(); item++)
    {
        RecvDataPackage *pack = item->second;
        SAFE_DELETE(pack);
        item_list.erase(item);
    }
}

//
void Sloong::CGlobalFunction::RecvDataConnFunc()
{
    spdlog::logger *pLog = m_pLog;
    while (m_emStatus != RUN_STATUS::Exit)
    {
        int conn_sock = -1;
        if ((conn_sock = accept(m_ListenSock, NULL, NULL)) > 0)
        {
            pLog->debug(format("Accept data connect :[{}][{}]", CUtility::GetSocketIP(conn_sock), CUtility::GetSocketPort(conn_sock)));
            // When accept the connect , receive the uuid data. and
            char *pCheckBuf = new char[g_uuid_len + 1];
            memset(pCheckBuf, 0, g_uuid_len + 1);
            // In Check function, client need send the check key in 3 second.
            int nLen = Helper::RecvEx(conn_sock, pCheckBuf, CGlobalFunction::g_uuid_len, m_nRecvDataTimeoutTime);
            // Check uuid length
            if (nLen != CGlobalFunction::g_uuid_len)
            {
                pLog->warn(format("The uuid length error:[{}]. Close connect.", nLen));
                close(conn_sock);
                continue;
            }
            // Check uuid validity
            if (g_RecvDataInfoList.find(pCheckBuf) == g_RecvDataInfoList.end())
            {
                pLog->warn(format("The uuid is not find in list:[{}]. Close connect.", pCheckBuf));
                close(conn_sock);
                continue;
            }
            // Add to connect list
            g_RecvDataConnList[conn_sock] = pCheckBuf;
            // Start new thread to recv data for this connect.
            auto f = std::bind(&CGlobalFunction::RecvFileFunc, this, conn_sock);
            // TODO: modify the libarary function.
            TaskPool::Run([f]() {
                f();
                return nullptr;
            });
        }
    }
}

void Sloong::CGlobalFunction::RecvFileFunc(int conn_sock)
{
    spdlog::logger *pLog = m_pLog;
    // Find the recv uuid.
    auto conn_item = g_RecvDataConnList.find(conn_sock);
    if (conn_item == g_RecvDataConnList.end())
    {
        pLog->error("The socket id is not find in conn list.");
        return;
    }
    string uuid = conn_item->second;
    pLog->info(format("Start thread to receive file data for :[{}]", uuid));
    // Find the recv info list.
    auto info_item = g_RecvDataInfoList.find(uuid);
    if (info_item == g_RecvDataInfoList.end())
    {
        pLog->error("The uuid is not find in info list.");
        return;
    }
    try
    {
        map<string, RecvDataPackage *> recv_file_list = info_item->second;
        bool bLoop = false;
        do
        {
            char *pLongBuffer = new char[g_data_pack_len + 1](); //dataLeng;
            memset(pLongBuffer, 0, g_data_pack_len + 1);
            int nRecvSize = Helper::RecvTimeout(conn_sock, pLongBuffer, g_data_pack_len, m_nRecvDataTimeoutTime);
            if (nRecvSize <= 0)
            {
                SAFE_DELETE_ARR(pLongBuffer);
                pLog->warn("Recv data package length error.");
                return;
            }
            else
            {

                auto dlen = Helper::BytesToInt64(pLongBuffer);
                SAFE_DELETE_ARR(pLongBuffer);
                // package length cannot big than 2147483648. this is max value for int.
                if (dlen <= 0 || dlen > FILE_TRANS_MAX_SIZE || nRecvSize != g_data_pack_len)
                {
                    pLog->error("Receive data length error.");
                    return;
                }
                int dtlen = (int)dlen;

                char *szMD5 = new char[g_md5_len + 1];
                memset(szMD5, 0, g_md5_len + 1);
                nRecvSize = Helper::RecvTimeout(conn_sock, szMD5, g_md5_len, m_nRecvDataTimeoutTime, true);
                if (nRecvSize <= 0)
                {
                    pLog->error("Receive data package md5 error.");
                    SAFE_DELETE_ARR(szMD5);
                    return;
                }
                string trans_md5 = string(szMD5);
                Helper::tolower(trans_md5);

                auto recv_file_item = recv_file_list.find(trans_md5);
                if (recv_file_item == recv_file_list.end())
                {
                    pLog->error("the file md5 is not find in recv list.");
                    return;
                }
                RecvDataPackage *pack = recv_file_item->second;
                pack->emStatus = RecvStatus::Receiving;

                char *data = new char[dtlen];
                memset(data, 0, dtlen);

                // In here receive 10240 length data in one time.
                // because the file length is different, if the file is too big, and user network speed not to fast,
                // it will be fialed.
                char *pData = data;
                int nRecvdLen = 0;
                while (nRecvdLen < dtlen)
                {
                    int nOnceRecvLen = 10240;
                    if (dtlen - nRecvdLen < 10240)
                        nOnceRecvLen = dtlen - nRecvdLen;
                    nRecvSize = Helper::RecvTimeout(conn_sock, pData, nOnceRecvLen, m_nRecvDataTimeoutTime, true);
                    if (nRecvSize < 0)
                    {
                        pLog->error("Receive data error.");
                        SAFE_DELETE_ARR(data);
                        return;
                    }
                    else if (nRecvSize == 0)
                    {
                        pLog->error("Receive data timeout.");
                        SAFE_DELETE_ARR(data);
                        return;
                    }
                    else
                    {
                        pData += nRecvSize;
                        nRecvdLen += nRecvSize;
                    }
                }

                pack->emStatus = RecvStatus::Saveing;

                // check target file path is not exist
                Helper::CheckFileDirectory(pack->strPath);

                // save to file
                ofstream of;
                of.open(pack->strPath + pack->strName, ios::out | ios::trunc | ios::binary);
                of.write(data, dtlen);
                of.close();
                SAFE_DELETE_ARR(data);

                string file_md5 = CMD5::Encode(pack->strPath + pack->strName, true);
                Helper::tolower(file_md5);

                // check md5
                if (trans_md5.compare(file_md5))
                {
                    pLog->error("the file data is different with md5 code.");
                    pack->emStatus = RecvStatus::VerificationError;
                }
                else
                {
                    pack->emStatus = RecvStatus::Done;
                }

                // check the receive file list status
                for (auto item = recv_file_list.begin(); item != recv_file_list.end(); item++)
                {
                    auto pack = item->second;
                    if (pack->emStatus == RecvStatus::Wait)
                    {
                        bLoop = true;
                        break;
                    }
                    else
                    {
                        bLoop = false;
                    }
                }
            }
        } while (bLoop);

        pLog->debug(format("Receive connect done. close:[{}:{}]", CUtility::GetSocketIP(conn_sock), CUtility::GetSocketPort(conn_sock)));
        close(conn_sock);
    }
    catch (const std::exception &)
    {
        close(conn_sock);
        ClearReceiveInfoByUUID(uuid);
    }
}

void Sloong::CGlobalFunction::OnStart(SharedEvent e)
{
    auto event = make_shared<SendPackageToManagerEvent>(Functions::QueryReferenceInfo, "");
    event->SetCallbackFunc(std::bind(&CGlobalFunction::QueryReferenceInfoResponseHandler, this, std::placeholders::_1, std::placeholders::_2));
    m_iC->SendMessage(event);
    m_emStatus = RUN_STATUS::Running;
}
void Sloong::CGlobalFunction::OnStop(SharedEvent e)
{
    m_emStatus = RUN_STATUS::Exit;
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
    auto event = make_shared<RegisterConnectionEvent>(addr, port);
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
    m_pLog->info(format("New node[{}][{}:{}] is online:templateid[{}],list size[{}]", item.uuid(), item.address(), item.port(), item.templateid(), m_mapTemplateIDToUUIDs[item.templateid()].size()));

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
    m_pLog->info(format("Node is offline [{}], template id[{}],list size[{}]", item.uuid(), item.templateid(), m_mapTemplateIDToUUIDs[item.templateid()].size()));
}

void Sloong::CGlobalFunction::RegisterFuncToLua(CLua *pLua)
{
    vector<LuaFunctionRegisterr> funcList(g_LuaFunc, g_LuaFunc + ARRAYSIZE(g_LuaFunc));
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
        msg = format("[Script]:[{}]", msg);

    auto log = CGlobalFunction::Instance->m_pLog;
    if (luaTitle == "Info")
        log->info(msg);
    else if (luaTitle == "Warn")
        log->warn(msg);
    else if (luaTitle == "Debug")
        log->debug(msg);
    else if (luaTitle == "Error")
        log->error(msg);
    else if (luaTitle == "Verbos")
        log->trace(msg);
    else if (luaTitle == "Fatal")
        log->critical(msg);
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

int CGlobalFunction::Lua_MoveFile(lua_State *l)
{
    string orgName = CLua::GetString(l, 1, "");
    string newName = CLua::GetString(l, 2, "");
    int nRes(0);
    try
    {
        if (orgName == "" || newName == "")
        {
            nRes = -2;
            std::throw_with_nested(invalid_argument(format("Move File error. File name cannot empty. orgName:{};newName:{}", orgName, newName)));
        }

        if (access(orgName.c_str(), ACC_R) != 0)
        {
            nRes = -1;
            std::throw_with_nested(runtime_error(format("Move File error. Origin file not exist or can not read:[{}]", orgName)));
        }

        int res = Helper::CheckFileDirectory(newName);
        if (res < 0)
        {
            nRes = -1;
            std::throw_with_nested(runtime_error(format("Move File error.CheckFileDirectory error:[{}][{}]", newName, res)));
        }

        if (!Helper::MoveFile(orgName, newName))
        {
            // Move file need write access. so if move file error, try copy .
            if (!Helper::RunSystemCmd(format("cp \"{}\" \"{}\"", orgName, newName)))
            {
                nRes = -3;
                std::throw_with_nested(runtime_error("Move File and try copy file error."));
            }
            nRes = 1;
        }
    }
    catch (exception &e)
    {
        CGlobalFunction::Instance->m_pLog->error(e.what());
        CLua::PushInteger(l, nRes);
        CLua::PushString(l, e.what());
        return 2;
    }

    // if succeed return 0, else return nozero
    CLua::PushInteger(l, nRes);
    CLua::PushString(l, "");
    return 2;
}

int Sloong::CGlobalFunction::Lua_SendFile(lua_State *l)
{
    return Lua_SetExtendDataByFile(l);
}

// Receive File funcs
// Client requeset with file list info
// and here add the info to list and Build one uuid and return this uuid.
int CGlobalFunction::Lua_ReceiveFile(lua_State *l)
{
    string save_folder = CLua::GetString(l, 2);

    // The file list, key is md5 ,value is file name
    auto fileList = CLua::GetTableParam(l, 1);
    string uuid = CUtility::GenUUID();

    map<string, RecvDataPackage *> recv_list;
    for (auto item : *fileList)
    {
        RecvDataPackage *pack = new RecvDataPackage();
        string md5 = item.first;
        Helper::tolower(md5);
        pack->strName = item.second;
        pack->strPath = save_folder;
        pack->strMD5 = md5;
        pack->emStatus = RecvStatus::Wait;
        recv_list[md5] = pack;
    }

    CGlobalFunction::Instance->g_RecvDataInfoList[uuid] = recv_list;

    CLua::PushString(l, uuid);
    return 1;
}

int CGlobalFunction::Lua_CheckRecvStatus(lua_State *l)
{
    string uuid = CLua::GetString(l, 1);
    string md5 = CLua::GetString(l, 2);
    Helper::tolower(md5);
    auto recv_list = CGlobalFunction::Instance->g_RecvDataInfoList[uuid];
    auto recv_item = recv_list.find(md5);
    if (recv_item == recv_list.end())
    {
        CLua::PushInteger(l, RecvStatus::OtherError);
        CLua::PushString(l, "Cannot find the hash receive info.");
        return 2;
    }
    else
    {
        RecvDataPackage *pack = recv_item->second;
        if (pack->emStatus == RecvStatus::Done)
        {
            recv_list.erase(recv_item);
            CLua::PushInteger(l, RecvStatus::Done);
            CLua::PushString(l, pack->strPath + pack->strName);
            return 2;
        }
        else
        {
            CLua::PushInteger(l, pack->emStatus);
            CLua::PushString(l, "");
            return 2;
        }
    }
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
        return U64Result::Make_Error(format("Template[{}] no node online.", templateid));
    }

    auto uuid = CGlobalFunction::Instance->m_mapTemplateIDToUUIDs[templateid].front();
    if (!CGlobalFunction::Instance->m_mapUUIDToConnectionID.exist(uuid))
    {
        auto item = CGlobalFunction::Instance->m_mapUUIDToNode[uuid];
        CGlobalFunction::Instance->AddConnection(uuid, item.address(), item.port());

        return U64Result::Make_Error(format("Try connect to [{}][{}][{}:{}], please wait and retry.", templateid, uuid, item.address(), item.port()));
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
        CLua::PushString(l, "No enable datacenter, please check the configuration.");
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
    req->SetRequest( DataCenter::Functions::ConnectDatabase, ConvertObjToStr(&request), PRIORITY_LEVEL::Real_time );
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
        return U64Result::Make_Error("request data is empty");
    }

    auto res = CGlobalFunction::Instance->GetConnectionID(CGlobalFunction::Instance->m_DataCenterTemplateID.load());
    if (res.IsFialed())
    {
        return res;
    }

    return res;
}

CResult CGlobalFunction::RunSQLFunction(uint64_t session, const string &request_str, int func)
{
    auto req = make_shared<SendPackageEvent>(session);
    req->SetRequest(func, request_str, PRIORITY_LEVEL::Real_time);
    return req->SyncCall(CGlobalFunction::Instance->m_iC, CGlobalFunction::Instance->m_nTimeout);
}

// Request sql query cmd to dbcenter.
// Params:
//     1> SessionID
//     2> Cmd
int CGlobalFunction::Lua_SQLQueryToDBCenter(lua_State *l)
{
    auto SessionID = CLua::GetInteger(l, 1, -1);
    auto sql_cmd = CLua::GetString(l, 2, "");
    auto session_res = SQLFunctionPrepareCheck(l, SessionID, sql_cmd);
    if (session_res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, session_res.GetMessage());
        return 2;
    }
    auto session = session_res.GetResultObject();

    DataCenter::QuerySQLCmdRequest request;
    request.set_session(SessionID);
    request.set_sqlcmd(sql_cmd);

    auto res = RunSQLFunction(session, ConvertObjToStr(&request), (int)DataCenter::Functions::QuerySQLCmd);
    if (res.IsSucceed())
    {
        auto response = ConvertStrToObj<DataCenter::QuerySQLCmdResponse>(res.GetMessage());
        if (response->lines_size() == 0)
        {
            CLua::PushInteger(l, Base::ResultType::Succeed);
            CLua::PushInteger(l, 0);
            CLua::PushNil(l);
            return 3;
        }
        else
        {
            CLua::PushInteger(l, Base::ResultType::Succeed);
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
    auto session_res = SQLFunctionPrepareCheck(l, SessionID, sql_cmd);
    if (session_res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, session_res.GetMessage());
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
    auto session_res = SQLFunctionPrepareCheck(l, SessionID, sql_cmd);
    if (session_res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, session_res.GetMessage());
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
        CLua::PushInteger(l, Base::ResultType::Succeed);
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
    auto session_res = SQLFunctionPrepareCheck(l, SessionID, sql_cmd);
    if (session_res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, session_res.GetMessage());
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
        CLua::PushInteger(l, Base::ResultType::Succeed);
        CLua::PushInteger(l, response->affectedrows());
        return 2;
    }
    else
    {
        CLua::PushInteger(l, Base::ResultType::Error);
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
    req->SetRequest(  FileCenter::Functions::PrepareUpload, ConvertObjToStr(&request), PRIORITY_LEVEL::Real_time);
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
    req->SetRequest( FileCenter::Functions::Uploaded, ConvertObjToStr(&request), PRIORITY_LEVEL::Real_time);
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
    req->SetRequest(  FileCenter::Functions::GetThumbnail, ConvertObjToStr(&request),PRIORITY_LEVEL::Real_time);
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
    auto timeout = CLua::GetInteger(l, 5, CGlobalFunction::Instance->m_nTimeout);
    if (file_index.empty() || quality <= 0)
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "Param error.");
        return 2;
    }

    if (!FileCenter::SupportFormat_IsValid(target))
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, format("Convert [{}] to FileCenter::SupportFormat enum fialed.", target));
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
    req->SetRequest( FileCenter::Functions::ConvertImageFile, ConvertObjToStr(&request),PRIORITY_LEVEL::Real_time);
    auto res = req->SyncCall(CGlobalFunction::Instance->m_iC, timeout);
    if (res.IsFialed())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, res.GetMessage());
        return 2;
    }

    auto response = ConvertStrToObj<FileCenter::ConvertImageFileResponse>(res.GetMessage());
    CLua::PushInteger(l, Base::ResultType::Succeed);
    auto new_info = response->newfileinfo();
    map<string, string> t;
    t["index"] = new_info.index();
    t["sha256"] = new_info.sha256();
    t["size"] = new_info.size();
    t["format"] = new_info.format();
    CLua::PushTable(l, t);
    if (response->extendinfos_size() > 0)
    {
        list<map<string, string>> ex_l;
        for (auto i : response->extendinfos())
        {
            map<string, string> t;
            t["sha256"] = i.sha256();
            t["size"] = i.size();
            t["format"] = i.format();
            ex_l.push_back(t);
        }
        CLua::Push2DTable(l, ex_l);
        return 3;
    }
    else
    {
        return 2;
    }
}