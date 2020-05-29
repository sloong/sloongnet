#include "filemanager.h"

using namespace Sloong;

CResult Sloong::FileManager::Initialize(IControl *ic)
{
    IObject::Initialize(ic);
    m_mapFuncToHandler[Functions::UploadStart] = std::bind(&FileManager::PrepareSendFile, this, std::placeholders::_1, std::placeholders::_2);
    return CResult::Succeed();
}

CResult Sloong::FileManager::RequestPackageProcesser(CDataTransPackage *tans_pack)
{
    auto function = (Functions)tans_pack->GetFunction();
    if (!Functions_IsValid(function))
    {
        tans_pack->ResponsePackage(ResultType::Error, Helper::Format("FileCenter no provide [%d] function.", function));
        return CResult::Succeed();
    }

    auto req_obj = tans_pack->GetRecvMessage();
    auto func_name = Functions_Name(function);
    m_pLog->Debug(Helper::Format("Request [%d][%s]:[%s]", function, func_name.c_str(), req_obj.c_str()));
    if (!m_mapFuncToHandler.exist(function))
    {
        tans_pack->ResponsePackage(ResultType::Error, Helper::Format("Function [%s] no handler.", func_name.c_str()));
        return CResult::Succeed();
    }

    auto res = m_mapFuncToHandler[function](req_obj, tans_pack);
    m_pLog->Debug(Helper::Format("Response [%s]:[%s][%s].", func_name.c_str(), ResultType_Name(res.Result()).c_str(), res.Message().c_str()));
    tans_pack->ResponsePackage(res);
    return CResult::Succeed();
}

CResult Sloong::FileManager::MoveFile(const string &source, const string &target)
{
    try
    {
        if (source.length() < 3 || target.length() < 3)
        {
            return CResult::Make_Error(Helper::Format("Move File error. File name cannot empty. source:%s;target:%s", source.c_str(), target.c_str()));
        }

        if (access(source.c_str(), ACC_R) != 0)
        {
            return CResult::Make_Error(Helper::Format("Move File error. Origin file not exist or can not read:[%s]", source.c_str()));
        }

        auto res = CUniversal::CheckFileDirectory(target);
        if (res < 0)
        {
            return CResult::Make_Error(Helper::Format("Move File error.CheckFileDirectory error:[%s][%d]", target.c_str(), res));
        }

        if (!Helper::MoveFile(source, target))
        {
            // Move file need write access. so if move file error, try copy .
            if (!CUniversal::RunSystemCmd(Helper::Format("cp \"%s\" \"%s\"", source.c_str(), target.c_str())))
            {
                return CResult::Make_Error("Move File and try copy file error.");
            }
            return CResult::Succeed();
        }
    }
    catch (const exception &e)
    {
        m_pLog->Error(e.what());
        return CResult::Make_Error(e.what());
    }

    return CResult::Succeed();
}

CResult Sloong::FileManager::EnableDataReceive(int port, int timtout)
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
        return CResult::Make_Error(Helper::Format("bind to %d field. errno = %d", port, errno));

    errno = listen(m_ListenSock, 1024);

    m_nRecvDataTimeoutTime = timtout;
    CThreadPool::AddWorkThread(std::bind(&FileManager::RecvDataConnFunc, this), 1);
    return CResult::Succeed();
}

void FileManager::ClearReceiveInfoByUUID(string uuid)
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

CResult Sloong::FileManager::PrepareSendFile(const string &str_req, CDataTransPackage *trans_pack)
{
    auto req = ConvertStrToObj<PrepareSendFileRequest>(str_req);
    auto &filename = req->filename();
    if (filename.length() < 1)
        return CResult::Make_Error("Param is empty.");

    char *pBuf = nullptr;
    auto nSize = CUtility::ReadFile(filename, pBuf);
    if (nSize < 0)
    {
        return CResult::Make_Error(Helper::Format("Cannot access target file[%s].", filename.c_str()));
    }

    auto uuid = CUtility::GenUUID();

    m_iC->AddTemp(uuid, pBuf);

    PrepareSendFileResponse res;
    res.set_filesize(nSize);
    res.set_token(uuid);
    return CResult::Make_OK(ConvertObjToStr(&res));
}

