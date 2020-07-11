#include "globalfunction.h"

#include "utility.h"
#include "version.h"
#include "epollex.h"
#include "DataTransPackage.h"
#include "SendPackageEvent.hpp"
#include "IData.h"

#include "snowflake.h"

#include "protocol/datacenter.pb.h"
#include "protocol/manager.pb.h"

using namespace std;

#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;

unique_ptr<CGlobalFunction> Sloong::CGlobalFunction::Instance = make_unique<CGlobalFunction>();

LuaFunctionRegistr g_LuaFunc[] =
    {
        {"ShowLog", CGlobalFunction::Lua_ShowLog},
        {"GetEngineVer", CGlobalFunction::Lua_GetEngineVer},
        {"Base64Encode", CGlobalFunction::Lua_Base64_Encode},
        {"Base64Decode", CGlobalFunction::Lua_Base64_Decode},
        {"HashEncode", CGlobalFunction::Lua_Hash_Encode},
        {"ReloadScript", CGlobalFunction::Lua_ReloadScript},
        {"GetConfig", CGlobalFunction::Lua_GetConfig},
        {"GenUUID", CGlobalFunction::Lua_GenUUID},
        {"SetCommData", CGlobalFunction::Lua_SetCommData},
        {"GetCommData", CGlobalFunction::Lua_GetCommData},
        {"SetExtendData", CGlobalFunction::Lua_SetExtendData},
        {"SetExtendDataByFile", CGlobalFunction::Lua_SetExtendDataByFile},
        {"SendReqeustToDBCenter", CGlobalFunction::Lua_SendRequestToDBCenter},
};

CResult Sloong::CGlobalFunction::Initialize(IControl *ic)
{
    IObject::Initialize(ic);
    IData::Initialize(ic);
    m_pModuleConfig = IData::GetModuleConfig();

    int manager_socket = IData::GetManagerSocket();
    if( manager_socket == INVALID_SOCKET )
        return CResult::Make_Error("Get manager socket error.");

    Manager::QueryTemplateRequest request;
    request.add_templatetype(Core::MODULE_TYPE::DataCenter);
    auto package_id = snowflake::Instance->nextid();
    auto req = make_unique<CSendPackageEvent>();
    req->SetCallbackFunc(std::bind(&CGlobalFunction::OnQueryDBCenterResponse, CGlobalFunction::Instance.get(), std::placeholders::_1, std::placeholders::_2));
    req->SetRequest(manager_socket, IData::GetRuntimeData()->nodeuuid(), package_id, Base::HEIGHT_LEVEL, Manager::Functions::QueryTemplate , ConvertObjToStr(&request), "", DataPackage_PackageType::DataPackage_PackageType_RequestPackage);
    CGlobalFunction::Instance->m_iC->SendMessage(std::move(req));
    return CResult::Succeed();
}

void Sloong::CGlobalFunction::RegistFuncToLua(CLua *pLua)
{
    vector<LuaFunctionRegistr> funcList(g_LuaFunc, g_LuaFunc + ARRAYSIZE(g_LuaFunc));
    pLua->AddFunctions(&funcList);
}

