/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-03-26 17:28:54
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
        string SHA256;
        int64_t FileSize;
        list<FileRange> DataList;
    } UploadInfo;
    class FileManager : IObject
    {
    public:
        CResult Initialize(IControl *ic);
        CResult EnableDataReceive(int, int);

        PackageResult RequestPackageProcesser(Package *);
        PackageResult ResponsePackageProcesser(Package *);

        CResult PrepareUploadHandler(const string &str_req, Package *trans_pack);
        CResult UploadingHandler(const string &str_req, Package *trans_pack);
        CResult UploadedHandler(const string &str_req, Package *trans_pack);
        CResult SimpleUploadHandler(const string &str_req, Package *trans_pack);
        CResult DownloadVerifyHandler(const string &str_req, Package *trans_pack);    
        CResult DownloadFileHandler(const string &str_req, Package *trans_pack); 
        CResult ConvertImageFileHandler(const string &str_req, Package *trans_pack);
        CResult GetThumbnailHandler(const string &str_req, Package *trans_pack);

    protected:
        CResult ArchiveFile(const string& , const string& );

        string GetFileTruePath( const string&  );
        string GetFileFolder( const string&  );

        CResult MergeFile(const list<FileRange> &fileList, const string &saveFile);
        CResult SplitFile(const string &saveFile, int splitSize, map_ex<int, string> &pReadList, int* out_all_size);
        int GetFileSize(const string &path);
        void ClearCache(const string &folder);

    protected:
        map_ex<FileCenter::Functions, FunctionHandler> m_mapFuncToHandler;
        map_ex<string, UploadInfo>* m_mapTokenToUploadInfo;

        string m_strUploadTempSaveFolder = "./tmp/";
        string m_strCacheFolder = "./cache/";
        string m_strArchiveFolder = "./archive/";
    };

} // namespace Sloong