void Sloong::FileManager::RecvDataConnFunc()
{
    CLog *pLog = m_pLog;
    while (m_emStatus != RUN_STATUS::Exit)
    {
        int conn_sock = -1;
        if ((conn_sock = accept(m_ListenSock, NULL, NULL)) > 0)
        {
            pLog->Debug(Helper::Format("Accept data connect :[%s][%d]", CUtility::GetSocketIP(conn_sock).c_str(), CUtility::GetSocketPort(conn_sock)));
            // When accept the connect , receive the uuid data. and
            char *pCheckBuf = new char[g_uuid_len + 1];
            memset(pCheckBuf, 0, g_uuid_len + 1);
            // In Check function, client need send the check key in 3 second.
            // 这里仍然使用Universal提供的ReceEx。这里不需要进行SSL接收
            int nLen = CUniversal::RecvEx(conn_sock, pCheckBuf, g_uuid_len, m_nRecvDataTimeoutTime);
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
           
            // TODO: modify the libarary function.
            //CThreadPool::AddWorkThread(std::bind(&FileManager::RecvFileFunc, this, std::placeholders::_1),  make_shared<int>(conn_sock));
        }
    }
}

void Sloong::FileManager::RecvFileFunc(int conn_sock)
{
    CLog *pLog = m_pLog;
    // Find the recv uuid.
    auto conn_item = g_RecvDataConnList.find(conn_sock);
    if (conn_item == g_RecvDataConnList.end())
    {
        pLog->Error("The socket id is not find in conn list.");
        return;
    }
    string uuid = conn_item->second;
    pLog->Info(Helper::Format("Start thread to receive file data for :[%s]", uuid.c_str()));
    // Find the recv info list.
    auto info_item = g_RecvDataInfoList.find(uuid);
    if (info_item == g_RecvDataInfoList.end())
    {
        pLog->Error("The uuid is not find in info list.");
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
            int nRecvSize = CUniversal::RecvTimeout(conn_sock, pLongBuffer, g_data_pack_len, m_nRecvDataTimeoutTime);
            if (nRecvSize <= 0)
            {
                // 读取错误,将这个连接从监听中移除并关闭连接
                SAFE_DELETE_ARR(pLongBuffer);
                pLog->Warn("Recv data package length error.");
                return;
            }
            else
            {

                auto dlen = Helper::BytesToInt64(pLongBuffer);
                SAFE_DELETE_ARR(pLongBuffer);
                // package length cannot big than 2147483648. this is max value for int.
                if (dlen <= 0 || dlen > FILE_TRANS_MAX_SIZE || nRecvSize != g_data_pack_len)
                {
                    pLog->Error("Receive data length error.");
                    return;
                }
                int dtlen = (int)dlen;

                char *szMD5 = new char[g_md5_len + 1];
                memset(szMD5, 0, g_md5_len + 1);
                nRecvSize = CUniversal::RecvTimeout(conn_sock, szMD5, g_md5_len, m_nRecvDataTimeoutTime, true);
                if (nRecvSize <= 0)
                {
                    pLog->Error("Receive data package md5 error.");
                    SAFE_DELETE_ARR(szMD5);
                    return;
                }
                string trans_md5 = string(szMD5);
                Helper::tolower(trans_md5);

                auto recv_file_item = recv_file_list.find(trans_md5);
                if (recv_file_item == recv_file_list.end())
                {
                    pLog->Error("the file md5 is not find in recv list.");
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
                    nRecvSize = CUniversal::RecvTimeout(conn_sock, pData, nOnceRecvLen, m_nRecvDataTimeoutTime, true);
                    if (nRecvSize < 0)
                    {
                        pLog->Error("Receive data error.");
                        SAFE_DELETE_ARR(data);
                        return;
                    }
                    else if (nRecvSize == 0)
                    {
                        pLog->Error("Receive data timeout.");
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
    }
    catch (const std::exception &)
    {
        close(conn_sock);
        ClearReceiveInfoByUUID(uuid);
    }
}

// Receive File funcs
// Client requeset with file list info
// and here add the info to list and Build one uuid and return this uuid.
CResult FileManager::ReceiveFile(const string &str_req, CDataTransPackage *trans_pack)
{
    auto req = ConvertStrToObj<ReceiveFileRequest>(str_req);

    string save_folder = req->savefolder();
    string uuid = CUtility::GenUUID();
    // The file list, key is md5 ,value is file name

    map<string, RecvDataPackage *> recv_list;
    for (auto &item : req->infos())
    {
        RecvDataPackage *pack = new RecvDataPackage();
        string md5 = item.filehash();
        Helper::tolower(md5);
        pack->strName = item.filename();
        pack->strPath = save_folder;
        pack->strMD5 = md5;
        pack->emStatus = RecvStatus::Wait;
        recv_list[md5] = pack;
    }

    g_RecvDataInfoList[uuid] = recv_list;

    ReceiveFileResponse res;
    res.set_token(uuid);
    return CResult::Make_OK(ConvertObjToStr(&res));
}
/*
int FileManager::CheckRecvStatus(const string &str_req, CDataTransPackage *trans_pack)
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
}*/