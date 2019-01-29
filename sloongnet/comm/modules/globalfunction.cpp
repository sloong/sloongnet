#include "globalfunction.h"
// sys
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
// univ
#include <univ/Base64.h>

#include "utility.h"
#include "version.h"
#include "configuation.h"
#include "epollex.h"
#include "NormalEvent.h"
#include "configuation.h"
#include "IData.h"

using namespace std;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;

CGlobalFunction* CGlobalFunction::g_pThis = NULL;
mutex g_SQLMutex;
map<int, string> g_RecvDataConnList;
map<string, map<string, RecvDataPackage *>> g_RecvDataInfoList;

static int g_data_pack_len = 8;
static int g_uuid_len = 36;
static int g_md5_len = 32;

static string g_temp_file_path = "/tmp/sloong/receivefile/temp.tmp";

LuaFunctionRegistr g_LuaFunc[] =
    {
        {"Sloongnet_ShowLog", CGlobalFunction::Lua_ShowLog},
        {"Sloongnet_GetEngineVer", CGlobalFunction::Lua_GetEngineVer},
        {"Sloongnet_Base64_encode", CGlobalFunction::Lua_Base64_Encode},
        {"Sloongnet_Base64_decode", CGlobalFunction::Lua_Base64_Decode},
        {"Sloongnet_Hash_Encode", CGlobalFunction::Lua_Hash_Encode},
        {"Sloongnet_SendFile", CGlobalFunction::Lua_SendFile},
        {"Sloongnet_ReloadScript", CGlobalFunction::Lua_ReloadScript},
        {"Sloongnet_Get", CGlobalFunction::Lua_GetConfig},
        {"Sloongnet_MoveFile", CGlobalFunction::Lua_MoveFile},
        {"Sloongnet_GenUUID", CGlobalFunction::Lua_GenUUID},
        {"Sloongnet_ReceiveFile", CGlobalFunction::Lua_ReceiveFile},
        {"Sloongnet_CheckRecvStatus", CGlobalFunction::Lua_CheckRecvStatus},
        {"Sloongnet_SetCommData", CGlobalFunction::Lua_SetCommData},
        {"Sloongnet_GetCommData", CGlobalFunction::Lua_GetCommData},
        {"Sloongnet_GetLogObject", CGlobalFunction::Lua_GetLogObject},
};

CGlobalFunction::CGlobalFunction()
{
    m_pUtility = new CUtility();
    g_pThis = this;
}

CGlobalFunction::~CGlobalFunction()
{
    SAFE_DELETE(m_pUtility);
    for (auto item = g_RecvDataInfoList.begin(); item != g_RecvDataInfoList.end(); item++)
    {
        ClearReceiveInfoByUUID(item->first);
    }
}

void CGlobalFunction::ClearReceiveInfoByUUID(string uuid)
{
    m_pLog->Debug(CUniversal::Format("Clean receive info from g_RecvDataInfoList by :[%s]", uuid));
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

void Sloong::CGlobalFunction::Initialize(IControl *iMsg)
{
    IObject::Initialize(iMsg);

    CConfiguation *pConfig = IData::GetServerConfig();
    if (pConfig->m_oDataConfig.datareceiveport()>0)
    {
        EnableDataReceive(pConfig->m_oDataConfig.datareceiveport());
    }
}

void Sloong::CGlobalFunction::Exit()
{
    m_bIsRunning = false;
}

void Sloong::CGlobalFunction::RegistFuncToLua(CLua *pLua)
{
    vector<LuaFunctionRegistr> funcList(g_LuaFunc, g_LuaFunc + ARRAYSIZE(g_LuaFunc));
    pLua->AddFunctions(&funcList);
}

void Sloong::CGlobalFunction::EnableDataReceive(int port)
{
    if (port > 0)
    {
        m_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
        int sock_op = 1;
        // SOL_SOCKET:在socket层面设置
        // SO_REUSEADDR:允许套接字和一个已在使用中的地址捆绑
        setsockopt(m_ListenSock, SOL_SOCKET, SO_REUSEADDR, &sock_op, sizeof(sock_op));

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);

        errno = bind(m_ListenSock, (struct sockaddr *)&address, sizeof(address));

        if (errno == -1)
            throw normal_except(CUniversal::Format("bind to %d field. errno = %d", port, errno));

        errno = listen(m_ListenSock, 1024);

        CThreadPool::AddWorkThread(RecvDataConnFunc, this, 1);
    }
}

