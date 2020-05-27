#include "main.h"

using namespace Sloong;

map<int, string> g_RecvDataConnList;
map<string, map<string, RecvDataPackage *>> g_RecvDataInfoList;

static int g_data_pack_len = 8;
static int g_uuid_len = 36;
static int g_md5_len = 32;
static int FILE_TRANS_MAX_SIZE = 20 * 1024 * 1024; //20mb
static string g_temp_file_path = "/tmp/sloong/receivefile/temp.tmp";


FileCenter::~FileCenter()
{
    for (auto item = g_RecvDataInfoList.begin(); item != g_RecvDataInfoList.end(); item++)
    {
        ClearReceiveInfoByUUID(item->first);
    }
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
            throw normal_except(Helper::Format("Move File error. File name cannot empty. orgName:%s;newName:%s", orgName.c_str(), newName.c_str()));
        }

        if (access(orgName.c_str(), ACC_R) != 0)
        {
            nRes = -1;
            throw normal_except(Helper::Format("Move File error. Origin file not exist or can not read:[%s]", orgName.c_str()));
        }

        int res = CUniversal::CheckFileDirectory(newName);
        if (res < 0)
        {
            nRes = -1;
            throw normal_except(Helper::Format("Move File error.CheckFileDirectory error:[%s][%d]", newName.c_str(), res));
        }

        if (!Helper::MoveFile(orgName, newName))
        {
            // Move file need write access. so if move file error, try copy .
            if (!CUniversal::RunSystemCmd(Helper::Format("cp \"%s\" \"%s\"", orgName.c_str(), newName.c_str())))
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

void CGlobalFunction::ClearReceiveInfoByUUID(string uuid)
{
    m_pLog->Debug(Helper::Format("Clean receive info from g_RecvDataInfoList by :[%s]", uuid.c_str()));
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
        address.sin_port = htons((uint16_t)port);

        errno = bind(m_ListenSock, (struct sockaddr *)&address, sizeof(address));

        if (errno == -1)
            throw normal_except(Helper::Format("bind to %d field. errno = %d", port, errno));

        errno = listen(m_ListenSock, 1024);

        CThreadPool::AddWorkThread(RecvDataConnFunc, this, 1);
    }
}


//
void *Sloong::CGlobalFunction::RecvDataConnFunc(void *pParam)
{
    CGlobalFunction *pThis = (CGlobalFunction *)pParam;
    CLog *pLog = pThis->m_pLog;
    while (pThis->m_bIsRunning)
    {
        int conn_sock = -1;
        if ((conn_sock = accept(pThis->m_ListenSock, NULL, NULL)) > 0)
        {
            pLog->Debug(Helper::Format("Accept data connect :[%s][%d]", CUtility::GetSocketIP(conn_sock).c_str(), CUtility::GetSocketPort(conn_sock)));
            // When accept the connect , receive the uuid data. and
            char *pCheckBuf = new char[g_uuid_len + 1];
            memset(pCheckBuf, 0, g_uuid_len + 1);
            // In Check function, client need send the check key in 3 second.
            // 这里仍然使用Universal提供的ReceEx。这里不需要进行SSL接收
            int nLen = CUniversal::RecvEx(conn_sock, pCheckBuf, g_uuid_len, pThis->m_nRecvDataTimeoutTime);
            // Check uuid length
            if (nLen != g_uuid_len)
            {
                pLog->Warn(Helper::Format("The uuid length error:[%d]. Close connect.", nLen));
                close(conn_sock);
                continue;
            }
            // Check uuid validity
            if (g_RecvDataInfoList.find(pCheckBuf) == g_RecvDataInfoList.end())
            {
                pLog->Warn(Helper::Format("The uuid is not find in list:[%s]. Close connect.", pCheckBuf));
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
    return nullptr;
}

void *Sloong::CGlobalFunction::RecvFileFunc(void *pParam)
{
    CGlobalFunction *pThis = CGlobalFunction::g_pThis;
    CLog *pLog = pThis->m_pLog;
    int *pSock = (int *)pParam;
    int conn_sock = *pSock;
    SAFE_DELETE(pSock);
    // Find the recv uuid.

    auto conn_item = g_RecvDataConnList.find(conn_sock);
    if (conn_item == g_RecvDataConnList.end())
    {
        pLog->Error("The socket id is not find in conn list.");
        return nullptr;
    }
    string uuid = conn_item->second;
    pLog->Info(Helper::Format("Start thread to receive file data for :[%s]", uuid.c_str()));
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
            int nRecvSize = CUniversal::RecvTimeout(conn_sock, pLongBuffer, g_data_pack_len, pThis->m_nRecvDataTimeoutTime);
            if (nRecvSize <= 0)
            {
                // 读取错误,将这个连接从监听中移除并关闭连接
                SAFE_DELETE_ARR(pLongBuffer);
                pLog->Warn("Recv data package length error.");
                throw normal_except();
            }
            else
            {

                auto dlen = Helper::BytesToInt64(pLongBuffer);
                SAFE_DELETE_ARR(pLongBuffer);
                // package length cannot big than 2147483648. this is max value for int.
                if (dlen <= 0 || dlen > FILE_TRANS_MAX_SIZE || nRecvSize != g_data_pack_len)
                {
                    pLog->Error("Receive data length error.");
                    throw normal_except();
                }
                int dtlen = (int)dlen;

                char *szMD5 = new char[g_md5_len + 1];
                memset(szMD5, 0, g_md5_len + 1);
                nRecvSize = CUniversal::RecvTimeout(conn_sock, szMD5, g_md5_len, pThis->m_nRecvDataTimeoutTime, true);
                if (nRecvSize <= 0)
                {
                    pLog->Error("Receive data package md5 error.");
                    SAFE_DELETE_ARR(szMD5);
                    throw normal_except();
                }
                string trans_md5 = string(szMD5);
                Helper::tolower(trans_md5);

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
                    nRecvSize = CUniversal::RecvTimeout(conn_sock, pData, nOnceRecvLen, pThis->m_nRecvDataTimeoutTime, true);
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
                Helper::tolower(file_md5);

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

        pLog->Debug(Helper::Format("Receive connect done. close:[%s:%d]", CUtility::GetSocketIP(conn_sock).c_str(), CUtility::GetSocketPort(conn_sock)));
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
    for ( auto item : *fileList )
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

    g_RecvDataInfoList[uuid] = recv_list;

    CLua::PushString(l, uuid);
    return 1;
}



int CGlobalFunction::Lua_CheckRecvStatus(lua_State *l)
{
    string uuid = CLua::GetString(l, 1);
    string md5 = CLua::GetString(l, 2);
    Helper::tolower(md5);
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