#include "filemanager.h"
#include "filecenter.h"
#include "utility.h"
#include "ImageProcesser.h"
#include "IData.h"
using namespace Sloong;

CResult Sloong::FileManager::Initialize(IControl *ic)
{
    IObject::Initialize(ic);

    auto m = ic->Get(FILECENTER_DATAITEM::UploadInfos);
    m_mapTokenToUploadInfo = STATIC_TRANS<map_ex<string, UploadInfo> *>(m);

    auto config = IData::GetModuleConfig();
    if (!(*config)["ArchiveFolder"].empty())
    {
        m_strArchiveFolder = (*config)["ArchiveFolder"].asString();
    }
    if (!(*config)["UploadTempSaveFolder"].empty())
    {
        m_strUploadTempSaveFolder = (*config)["UploadTempSaveFolder"].asString();
    }

    FormatFolderString(m_strArchiveFolder);
    FormatFolderString(m_strUploadTempSaveFolder);

    m_mapFuncToHandler[Functions::PrepareUpload] = std::bind(&FileManager::PrepareUploadHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Uploading] = std::bind(&FileManager::UploadingHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::Uploaded] = std::bind(&FileManager::UploadedHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::SimpleUpload] = std::bind(&FileManager::SimpleUploadHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::DownloadVerify] = std::bind(&FileManager::DownloadVerifyHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::DownloadFile] = std::bind(&FileManager::DownloadFileHandler, this, std::placeholders::_1, std::placeholders::_2);

    m_mapFuncToHandler[Functions::ConvertImageFile] = std::bind(&FileManager::ConvertImageFileHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[Functions::GetThumbnail] = std::bind(&FileManager::GetThumbnailHandler, this, std::placeholders::_1, std::placeholders::_2);

    return CResult::Succeed;
}

PackageResult Sloong::FileManager::RequestPackageProcesser(Package *pack)
{
    auto function = (Functions)pack->function();
    if (!Functions_IsValid(function))
    {
        return PackageResult::Make_OKResult(PackageHelper::MakeErrorResponse(pack, Helper::Format("FileCenter no provide [%d] function.", function)));
    }

    auto req_obj = pack->content();
    auto func_name = Functions_Name(function);
    m_pLog->Debug(Helper::Format("Request [%d][%s]:[%s]", function, func_name.c_str(), req_obj.c_str()));
    if (!m_mapFuncToHandler.exist(function))
    {
        return PackageResult::Make_OKResult(PackageHelper::MakeErrorResponse(pack, Helper::Format("Function [%s] no handler.", func_name.c_str())));
    }

    auto res = m_mapFuncToHandler[function](req_obj, pack);
    auto response = PackageHelper::MakeResponse(pack, res);
    if (res.IsSucceed())
        m_pLog->Debug(Helper::Format("Response [%s]:[%s][%d].", func_name.c_str(), ResultType_Name(res.GetResult()).c_str(), res.GetMessage().length()));
    else
        m_pLog->Debug(Helper::Format("Response [%s]:[%s][%s].", func_name.c_str(), ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str()));
    return PackageResult::Make_OKResult(move(response));
}

PackageResult Sloong::FileManager::ResponsePackageProcesser(Package *pack)
{
    return PackageResult::Ignore();
}

CResult Sloong::FileManager::MergeFile(const list<FileRange> &fileList, const string &saveFile)
{
    ofstream out(saveFile.c_str(), ios::binary);
    for (auto &item : fileList)
    {
        out.seekp(ios_base::beg + item.Start);
        out.write(item.Data.c_str(), item.Data.length());
    }
    out.close();
    return CResult::Succeed;
}

CResult Sloong::FileManager::SplitFile(const string &filepath, int splitSize, map_ex<int, string> &pReadList, int *out_all_size)
{
    if (!FileExist(filepath))
    {
        return CResult::Make_Error("File no exist.");
    }

    ifstream in(filepath.c_str(), ios::in | ios::binary);
    in.seekg(ios_base::end);
    int nSize = in.tellg();
    in.seekg(ios_base::beg);
    for (int i = 0; i < nSize; i += splitSize)
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

CResult Sloong::FileManager::ArchiveFile(const string &index, const string &source)
{
    try
    {
        string target = GetFileTruePath(index);

        m_pLog->Verbos(Helper::Format("Archive file: source[%s] target[%s]", source.c_str(), target.c_str()));
        if (source.length() < 3 || target.length() < 3)
        {
            return CResult::Make_Error(Helper::Format("Move File error. File name cannot empty. source:%s;target:%s", source.c_str(), target.c_str()));
        }

        if (access(source.c_str(), ACC_R) != 0)
        {
            return CResult::Make_Error(Helper::Format("Move File error. Origin file not exist or can not read:[%s]", source.c_str()));
        }

        auto res = Helper::CheckFileDirectory(target);
        if (res < 0)
        {
            return CResult::Make_Error(Helper::Format("Move File error.CheckFileDirectory error:[%s][%d]", target.c_str(), res));
        }

        if (!Helper::MoveFile(source, target))
        {
            // Move file need write access. so if move file error, try copy .
            if (!Helper::RunSystemCmd(Helper::Format("mv \"%s\" \"%s\"", source.c_str(), target.c_str())))
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
int Sloong::FileManager::GetFileSize(const string &source)
{
    ifstream in(source, ios::in | ios::binary);
    in.seekg(0, ios::end);
    int nSize = in.tellg();
    return nSize;
}

// The filecenter no't care the file format. so in here, we saved by the hashcode, the saver should be save the format.
string Sloong::FileManager::GetFileTruePath(const string &index)
{
    return GetFileFolder(index) + index;
}

// The filecenter no't care the file format. so in here, we saved by the hashcode, the saver should be save the format.
string Sloong::FileManager::GetFileFolder(const string &index)
{
    auto path = Helper::Format("%s%s/", m_strArchiveFolder.c_str(), index.substr(0, 3).c_str());
    Helper::CheckFileDirectory(path);
    return path;
}

CResult Sloong::FileManager::PrepareUploadHandler(const string &str_req, Package *trans_pack)
{
    auto req = ConvertStrToObj<PrepareUploadRequest>(str_req);

    auto token = CUtility::GenUUID();
    auto &savedInfo = (*m_mapTokenToUploadInfo)[token];
    savedInfo.SHA256 = req->sha256();
    savedInfo.FileSize = req->filesize();
    savedInfo.Path = m_strUploadTempSaveFolder + token + "/";
    Helper::RunSystemCmd(Helper::Format("mkdir -p %s", savedInfo.Path.c_str()));

    PrepareUploadResponse response;
    response.set_token(token);
    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::FileManager::UploadingHandler(const string &str_req, Package *pack)
{
    auto req = ConvertStrToObj<UploadingRequest>(str_req);

    auto info = m_mapTokenToUploadInfo->try_get(req->token());
    if (info == nullptr)
        return CResult::Make_Error("Need request PrepareUpload firest.");

    auto &data = req->uploaddata();

    if (data.end() - data.start() != data.data().length())
    {
        return CResult::Make_Error(Helper::Format("Length check error.[%d]<->[%d]", data.end() - data.start(), data.data().length()));
    }

    auto sha256 = CSHA256::Encode(data.data());
    if (data.sha256() != sha256)
    {
        return CResult::Make_Error(Helper::Format("Hasd check error.[%s]<->[%s]", sha256.c_str(), data.sha256().c_str()));
    }

    FileRange range;
    range.Start = data.start();
    range.End = data.end();
    range.Data = data.data();

    info->DataList.push_back(move(range));

    return CResult::Succeed;
}

CResult Sloong::FileManager::UploadedHandler(const string &str_req, Package *pack)
{
    auto req = ConvertStrToObj<UploadedRequest>(str_req);
    auto info = m_mapTokenToUploadInfo->try_get(req->token());
    if (info == nullptr)
        return CResult::Make_Error("Need request PrepareUpload firest.");

    auto temp_path = info->Path + info->SHA256;
    Helper::CheckFileDirectory(temp_path);

    auto res = MergeFile(info->DataList, temp_path);
    if (res.IsFialed())
        return CResult::Make_Error(res.GetMessage());

    m_pLog->Verbos(Helper::Format("Save file to [%s]. Hash [%s]", temp_path.c_str(), info->SHA256.c_str()));
    auto sha256 = CUtility::SHA256EncodeFile(temp_path);
    if (info->SHA256 != sha256)
        return CResult::Make_Error(Helper::Format("Hasd check error.[%s]<->[%s]", sha256.c_str(), info->SHA256.c_str()));

    res = ArchiveFile(req->token(), temp_path);
    if (res.IsFialed())
        return res;

    Helper::RunSystemCmd(Helper::Format("rm -d %s", info->Path.c_str()));

    return CResult::Succeed;
}

CResult Sloong::FileManager::SimpleUploadHandler(const string &str_req, Package *trans_pack)
{
    auto req = ConvertStrToObj<SimpleUploadRequest>(str_req);

    if (req->sha256() != CSHA256::Encode(req->data()))
    {
        return CResult::Make_Error("Hasd check error.");
    }
    if (req->filesize() != req->data().length())
    {
        return CResult::Make_Error("Length check error.");
    }

    auto temp_path = m_strUploadTempSaveFolder + "SimpleUpload/" + req->sha256();
    Helper::CheckFileDirectory(temp_path);

    auto res = CUtility::WriteFile(temp_path, req->data().c_str(), req->filesize());
    if (res.IsFialed())
        return res;

    auto token = CUtility::GenUUID();
    res = ArchiveFile(token, temp_path);
    if (res.IsFialed())
        return res;

    Helper::RunSystemCmd(Helper::Format("rm %s", temp_path.c_str()));

    return CResult::Make_OK(token);
}

CResult Sloong::FileManager::DownloadVerifyHandler(const string &str_req, Package *pack)
{ /*
    auto req = ConvertStrToObj<DownloadVerifyHandler>(str_req);

    string real_path = QueryFilePath(req->hashcode());
    if (access(real_path.c_str(), ACC_R) != 0)
    {
        return CResult::Make_Error("Cann't access to target file:" + real_path );
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
    auto res = SplitFile(real_path,packageSize, info.SplitPackage, &filesize);
    if (res.IsFialed())
        return CResult::Make_Error(res.GetMessage());

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

    return CResult::Make_OK(ConvertObjToStr(&response));*/
    return CResult::Succeed;
}

CResult Sloong::FileManager::DownloadFileHandler(const string &str_req, Package *pack)
{
    auto req = ConvertStrToObj<DownloadFileRequest>(str_req);

    string real_path = GetFileTruePath(req->index());
    if (access(real_path.c_str(), ACC_R) != 0)
    {
        return CResult::Make_Error("Cann't access to target file:" + real_path);
    }

    auto data = req->filedata();

    auto nSize = GetFileSize(real_path);
    ifstream in(real_path, ios::in | ios::binary);

    DownloadFileResponse response;
    response.set_filesize(nSize);

    for (auto item : data)
    {
        auto readSize = min(item.end(), nSize) - item.start();

        string str;
        str.resize(readSize);
        in.read(str.data(), readSize);
        if (!in)
        {
            int canRead = in.gcount();
            in.close();
            return CResult::Make_Error(Helper::Format("Error when read file:error: only %d could be read", canRead));
        }

        auto d = response.add_filedata();
        d->set_start(item.start());
        d->set_end(item.end());
        d->set_sha256(CSHA256::Encode(str));
        d->set_data(move(str));
    }
    in.close();

    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::FileManager::ConvertImageFileHandler(const string &str_req, Package *trans_pack)
{
    auto req = ConvertStrToObj<ConvertImageFileRequest>(str_req);
    if (req->targetformat() == SupportFormat::FLIF || !SupportFormat_IsValid(req->targetformat()))
    {
        return CResult::Make_Error(Helper::Format("Unsupport format[%s].", SupportFormat_Name(req->targetformat()).c_str()));
    }

    string real_path = GetFileTruePath(req->index());
    auto temp_path = m_strUploadTempSaveFolder + "FormatConvertTemp/" + req->index();

    ConvertImageFileResponse response;

    list<SupportFormat> fms;
    if (req->targetformat() == SupportFormat::Best)
    {
        fms.push_back(SupportFormat::WEBP);
        fms.push_back(SupportFormat::AVIF);
        fms.push_back(SupportFormat::HEIF);
    }
    else
    {
        fms.push_back(req->targetformat());
    }

    map<SupportFormat, string> f_path;
    string new_file_path;
    for (auto f : fms)
    {
        auto res = ImageProcesser::ConvertFormat(real_path, temp_path, f, req->quality());
        if (res.IsFialed())
            return res;

        auto info = response.add_extendinfos();
        auto temp_path = res.GetMessage();
        f_path[f] = temp_path;
        info->set_size(GetFileSize(temp_path));
        info->set_sha256(CSHA256::Encode(temp_path, true));
        info->set_md5(CMD5::Encode(temp_path, true));
    }

    auto best_fmt = SupportFormat::Best;
    auto best_size = ((unsigned int)-1) >> 1;
    for (auto f : response.extendinfos())
    {
        if (f.size() < best_size)
        {
            best_size = f.size();
            best_fmt = f.format();
        }
    }

    auto infos = response.mutable_extendinfos();
    for( auto i = infos->begin(); i != infos->end(); ++i)
    {
        auto f = *i;
        if(f.format() == best_fmt )
        {
            new_file_path = f_path[f.format()];
            response.set_allocated_newfileinfo(&f);
            infos->erase(i);
        }else {
            remove(f_path[f.format()].c_str());
        }
    }

    auto uuid = CUtility::GenUUID();
    auto res = ArchiveFile(uuid, new_file_path);
    if (res.IsFialed())
        return res;

    if (!req->retainsourcefile())
    {
        m_pLog->Info(Helper::Format("%s is convert to %s with %s format.delete old file.", req->index().c_str(), uuid.c_str(), SupportFormat_Name(req->targetformat()).c_str()));
        int r = remove(real_path.c_str());
        if (r != 0)
        {
            m_pLog->Warn(Helper::Format("old file delete fialed. return code %d", r));
        }
    }

    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::FileManager::GetThumbnailHandler(const string &str_req, Package *trans_pack)
{
    auto req = ConvertStrToObj<GetThumbnailRequest>(str_req);
    string thumb_file = Helper::Format("%s_%d_%d_%d.webp", Helper::ntos(req->index()).c_str(), req->width(), req->height(), req->quality());
    if (!FileExist(thumb_file))
    {
        auto real_path = GetFileTruePath(req->index());
        auto res = ImageProcesser::GetThumbnail(real_path, thumb_file, req->width(), req->height(), req->quality());
        if (res.IsFialed())
            return CResult::Make_Error(res.GetMessage());
    }

    string data;
    auto size = ReadFile(thumb_file, data);
    GetThumbnailResponse response;
    response.set_filesize(size);
    response.set_data(move(data));

    return CResult::Make_OK(ConvertObjToStr(&response));
}
