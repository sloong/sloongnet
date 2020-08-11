#include "filemanager.h"
#include "filecenter.h"
#include "utility.h"
using namespace Sloong;

CResult Sloong::FileManager::Initialize(IControl *ic)
{
    IObject::Initialize(ic);

    auto m = ic->Get(FILECENTER_DATAITEM::UploadInfos);
    m_mapTokenToUploadInfo = STATIC_TRANS<map_ex<string, UploadInfo>*>(m);
    m = ic->Get(FILECENTER_DATAITEM::DownloadInfos);
    m_mapTokenToDownloadInfo = STATIC_TRANS<map_ex<string, DownloadInfo>*>(m);

    m_mapFuncToHandler[Functions::TestSpeed] = std::bind(&FileManager::TestSpeedHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::PrepareUpload] = std::bind(&FileManager::PrepareUploadHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Uploading] = std::bind(&FileManager::UploadingHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Uploaded] = std::bind(&FileManager::UploadedHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::PrepareDownload] = std::bind(&FileManager::PrepareDownloadHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Downloading] = std::bind(&FileManager::DownloadingHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Downloaded] = std::bind(&FileManager::DownloadedHandler, this, std::placeholders::_1, std::placeholders::_2);
    return CResult::Succeed;
}

PackageResult Sloong::FileManager::RequestPackageProcesser(DataPackage *pack)
{
    auto function = (Functions)pack->function();
    if (!Functions_IsValid(function))
    {
        return PackageResult::Make_OKResult(Package::MakeErrorResponse(pack, Helper::Format("FileCenter no provide [%d] function.", function)));
    }

    auto req_obj = pack->content();
    auto func_name = Functions_Name(function);
    m_pLog->Debug(Helper::Format("Request [%d][%s]:[%s]", function, func_name.c_str(), req_obj.c_str()));
    if (!m_mapFuncToHandler.exist(function))
    {
        return PackageResult::Make_OKResult(Package::MakeErrorResponse(pack, Helper::Format("Function [%s] no handler.", func_name.c_str())));
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
    return PackageResult::Make_OKResult(move(response));
}

CResult Sloong::FileManager::MergeFile(const map_ex<int, string> &fileList, const string &saveFile)
{
    ofstream out(saveFile.c_str(), ios::binary );
    for (auto &item : fileList)
    {
        out.write(item.second.c_str(),item.second.length());
    }
    out.close();
    return CResult::Succeed;
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
	return CResult::Succeed;
}

CResult Sloong::FileManager::ArchiveFile(UploadInfo* info)
{
    try
    {
        string target = GetPathByHashcode(info->Hash_MD5);
        string source = info->Path;
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
            if (!CUniversal::RunSystemCmd(Helper::Format("mv \"%s\" \"%s\"", source.c_str(), target.c_str())))
            {
                return CResult::Make_Error("Move File and try copy file error.");
            }
            return CResult::Succeed;
        }
    }
    catch (const exception &e)
    {
        m_pLog->Error(e.what());
        return CResult::Make_Error(e.what());
    }

    return CResult::Succeed;
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

// The filecenter no't care the file format. so in here, we saved by the hashcode, the saver should be save the format.
string Sloong::FileManager::GetPathByHashcode( const string& hashcode )
{
    auto path = Helper::Format("%s/%s/%s", m_strArchiveFolder.c_str(), hashcode.substr(0,2).c_str(), hashcode.c_str() );
    CUniversal::CheckFileDirectory(path);
    return path;
}

// In Design, The other module no need know the file true path. 
// So have two way:
//  1> Use the database to save the file treu path and the file hash code.
//     But this will make the filecenter to be complex. 
//  2> Use the hashcode to comput the path, because when save file, the path is build with the hashcode 
//   so we can use the hashcode to rebuild the save path.
//     But this will fix the file tree structure for storage.
CResult Sloong::FileManager::QueryFilePath(const string &hash_md5)
{
    return CResult::Make_OK(GetPathByHashcode(hash_md5));
}

SResult Sloong::FileManager::PrepareUploadHandler(const string &str_req, DataPackage *trans_pack)
{
    auto req = ConvertStrToObj<PrepareUploadRequest>(str_req);

    auto token = CUtility::GenUUID();
    auto& savedInfo = (*m_mapTokenToUploadInfo)[token];
    savedInfo.Hash_MD5 = req->hash_md5();
    savedInfo.Path = FormatFolderString(m_strUploadTempSaveFolder) + token + "/";
    CUniversal::RunSystemCmd(Helper::Format("mkdir -p %s", savedInfo.Path.c_str()));

    PrepareUploadResponse res;
    res.set_token(token);
    return SResult::Make_OK(ConvertObjToStr(&res));
}

SResult Sloong::FileManager::UploadingHandler(const string &str_req, DataPackage *pack)
{
    auto req = ConvertStrToObj<UploadingRequest>(str_req);
    auto &token = req->token();
    auto info = m_mapTokenToUploadInfo->try_get(token);
    if (info == nullptr)
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

    info->SplitPackage[req->splitpackageid()] = extend;

    return SResult::Succeed();
}

SResult Sloong::FileManager::UploadedHandler(const string &str_req, DataPackage *pack)
{
    auto req = ConvertStrToObj<UploadedRequest>(str_req);
    auto &token = req->token();
    auto info = m_mapTokenToUploadInfo->try_get(token);
    if (info == nullptr)
        return SResult::Make_Error("Need request PrepareUpload firest.");

    auto temp_path = info->Path + info->Hash_MD5;
    CUniversal::CheckFileDirectory(temp_path);
    
    auto res = MergeFile(info->SplitPackage, temp_path);
    if (res.IsFialed())
        return SResult::Make_Error(res.GetMessage());

    m_pLog->Verbos(Helper::Format("Save file to [%s]. Hash [%s]", temp_path.c_str(), info->Hash_MD5.c_str()));

    if (info->Hash_MD5 != CMD5::Encode(temp_path, true))
        return SResult::Make_Error("Hasd check error.");

    res = ArchiveFile(info);
    if (res.IsFialed())
        return SResult::Make_Error(res.GetMessage());

    CUniversal::RunSystemCmd(Helper::Format("rm -d %s", info->Path.c_str()));

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
        return SResult::Make_Error("Cann't access to target file:" + real_path );
    }

    // 怎样来避免文件内容被拷贝的同时，满足拆分读取的需求？
    // 这里准备使用list的方式，读取时指定一个大小，将文件内容读取到一个string的list中
    // 每个string都是指定的长度。然后外界根据索引来读取指定的string
    // 这个list使用shared_ptr来进行包装，然后存储在IC中，这样使用者也不需要带着这个列表到处传递
    auto packageSize = req->splitpackagesize();
    int filesize = 0;
    auto token = CUtility::GenUUID();
    // TODO. need process the pialed case.
    auto &info = (*m_mapTokenToDownloadInfo)[token];
    res = SplitFile(real_path,packageSize, info.SplitPackage, &filesize);
    if (res.IsFialed())
        return SResult::Make_Error(res.GetMessage());

    info.RealPath = real_path;
    info.Hash_MD5 = CMD5::Encode(real_path, true);
    info.Size = filesize;

    PrepareDownloadResponse response;
    response.set_token(token);
    response.set_filesize(filesize);
    auto infoMap = response.mutable_splitpackageinfos();
    for (auto &item : info.SplitPackage)
    {
        infoMap->operator[](item.first) = CMD5::Encode(item.second);
    }

    return SResult::Make_OK(ConvertObjToStr(&response));
}

