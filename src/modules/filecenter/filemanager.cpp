#include "filemanager.h"

using namespace Sloong;

CResult Sloong::FileManager::Initialize(IControl *ic)
{
    IObject::Initialize(ic);
    m_mapFuncToHandler[Functions::TestSpeed] = std::bind(&FileManager::TestSpeedHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::PrepareUpload] = std::bind(&FileManager::PrepareUploadHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Uploading] = std::bind(&FileManager::UploadingHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Uploaded] = std::bind(&FileManager::UploadedHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::PrepareDownload] = std::bind(&FileManager::PrepareDownloadHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Downloading] = std::bind(&FileManager::DownloadingHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Downloaded] = std::bind(&FileManager::DownloadedHandler, this, std::placeholders::_1, std::placeholders::_2);
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
    m_pLog->Debug(Helper::Format("Response [%s]:[%s][%s].", func_name.c_str(), ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str()));
    tans_pack->ResponsePackage(res);
    return CResult::Succeed();
}

CResult Sloong::FileManager::MergeFile(const map_ex<int,string> &fileList, const string &saveFile)
{
    string files;
    for (auto &item : fileList)
        files += item.second + " ";
    auto res = CUniversal::RunSystemCmdAndGetResult(Helper::Format("cat %s > %s", files.c_str(), saveFile.c_str()));
    return CResult::Succeed();
}

