#ifndef SLOONGNET_MODULE_FILECENTER_FILEMANAGER_H
#define SLOONGNET_MODULE_FILECENTER_FILEMANAGER_H

#include "IObject.h"
#include "DataTransPackage.h"

#include "protocol/filecenter.pb.h"
using namespace FileCenter;

namespace Sloong
{
    typedef struct UploadInfo
    {
        string Path;
        string Name;
        string Hash_MD5;
        map_ex<int, string> SplitPackageFile;
    } UploadInfo;
    typedef struct DownloadInfo
    {
        string RealPath;
        string CacheFolder;
        string Hash_MD5;
        int Size;
        map_ex<int, string> SplitPackageFile;
    } DownloadInfo;
    
    class FileManager : IObject
    {
    public:
        CResult Initialize(IControl *ic);
        CResult EnableDataReceive(int, int);

        CResult RequestPackageProcesser(CDataTransPackage *);
        CResult ResponsePackageProcesser(CDataTransPackage *);

        CResult PrepareUploadHandler(const string &str_req, CDataTransPackage *trans_pack);
        CResult UploadingHandler(const string &str_req, CDataTransPackage *trans_pack);
        CResult UploadedHandler(const string &str_req, CDataTransPackage *trans_pack);
        CResult PrepareDownloadHandler(const string &str_req, CDataTransPackage *trans_pack);
        CResult DownloadingHandler(const string &str_req, CDataTransPackage *trans_pack);
        CResult DownloadedHandler(const string &str_req, CDataTransPackage *trans_pack);
        CResult TestSpeedHandler(const string &str_req, CDataTransPackage *trans_pack);

    protected:
        CResult MoveFile(const string &source, const string &target);

        CResult QueryFilePath(const string &);

        CResult MergeFile(const map_ex<int, string> &fileList, const string &saveFile);
        CResult SplitFile(const string &saveFile, map_ex<int, string> &fileList);
        CResult GetFileSize(const string &path, int *out_size);
        void ClearCache(const string &folder);

    protected:
        RUN_STATUS m_emStatus = RUN_STATUS::Created;
        map_ex<FileCenter::Functions, FunctionHandler> m_mapFuncToHandler;
        map_ex<string, UploadInfo> m_mapTokenToUploadInfo;
        map_ex<string, DownloadInfo> m_mapTokenToDownloadInfo;

        string m_strUploadTempSaveFolder;

        // TODO: set the archive path.
        string m_strArchiveFolder = "/tmp";
        string m_strCacheFolder = "/tmp";
    };

} // namespace Sloong

#endif