SResult Sloong::FileManager::DownloadingHandler(const string &str_req, DataPackage *pack)
{
    auto req = ConvertStrToObj<DownloadingRequest>(str_req);
    auto &token = req->token();
    auto id = req->splitpackageid();
    auto info = m_mapTokenToDownloadInfo->try_get(token);
    if (info == nullptr)
        return SResult::Make_Error("Need request PrepareDownload firest.");

    auto data = info->SplitPackage.try_get(id);
    if( data == nullptr )
        return SResult::Make_Error(Helper::Format("No find the data with package id[%d], the package nums[%d]. please check.", id, info->SplitPackage.size()));

    DownloadingResponse res;
    res.set_hash_md5(CMD5::Encode(*data));

    // TODO: Here may the copy once, the pr
    return SResult::Make_OKResult(ConvertObjToStr(&res), *data);
}

SResult Sloong::FileManager::DownloadedHandler(const string &str_req, DataPackage *trans_pack)
{
    auto req = ConvertStrToObj<DownloadedRequest>(str_req);
    auto &token = req->token();
     auto info = m_mapTokenToDownloadInfo->try_get(token);
    if (info == nullptr)
        return SResult::Make_Error("Need request PrepareDownload firest.");

    m_mapTokenToDownloadInfo->erase(token);
    
    return SResult::Succeed();
}

SResult Sloong::FileManager::TestSpeedHandler(const string &str_req, DataPackage *trans_pack)
{
    return SResult::Succeed();
}