CResult Sloong::FileManager::SplitFile(const string &saveFile, map_ex<int,string> &fileList)
{
    /*
    split [选项]... [要切割的文件 [输出文件前缀]]
    -a, --suffix-length=N   使用长度为 N 的后缀 (默认 2)
    -b, --bytes=SIZE        设置输出文件的大小。支持单位：m,k
    -C, --line-bytes=SIZE   设置输出文件的最大行数。与 -b 类似，但会尽量维持每行的完整性
    -d, --numeric-suffixes  使用数字后缀代替字母
    -l, --lines=NUMBER      设备输出文件的行数
        --help     显示版本信息
        --version  输出版本信息
    */
   return CResult::Make_Error("No readlize");
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
void Sloong::FileManager::ClearCache(const string &folder)
{
    // TODO:
    
}
CResult Sloong::FileManager::GetFileSize(const string &source, int *out_size)
{
    // TODO:
    return CResult::Make_Error("No readlize");
}
CResult Sloong::FileManager::QueryFilePath(const string &hash_md5)
{
    return CResult::Make_Error("No readlize");
}

CResult Sloong::FileManager::PrepareUploadHandler(const string &str_req, CDataTransPackage *trans_pack)
{
    auto req = ConvertStrToObj<PrepareUploadRequest>(str_req);

    auto token = CUtility::GenUUID();
    auto info = req->info();
    m_mapTokenToUploadInfo[token].Name = info.name();
    m_mapTokenToUploadInfo[token].Hash_MD5 = info.hash_md5();
    m_mapTokenToUploadInfo[token].Path = FormatFolderString(m_strUploadTempSaveFolder) + token + "/";

    PrepareUploadResponse res;
    res.set_token(token);
    return CResult::Make_OK(ConvertObjToStr(&res));
}

CResult Sloong::FileManager::UploadingHandler(const string &str_req, CDataTransPackage *trans_pack)
{
    auto req = ConvertStrToObj<UploadingRequest>(str_req);
auto& token = req->token();
    if (!m_mapTokenToUploadInfo.exist(token))
        return CResult::Make_Error("Need request PrepareUpload firest.");

    auto extend = trans_pack->GetExtendData();
    if (extend.length() == 0)
    {
        return CResult::Make_Error("The package extend length is 0.");
    }
    if (req->hash_md5() != CMD5::Encode(extend))
    {
        return CResult::Make_Error("Hasd check error.");
    }

    auto info = m_mapTokenToUploadInfo.try_get(token);
    auto fileName = Helper::Format("%s%s.%d", info->Path, info->Name, req->splitpackageid());
    CUtility::WriteFile(fileName, extend.c_str(),extend.size());
    info->SplitPackageFile[req->splitpackageid()] = fileName;

    return CResult::Succeed();
}

CResult Sloong::FileManager::UploadedHandler(const string &str_req, CDataTransPackage *trans_pack)
{
    auto req = ConvertStrToObj<UploadedRequest>(str_req);
    auto& token = req->token();
    if (!m_mapTokenToUploadInfo.exist(token))
        return CResult::Make_Error("Need request PrepareUpload firest.");

    auto info = m_mapTokenToUploadInfo.try_get(token);
    auto full_path = info->Path + info->Name;
    auto res = MergeFile(info->SplitPackageFile, full_path);
    if (res.IsFialed())
        return res;

    if (info->Hash_MD5 != CMD5::Encode(full_path, true))
        return CResult::Make_Error("Hasd check error.");

    
    res = MoveFile(full_path, m_strArchiveFolder);
    if (res.IsFialed())
        return res;

    return CResult::Succeed();
}

CResult Sloong::FileManager::PrepareDownloadHandler(const string &str_req, CDataTransPackage *trans_pack)
{
    auto req = ConvertStrToObj<PrepareDownloadRequest>(str_req);

    auto res = QueryFilePath(req->hash_md5());
    if( res.IsFialed())
        return res;

    string real_path = res.GetMessage();
    if (access(real_path.c_str(), ACC_R) != 0)
    {
        return CResult::Make_Error("Cann't access to target file.");
    }

    auto token = CUtility::GenUUID();
    string cacheFolder = Helper::Format("%s%s",m_strCacheFolder.c_str(), token.c_str());
    // TODO. need process the pialed case.
    auto& file_list = m_mapTokenToDownloadInfo[token].SplitPackageFile;
    res = SplitFile(real_path, file_list);
    if (res.IsFialed())
        return res;

    int size = 0;
    res = GetFileSize(real_path, &size);
    if (res.IsFialed())
        return res;

    m_mapTokenToDownloadInfo[token].RealPath = real_path;
    m_mapTokenToDownloadInfo[token].CacheFolder = cacheFolder;
    m_mapTokenToDownloadInfo[token].Hash_MD5 = CMD5::Encode(real_path, true);
    m_mapTokenToDownloadInfo[token].Size = size;

    PrepareDownloadResponse response;
    response.set_token(token);
    response.set_filesize(size);
    auto infoMap = response.mutable_splitpackageinfos();
    for( auto &item : file_list)
    {
        infoMap->operator[](item.first) = CMD5::Encode(item.second,true);
    }
    
    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::FileManager::DownloadingHandler(const string &str_req, CDataTransPackage *trans_pack)
{
    auto req = ConvertStrToObj<DownloadingRequest>(str_req);
    auto& token = req->token();
    if (!m_mapTokenToUploadInfo.exist(token))
        return CResult::Make_Error("Need request PrepareDownload firest.");

    auto info = m_mapTokenToDownloadInfo.try_get(token);
    auto path = info->SplitPackageFile.try_get(req->splitpackageid());
    int size = 0;
    auto buf = CUtility::ReadFile(*path,&size);
    auto data_package = trans_pack->GetDataPackage();
    data_package->set_extend(buf.get(),size);
    DownloadingResponse res;
    res.set_hash_md5(CMD5::Encode(*path,true));
    return CResult::Make_OK(ConvertObjToStr(&res));
}

CResult Sloong::FileManager::DownloadedHandler(const string &str_req, CDataTransPackage *trans_pack)
{
    auto req = ConvertStrToObj<DownloadedRequest>(str_req);
    auto& token = req->token();
    if (m_mapTokenToUploadInfo.exist(token))
    {
        auto info = m_mapTokenToDownloadInfo.try_get(token);
        ClearCache(info->CacheFolder);
        m_mapTokenToDownloadInfo.erase(token);
    }

    return CResult::Succeed();
}

CResult Sloong::FileManager::TestSpeedHandler(const string &str_req, CDataTransPackage *trans_pack)
{
    return CResult::Succeed();
}