int Sloong::CGlobalFunction::Lua_GetEngineVer(lua_State *l)
{
    CLua::PushString(l, VERSION_TEXT);
    return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Encode(lua_State *l)
{
    string req = CLua::GetString(l, 1, "");
    string res = CBase64::Encode(req);
    CLua::PushString(l, res);
    return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Decode(lua_State *l)
{
    string req = CLua::GetString(l, 1, "");
    string res = CBase64::Decode(req);
    CLua::PushString(l, res);
    return 1;
}

/**
 * @Remarks: 
 * @Params: 1 string > string data. if file mode, is the file path
 * 			2 string > hash type. support : 0(MD5), 1(SHA-1),2(SHA-256), 3(SHA-512). default by SHA-1
 * 			3 boolen > file mode. default by false
 * @Return: Hash result
 */
int Sloong::CGlobalFunction::Lua_Hash_Encode(lua_State *l)
{
    string data = CLua::GetString(l, 1, "");
    int hash_type = CLua::GetInteger(l, 2, 1);
    bool file_mode = CLua::GetBoolen(l, 3);
    string result("");

    switch (hash_type)
    {
    case HashType::MD5:
        result = CMD5::Encode(data, file_mode);
        break;
    case HashType::SHA_1:
        result = CSHA1::Encode(data, file_mode);
        break;
    case HashType::SHA_256:
        result = CSHA256::Encode(data, file_mode);
        break;
    case HashType::SHA_512:
        result = CSHA512::Encode(data, file_mode);
        break;
    default:
        result = "Hash type error. support : 0(MD5), 1(SHA-1),2(SHA-256), 3(SHA-512).";
        break;
    }
    CLua::PushString(l, result);
    return 1;
}

int Sloong::CGlobalFunction::Lua_ReloadScript(lua_State *l)
{
    CGlobalFunction::Instance->m_iC->SendMessage(EVENT_TYPE::ReloadLuaContext);
    return 0;
}

int Sloong::CGlobalFunction::Lua_GetConfig(lua_State *l)
{
    string section = CLua::GetString(l, 1);
    string key = CLua::GetString(l, 2);
    string def = CLua::GetString(l, 3);
    auto config = CGlobalFunction::Instance->m_pModuleConfig;
    if (config->isMember(section) && (*config)[section].isMember(key))
    {
        CLua::PushString(l, (*config)[section][key].asString());
    }
    else
    {
        CLua::PushString(l, def);
    }
    return 1;
}

int CGlobalFunction::Lua_GenUUID(lua_State *l)
{
    CLua::PushString(l, CUtility::GenUUID());
    return 1;
}

int CGlobalFunction::Lua_ShowLog(lua_State *l)
{
    string luaTitle = CLua::GetString(l, 2, "Info");
    string msg = CLua::GetString(l, 1);
    if (msg == "")
        return 0;
    else
        msg = Helper::Format("[Script]:[%s]", msg.c_str());

    auto log = CGlobalFunction::Instance->m_pLog;
    if (luaTitle == "Info")
        log->Info(msg);
    else if (luaTitle == "Warn")
        log->Warn(msg);
    else if (luaTitle == "Debug")
        log->Debug(msg);
    else if (luaTitle == "Error")
        log->Error(msg);
    else if (luaTitle == "Verbos")
        log->Verbos(msg);
    else if (luaTitle == "Assert")
        log->Assert(msg);
    else if (luaTitle == "Fatal")
        log->Fatal(msg);
    else
        log->Log(msg, luaTitle);
    return 0;
}

int CGlobalFunction::Lua_SetCommData(lua_State *l)
{
    auto key = CLua::GetString(l, 1, "");
    if (key.length() > 0)
    {
        Instance->m_mapCommData[key] = CLua::GetString(l, 2, "");
    }
    return 0;
}

int CGlobalFunction::Lua_GetCommData(lua_State *l)
{
    auto key = CLua::GetString(l, 1, "");
    if (key.length() > 0 && Instance->m_mapCommData.exist(key))
        CLua::PushString(l, Instance->m_mapCommData[key]);
    else
        CLua::PushString(l, "");
    return 1;
}

int CGlobalFunction::Lua_GetLogObject(lua_State *l)
{
    CLua::PushPointer(l, CGlobalFunction::Instance->m_pLog);
    return 1;
}

int CGlobalFunction::Lua_SetExtendData(lua_State *l)
{
    auto data = CLua::GetString(l, 1, "");
    auto uuid = "";
    if (data.length() > 0)
    {
        auto uuid = CUtility::GenUUID();
        Instance->m_iC->AddTempString(uuid, data);
    }

    CLua::PushString(l, uuid);
    return 1;
}

int CGlobalFunction::Lua_SetExtendDataByFile(lua_State *l)
{
    auto file = CLua::GetString(l, 1, "");
    auto uuid = "";
    int size = 0;
    auto pBuf = CUtility::ReadFile(file, &size);
    if (size > 0)
    {
        auto uuid = CUtility::GenUUID();
        Instance->m_iC->AddTempBytes(uuid, pBuf, size);
    }
    CLua::PushInteger(l, size);
    CLua::PushString(l, uuid);
    return 2;
}

int CGlobalFunction::Lua_SendRequestToDBCenter(lua_State *l)
{
    auto func = CLua::GetInteger(l, 1, DataCenter::Functions::Invalid);
    auto request_str = CLua::GetString(l, 2, "");
    if (func == DataCenter::Functions::Invalid || !DataCenter::Functions_IsValid(func))
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "Function is invalid");
        return 2;
    }

    if (request_str.empty())
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "request data is empty");
        return 2;
    }

    int connect_socket = 0;
    auto package_id = snowflake::Instance->nextid();
    auto req = make_unique<CSendPackageEvent>();
    req->SetCallbackFunc(std::bind(&CGlobalFunction::OnSendPackageResponse, CGlobalFunction::Instance.get(), std::placeholders::_1, std::placeholders::_2));
    req->SetRequest(connect_socket, IData::GetRuntimeData()->nodeuuid(), package_id, Base::HEIGHT_LEVEL, func, request_str, "", DataPackage_PackageType::DataPackage_PackageType_RequestPackage);
    CGlobalFunction::Instance->m_iC->SendMessage(std::move(req));

    CEasySync sync;
    CGlobalFunction::Instance->m_mapIDToSync[package_id] = &sync;
    if (!sync.wait_for(5000))
    {
        CLua::PushInteger(l, Base::ResultType::Error);
        CLua::PushString(l, "timtout");
        return 2;
    }

    auto response_str = CGlobalFunction::Instance->m_iC->GetTempString(Helper::ntos(package_id));
    CLua::PushInteger(l, Base::ResultType::Succeed);
    CLua::PushString(l, response_str);
    return 2;
}
CResult CGlobalFunction::OnQueryDBCenterResponse(IEvent *event, CDataTransPackage *pack)
{
    return CResult::Succeed();
}
CResult CGlobalFunction::OnSendPackageResponse(IEvent *event, CDataTransPackage *pack)
{
    auto id = pack->GetSerialNumber();

    m_iC->AddTempString(Helper::ntos(id), pack->GetRecvMessage());
    auto sync = m_mapIDToSync.try_get(id);
    m_mapIDToSync.erase(id);
    (*sync)->notify_one();
    return CResult::Succeed();
}