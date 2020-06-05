#include "globalfunction.h"
// sys
#include <sys/socket.h>
#include <netinet/in.h>
// univ

#include "utility.h"
#include "version.h"
#include "epollex.h"
#include "NormalEvent.hpp"
#include "IData.h"

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
};

void Sloong::CGlobalFunction::Initialize(IControl *ic)
{
    IObject::Initialize(ic);
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
    // TODO: change to send message mode.
    //CConfiguation *pConfig = TYPE_TRANS<CConfiguation *>(g_pThis->m_iC->Get(DATA_ITEM::Configuation));
    string value("NO SUPPORT");
    /*try
    {
        value = "NO SUPPORT";
        //value = pConfig->GetStringConfig("config",section, key, def);
    }
    catch (normal_except &e)
    {
        CLua::PushString(l, "");
        CLua::PushString(l, e.what());
        return 2;
    }*/

    CLua::PushString(l, value);
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
    auto file = CLua::GetString(l,1,"");
    auto uuid = "";
    int size = 0;
    auto pBuf = CUtility::ReadFile(file, &size);
    if( size > 0 )
    {
        auto uuid = CUtility::GenUUID();
        Instance->m_iC->AddTempBytes(uuid, pBuf, size);
    }
    CLua::PushString(l, uuid);
    return 1;
}