//
void *Sloong::CGlobalFunction::RecvDataConnFunc(void *pParam)
{
    CGlobalFunction *pThis = (CGlobalFunction *)pParam;
    CLog *pLog = pThis->m_pLog;
    CConfiguation *pConfig = TYPE_TRANS<CConfiguation *>(pThis->m_iC->Get(DATA_ITEM::Configuation));
    while (pThis->m_bIsRunning)
    {
        int conn_sock = -1;
        if ((conn_sock = accept(pThis->m_ListenSock, NULL, NULL)) > 0)
        {
            pLog->Verbos(CUniversal::Format("Accept data connect :[%s][%d]", CUtility::GetSocketIP(conn_sock), CUtility::GetSocketPort(conn_sock)));
            // When accept the connect , receive the uuid data. and
            char *pCheckBuf = new char[g_uuid_len + 1];
            memset(pCheckBuf, 0, g_uuid_len + 1);
            // In Check function, client need send the check key in 3 second.
            // 这里仍然使用Universal提供的ReceEx。这里不需要进行SSL接收
            int nLen = CUniversal::RecvEx(conn_sock, pCheckBuf, g_uuid_len, pConfig->m_oDataConfig.datarecvtime());
            // Check uuid length
            if (nLen != g_uuid_len)
            {
                pLog->Warn(CUniversal::Format("The uuid length error:[%d]. Close connect.", nLen));
                close(conn_sock);
                continue;
            }
            // Check uuid validity
            if (g_RecvDataInfoList.find(pCheckBuf) == g_RecvDataInfoList.end())
            {
                pLog->Warn(CUniversal::Format("The uuid is not find in list:[%s]. Close connect.", pCheckBuf));
                close(conn_sock);
                continue;
            }
            // Add to connect list
            g_RecvDataConnList[conn_sock] = pCheckBuf;
            // Start new thread to recv data for this connect.
            int *pSock = new int();
            *pSock = conn_sock;
            CThreadPool::AddWorkThread(RecvFileFunc, pSock);
        }
    }
}

