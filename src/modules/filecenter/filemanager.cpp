#include "filemanager.h"
#include "utility.h"
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

PackageResult Sloong::FileManager::RequestPackageProcesser(DataPackage *pack)
{
    auto function = (Functions)pack->function();
    if (!Functions_IsValid(function))
    {
        return PackageResult::Make_OK(Package::MakeErrorResponse(pack, Helper::Format("FileCenter no provide [%d] function.", function)));
    }

    auto req_obj = pack->content();
    auto func_name = Functions_Name(function);
    m_pLog->Debug(Helper::Format("Request [%d][%s]:[%s]", function, func_name.c_str(), req_obj.c_str()));
    if (!m_mapFuncToHandler.exist(function))
    {
        return PackageResult::Make_OK(Package::MakeErrorResponse(pack, Helper::Format("Function [%s] no handler.", func_name.c_str())));
    }

    auto res = m_mapFuncToHandler[function](req_obj, pack);
    auto response = Package::MakeResponse(pack, res);
    if (res.HaveResultObject())
    {
        auto extend = res.GetResultObject();
        response->set_extend(extend);
        m_pLog->Debug(Helper::Format("Response [%s]:[%s][%s].[%d]", func_name.c_str(), ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str(), extend.size()));
    }
    else
        m_pLog->Debug(Helper::Format("Response [%s]:[%s][%s].", func_name.c_str(), ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str()));
    return PackageResult::Make_OK(move(response));
}

CResult Sloong::FileManager::MergeFile(const map_ex<int, string> &fileList, const string &saveFile)
{
    string files;
    for (auto &item : fileList)
        files += item.second + " ";
    auto res = CUniversal::RunSystemCmdAndGetResult(Helper::Format("cat %s > %s", files.c_str(), saveFile.c_str()));
    return CResult::Succeed();
}

