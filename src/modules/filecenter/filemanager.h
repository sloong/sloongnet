/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-08-26 17:09:47
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
    typedef struct FileRange
    {
        int Start;
        int End;
        string Data;
    } FileRange;
    typedef struct UploadInfo
    {
        string Path;
        uint64_t HashCode;
        int64_t FileSize;
        list<FileRange> DataList;
    } UploadInfo;
    class FileManager : IObject
    {
    public:
        CResult Initialize(IControl *ic);
        CResult EnableDataReceive(int, int);

        PackageResult RequestPackageProcesser(DataPackage *);
        PackageResult ResponsePackageProcesser(DataPackage *);

        CResult PrepareUploadHandler(const string &str_req, DataPackage *trans_pack);
        CResult UploadingHandler(const string &str_req, DataPackage *trans_pack);
        CResult UploadedHandler(const string &str_req, DataPackage *trans_pack);
        CResult SimpleUploadHandler(const string &str_req, DataPackage *trans_pack);
        CResult DownloadVerifyHandler(const string &str_req, DataPackage *trans_pack);    
        CResult DownloadFileHandler(const string &str_req, DataPackage *trans_pack); 
        CResult ConvertImageFileHandler(const string &str_req, DataPackage *trans_pack);
        CResult GetThumbnailHandler(const string &str_req, DataPackage *trans_pack);

    protected:
        CResult ArchiveFile(uint64_t hashcode, const string& source);


        string QueryFilePath( uint64_t );
        string GetPathByHashcode( uint64_t );
        string GetFolderByHashcode( uint64_t );

        CResult MergeFile(const list<FileRange> &fileList, const string &saveFile);
        CResult SplitFile(const string &saveFile, int splitSize, map_ex<int, string> &pReadList, int* out_all_size);
        CResult GetFileSize(const string &path, int *out_size);
        void ClearCache(const string &folder);

    protected:
        map_ex<FileCenter::Functions, FunctionHandler> m_mapFuncToHandler;
        map_ex<string, UploadInfo>* m_mapTokenToUploadInfo;

        string m_strUploadTempSaveFolder = "./tmp/";

        // TODO: set the archive path.
        string m_strArchiveFolder = "./archive/";

        string m_strCacheFolder = "./Cache/";
    };

} // namespace Sloong