void *Sloong::CGlobalFunction::RecvFileFunc(void *pParam)
{
    CGlobalFunction *pThis = CGlobalFunction::g_pThis;
    CLog *pLog = pThis->m_pLog;
    int *pSock = (int *)pParam;
    int conn_sock = *pSock;
    CConfiguation *pConfig = TYPE_TRANS<CConfiguation *>(pThis->m_iC->Get(DATA_ITEM::Configuation));
    SAFE_DELETE(pSock);
    // Find the recv uuid.

    auto conn_item = g_RecvDataConnList.find(conn_sock);
    if (conn_item == g_RecvDataConnList.end())
    {
        pLog->Error("The socket id is not find in conn list.");
        return nullptr;
    }
    string uuid = conn_item->second;
    pLog->Info(CUniversal::Format("Start thread to receive file data for :[%s]", uuid));
    // Find the recv info list.
    auto info_item = g_RecvDataInfoList.find(uuid);
    if (info_item == g_RecvDataInfoList.end())
    {
        pLog->Error("The uuid is not find in info list.");
        return nullptr;
    }
    try
    {
        map<string, RecvDataPackage *> recv_file_list = info_item->second;
        bool bLoop = false;
        do
        {
            char *pLongBuffer = new char[g_data_pack_len + 1](); //dataLeng;
            memset(pLongBuffer, 0, g_data_pack_len + 1);
            int nRecvSize = CUniversal::RecvEx(conn_sock, pLongBuffer, g_data_pack_len, pConfig->m_oDataConfig.datarecvtime());
            if (nRecvSize <= 0)
            {
                // 读取错误,将这个连接从监听中移除并关闭连接
                SAFE_DELETE_ARR(pLongBuffer);
                pLog->Warn("Recv data package length error.");
                throw normal_except();
            }
            else
            {
                long long dtlen = CUniversal::BytesToLong(pLongBuffer);
                SAFE_DELETE_ARR(pLongBuffer);
                // package length cannot big than 2147483648. this is max value for int.
                if (dtlen <= 0 || dtlen > 2147483648 || nRecvSize != g_data_pack_len)
                {
                    pLog->Error("Receive data length error.");
                    throw normal_except();
                }

                char *szMD5 = new char[g_md5_len + 1];
                memset(szMD5, 0, g_md5_len + 1);
                nRecvSize = CUniversal::RecvEx(conn_sock, szMD5, g_md5_len, pConfig->m_oDataConfig.serverconfig().receivetime(), true);
                if (nRecvSize <= 0)
                {
                    pLog->Error("Receive data package md5 error.");
                    SAFE_DELETE_ARR(szMD5);
                    throw normal_except();
                }
                string trans_md5 = string(szMD5);
                CUniversal::tolower(trans_md5);

                auto recv_file_item = recv_file_list.find(trans_md5);
                if (recv_file_item == recv_file_list.end())
                {
                    pLog->Error("the file md5 is not find in recv list.");
                    throw normal_except();
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
                    nRecvSize = CUniversal::RecvEx(conn_sock, pData, nOnceRecvLen, pConfig->m_oDataConfig.serverconfig().receivetime(), true);
                    if (nRecvSize < 0)
                    {
                        pLog->Error("Receive data error.");
                        SAFE_DELETE_ARR(data);
                        throw normal_except();
                    }
                    else if (nRecvSize == 0)
                    {
                        pLog->Error("Receive data timeout.");
                        SAFE_DELETE_ARR(data);
                        throw normal_except();
                    }
                    else
                    {
                        pData += nRecvSize;
                        nRecvdLen += nRecvSize;
                    }
                }

                pack->emStatus = RecvStatus::Saveing;

                // check target file path is not exist
                CUniversal::CheckFileDirectory(pack->strPath);

                // save to file
                ofstream of;
                of.open(pack->strPath + pack->strName, ios::out | ios::trunc | ios::binary);
                of.write(data, dtlen);
                of.close();
                SAFE_DELETE_ARR(data);

                string file_md5 = CMD5::Encode(pack->strPath + pack->strName, true);
                CUniversal::tolower(file_md5);

                // check md5
                if (trans_md5.compare(file_md5))
                {
                    pLog->Error("the file data is different with md5 code.");
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

        pLog->Verbos(CUniversal::Format("Receive connect done. close:[%s:%d]", CUtility::GetSocketIP(conn_sock), CUtility::GetSocketPort(conn_sock)));
        close(conn_sock);
        return nullptr;
    }
    catch (const std::exception &)
    {
        close(conn_sock);
        pThis->ClearReceiveInfoByUUID(uuid);
        return nullptr;
    }
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

/**
 * @Remarks: 
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

    switch (  hash_type)
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

int Sloong::CGlobalFunction::Lua_SendFile(lua_State *l)
{
    auto filename = CLua::GetString(l, 1, "");
    if (filename == "")
    {
        CLua::PushDouble(l, -1);
        CLua::PushString(l, "Param is empty.");
        return 2;
    }

    char *pBuf = NULL;
    int nSize = 0;
    try
    {
        nSize = CUtility::ReadFile(filename, pBuf);
    }
    catch (normal_except &e)
    {
        g_pThis->m_pLog->Error(e.what());
        CLua::PushDouble(l, -1);
        CLua::PushString(l, e.what());
        return 2;
    }

    auto uuid = CUtility::GenUUID();

    g_pThis->m_iC->AddTemp("SendList" + uuid, pBuf);
    CLua::PushDouble(l, nSize);
    CLua::PushString(l, uuid);
    return 2;
}

int Sloong::CGlobalFunction::Lua_ReloadScript(lua_State *l)
{
    g_pThis->m_iC->SendMessage(MSG_TYPE::ReloadLuaContext);
    return 0;
}

int Sloong::CGlobalFunction::Lua_GetConfig(lua_State *l)
{
    string section = CLua::GetString(l, 1);
    string key = CLua::GetString(l, 2);
    string def = CLua::GetString(l, 3);
    CConfiguation *pConfig = TYPE_TRANS<CConfiguation *>(g_pThis->m_iC->Get(DATA_ITEM::Configuation));
    string value("");
    try
    {
        value = pConfig->GetStringConfig("config",section, key, def);
    }
    catch (normal_except &e)
    {
        CLua::PushString(l, "");
        CLua::PushString(l, e.what());
        return 2;
    }

    CLua::PushString(l, value);
    return 1;
}

int Sloong::CGlobalFunction::Lua_MoveFile(lua_State *l)
{
    string orgName = CLua::GetString(l, 1, "");
    string newName = CLua::GetString(l, 2, "");
    int nRes(0);
    try
    {
        if (orgName == "" || newName == "")
        {
            nRes = -2;
            throw normal_except(CUniversal::Format("Move File error. File name cannot empty. orgName:%s;newName:%s", orgName, newName));
        }

        if (access(orgName.c_str(), ACC_R) != 0)
        {
            nRes = -1;
            throw normal_except(CUniversal::Format("Move File error. Origin file not exist or can not read:[%s]", orgName));
        }

        int res = CUniversal::CheckFileDirectory(newName);
        if (res < 0)
        {
            nRes = -1;
            throw normal_except(CUniversal::Format("Move File error.CheckFileDirectory error:[%s][%d]", newName, res));
        }

        if (!CUniversal::MoveFile(orgName, newName))
        {
            // Move file need write access. so if move file error, try copy .
            if(!CUniversal::RunSystemCmd( CUniversal::Format("cp \"%s\" \"%s\"",orgName,newName)))
            {
                nRes = -3;
                throw normal_except("Move File and try copy file error.");
            }
            nRes = 1;
        }

    }
    catch (normal_except &e)
    {
        g_pThis->m_pLog->Error(e.what());
        CLua::PushInteger(l, nRes);
        CLua::PushString(l, e.what());
        return 2;
    }

    // if succeed return 0, else return nozero
    CLua::PushInteger(l, nRes);
    CLua::PushString(l, "mv file succeed");
    return 2;
}

int CGlobalFunction::Lua_GenUUID(lua_State *l)
{
    CLua::PushString(l, CUtility::GenUUID());
    return 1;
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
    for (auto i = fileList.begin(); i != fileList.end(); i++)
    {
        RecvDataPackage *pack = new RecvDataPackage();
        string md5 = i->first;
        CUniversal::tolower(md5);
        pack->strName = i->second;
        pack->strPath = save_folder;
        pack->strMD5 = md5;
        pack->emStatus = RecvStatus::Wait;
        recv_list[md5] = pack;
    }

    g_RecvDataInfoList[uuid] = recv_list;

    CLua::PushString(l, uuid);
    return 1;
}

int CGlobalFunction::Lua_CheckRecvStatus(lua_State *l)
{
    string uuid = CLua::GetString(l, 1);
    string md5 = CLua::GetString(l, 2);
    CUniversal::tolower(md5);
    auto recv_list = g_RecvDataInfoList[uuid];
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

int CGlobalFunction::Lua_ShowLog(lua_State *l)
{
    string luaTitle = CLua::GetString(l, 2, "Info");
    string msg = CLua::GetString(l, 1);
    if (msg == "")
        return 0;
    else
        msg = CUniversal::Format("[Script]:[%s]", msg);

    auto log = g_pThis->m_pLog;
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
}

int CGlobalFunction::Lua_GetCommData(lua_State *l)
{
}

int CGlobalFunction::Lua_GetLogObject(lua_State *l)
{
    CLua::PushPointer(l, g_pThis->m_pLog);
    return 1;
}