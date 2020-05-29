#ifndef SLOONGNET_MODULE_FILECENTER_FILEMANAGER_H
#define SLOONGNET_MODULE_FILECENTER_FILEMANAGER_H

#include "IObject.h"
#include "DataTransPackage.h"

#include "protocol/filecenter.pb.h"
using namespace FileCenter;

namespace Sloong
{
    enum RecvStatus
    {
        Wait = 0,
        Receiving = 1,
        Saveing = 2,
        Done = 3,
        VerificationError = 4,
        OtherError = 5,
    };
    struct RecvDataPackage
    {
        string strMD5 = "";
        RecvStatus emStatus = RecvStatus::Wait;
        string strName = "";
        string strPath = "";
    };

    typedef std::function<CResult(const string &, CDataTransPackage *)> FunctionHandler;
    class FileManager : IObject
    {
    public:
        ~FileManager()
        {
            for (auto &item : g_RecvDataInfoList)
            {
                ClearReceiveInfoByUUID(item.first);
            }
        }

        CResult Initialize(IControl *ic);
        CResult EnableDataReceive(int,int);

        CResult RequestPackageProcesser(CDataTransPackage *);
        CResult ResponsePackageProcesser(CDataTransPackage *);

        CResult PrepareSendFile(const string &str_req, CDataTransPackage *trans_pack);
        CResult ReceiveFile(const string &str_req, CDataTransPackage *trans_pack);

        

    protected:
        void RecvDataConnFunc();
        void ClearReceiveInfoByUUID(string uuid);
        CResult MoveFile(const string &source, const string &target);

        void RecvFileFunc(int conn_sock);

    protected:
        RUN_STATUS  m_emStatus = RUN_STATUS::Created;
        map_ex<FileCenter::Functions, FunctionHandler> m_mapFuncToHandler;
        int m_ListenSock ;
        map<int, string> g_RecvDataConnList;
        int m_nRecvDataTimeoutTime;
        map<string, map<string, RecvDataPackage *>> g_RecvDataInfoList;

    };

    static int g_data_pack_len = 8;
    static int g_uuid_len = 36;
    static int g_md5_len = 32;
    static int FILE_TRANS_MAX_SIZE = 20 * 1024 * 1024; //20mb
    static string g_temp_file_path = "/tmp/sloong/receivefile/temp.tmp";
} // namespace Sloong

#endif