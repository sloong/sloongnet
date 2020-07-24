/*** 
 * @Author: Chuanbin Wang
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-24 15:47:22
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/filecenter/filemanager.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#pragma once

#include "IObject.h"

#include "protocol/filecenter.pb.h"
using namespace FileCenter;

namespace Sloong
{
    typedef struct UploadInfo
    {
        string Path;
        string Name;
        string Hash_MD5;
        map_ex<int,string> SplitPackage;
    } UploadInfo;
    typedef struct DownloadInfo
    {
        string RealPath;
        string Hash_MD5;
        int Size;
        map_ex<int,string> SplitPackage;
    } DownloadInfo;
    // Extend function handler, return value have extend element.
    typedef std::function<SResult(const string &, DataPackage *)> ExtendFunctionHandler;
    class FileManager : IObject
    {
    public:
        CResult Initialize(IControl *ic);
        CResult EnableDataReceive(int, int);

        PackageResult RequestPackageProcesser(DataPackage *);
        PackageResult ResponsePackageProcesser(DataPackage *);

        SResult PrepareUploadHandler(const string &str_req, DataPackage *trans_pack);
        SResult UploadingHandler(const string &str_req, DataPackage *trans_pack);
        SResult UploadedHandler(const string &str_req, DataPackage *trans_pack);
        SResult PrepareDownloadHandler(const string &str_req, DataPackage *trans_pack);
        SResult DownloadingHandler(const string &str_req, DataPackage *trans_pack);
        SResult DownloadedHandler(const string &str_req, DataPackage *trans_pack);
        SResult TestSpeedHandler(const string &str_req, DataPackage *trans_pack);

    protected:
        CResult MoveFile(const string &source, const string &target);

        CResult QueryFilePath(const string &);

        CResult MergeFile(const map_ex<int, string> &fileList, const string &saveFile);
        CResult SplitFile(const string &saveFile, int splitSize, map_ex<int, string> &pReadList, int* out_all_size);
        CResult GetFileSize(const string &path, int *out_size);
        void ClearCache(const string &folder);

    protected:
        RUN_STATUS m_emStatus = RUN_STATUS::Created;
        map_ex<FileCenter::Functions, ExtendFunctionHandler> m_mapFuncToHandler;
        map_ex<string, UploadInfo> m_mapTokenToUploadInfo;
        map_ex<string, DownloadInfo> m_mapTokenToDownloadInfo;

        string m_strUploadTempSaveFolder;

        // TODO: set the archive path.
        string m_strArchiveFolder = "/tmp";
    };

} // namespace Sloong