CResult Sloong::FileManager::SplitFile(const string &filepath, int splitSize, map_ex<int, string> &pReadList, int* out_all_size)
{
    if (-1 == access(filepath.c_str(), R_OK))
	{
		return CResult::Make_Error("Cannot access file.");
	}

	ifstream in(filepath.c_str(), ios::in | ios::binary);
	streampos pos = in.tellg();
	in.seekg(0, ios::end);
	int nSize = in.tellg();
	in.seekg(pos);
	for( int i = 0; i<nSize; i+= splitSize)
	{
		string str;
		str.resize(splitSize);
		in.read(str.data(), splitSize);
		pReadList[i] = move(str);
	}
	in.close();
	*out_all_size = nSize;
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

SResult Sloong::FileManager::PrepareUploadHandler(const string &str_req, DataPackage *trans_pack)
{
    auto req = ConvertStrToObj<PrepareUploadRequest>(str_req);

    auto token = CUtility::GenUUID();
    auto info = req->info();
    m_mapTokenToUploadInfo[token].Name = info.name();
    m_mapTokenToUploadInfo[token].Hash_MD5 = info.hash_md5();
    m_mapTokenToUploadInfo[token].Path = FormatFolderString(m_strUploadTempSaveFolder) + token + "/";

    PrepareUploadResponse res;
    res.set_token(token);
    return SResult::Make_OK(ConvertObjToStr(&res));
}

SResult Sloong::FileManager::UploadingHandler(const string &str_req, DataPackage *pack)
{
    auto req = ConvertStrToObj<UploadingRequest>(str_req);
    auto &token = req->token();
    if (!m_mapTokenToUploadInfo.exist(token))
        return SResult::Make_Error("Need request PrepareUpload firest.");

    auto extend = pack->extend();
    if (extend.length() == 0)
    {
        return SResult::Make_Error("The package extend length is 0.");
    }
    if (req->hash_md5() != CMD5::Encode(extend))
    {
        return SResult::Make_Error("Hasd check error.");
    }

    auto info = m_mapTokenToUploadInfo.try_get(token);
    info->SplitPackage[req->splitpackageid()] = extend;

    return SResult::Succeed();
}

SResult Sloong::FileManager::UploadedHandler(const string &str_req, DataPackage *pack)
{
    auto req = ConvertStrToObj<UploadedRequest>(str_req);
    auto &token = req->token();
    if (!m_mapTokenToUploadInfo.exist(token))
        return SResult::Make_Error("Need request PrepareUpload firest.");

    auto info = m_mapTokenToUploadInfo.try_get(token);
    auto full_path = info->Path + info->Name;
    auto res = MergeFile(info->SplitPackage, full_path);
    if (res.IsFialed())
        return SResult::Make_Error(res.GetMessage());

    if (info->Hash_MD5 != CMD5::Encode(full_path, true))
        return SResult::Make_Error("Hasd check error.");

    res = MoveFile(full_path, m_strArchiveFolder);
    if (res.IsFialed())
        return SResult::Make_Error(res.GetMessage());

    return SResult::Succeed();
}

SResult Sloong::FileManager::PrepareDownloadHandler(const string &str_req, DataPackage *pack)
{
    auto req = ConvertStrToObj<PrepareDownloadRequest>(str_req);

    auto res = QueryFilePath(req->hash_md5());
    if (res.IsFialed())
        return SResult::Make_Error(res.GetMessage());

    string real_path = res.GetMessage();
    if (access(real_path.c_str(), ACC_R) != 0)
    {
        return SResult::Make_Error("Cann't access to target file.");
    }

    // 怎样来避免文件内容被拷贝的同时，满足拆分读取的需求？
    // 这里准备使用list的方式，读取时指定一个大小，将文件内容读取到一个string的list中
    // 每个string都是指定的长度。然后外界根据索引来读取指定的string
    // 这个list使用shared_ptr来进行包装，然后存储在IC中，这样使用者也不需要带着这个列表到处传递
    auto packageSize = req->splitpackagesize();
    int filesize = 0;
    auto token = CUtility::GenUUID();
    // TODO. need process the pialed case.
    auto &file_list = m_mapTokenToDownloadInfo[token].SplitPackage;
    res = SplitFile(real_path,packageSize, file_list, &filesize);
    if (res.IsFialed())
        return SResult::Make_Error(res.GetMessage());

    m_mapTokenToDownloadInfo[token].RealPath = real_path;
    m_mapTokenToDownloadInfo[token].Hash_MD5 = CMD5::Encode(real_path, true);
    m_mapTokenToDownloadInfo[token].Size = filesize;

    PrepareDownloadResponse response;
    response.set_token(token);
    response.set_filesize(filesize);
    auto infoMap = response.mutable_splitpackageinfos();
    for (auto &item : file_list)
    {
        infoMap->operator[](item.first) = CMD5::Encode(item.second, true);
    }

    return SResult::Make_OK(ConvertObjToStr(&response));
}

SResult Sloong::FileManager::DownloadingHandler(const string &str_req, DataPackage *pack)
{
    auto req = ConvertStrToObj<DownloadingRequest>(str_req);
    auto &token = req->token();
    if (!m_mapTokenToUploadInfo.exist(token))
        return SResult::Make_Error("Need request PrepareDownload firest.");

    auto info = m_mapTokenToDownloadInfo.try_get(token);
    auto path = info->SplitPackage.try_get(req->splitpackageid());
    

    // 这里的extend将会从前面读取的列表中获取
    string extend = "";

    DownloadingResponse res;
    res.set_hash_md5(CMD5::Encode(*path));

    return SResult::Make_OK(ConvertObjToStr(&res), extend);
}

SResult Sloong::FileManager::DownloadedHandler(const string &str_req, DataPackage *trans_pack)
{
    auto req = ConvertStrToObj<DownloadedRequest>(str_req);
    auto &token = req->token();
    if (m_mapTokenToUploadInfo.exist(token))
    {
        //auto info = m_mapTokenToDownloadInfo.try_get(token);
        m_mapTokenToDownloadInfo.erase(token);
    }

    return SResult::Succeed();
}

SResult Sloong::FileManager::TestSpeedHandler(const string &str_req, DataPackage *trans_pack)
{
    return SResult::Succeed();
}