#ifndef CGLOBALFUNCTION_H
#define CGLOBALFUNCTION_H


#include "core.h"
#include "IObject.h"
#include "lua.h"
#include "EasyConnect.h"
namespace Sloong
{

    enum HashType
    {
        MD5 = 0,
        SHA_1 = 1,
        SHA_256 = 2,
        SHA_512 = 3,
    };

    class CGlobalFunction : public IObject
    {
    public:
        CResult Initialize(IControl *iMsg);
        void RegistFuncToLua(CLua *pLua);

    public:
        static int Lua_ShowLog(lua_State *l);
        static int Lua_GetEngineVer(lua_State *l);
        static int Lua_Base64_Encode(lua_State *l);
        static int Lua_Base64_Decode(lua_State *l);
        static int Lua_Hash_Encode(lua_State *l);
        static int Lua_ReloadScript(lua_State *l);
        static int Lua_GetConfig(lua_State *l);
        static int Lua_GenUUID(lua_State *l);
        static int Lua_SetCommData(lua_State *l);
        static int Lua_GetCommData(lua_State *l);
        static int Lua_GetLogObject(lua_State *l);
        static int Lua_SetExtendData(lua_State *l);
        static int Lua_SetExtendDataByFile(lua_State *l);
        static int Lua_ConnectToDBCenter(lua_State *l);
        static int Lua_SQLQueryToDBCenter(lua_State *l);

    protected:
        CResult OnQueryDBCenterTemplateResponse(IEvent *event, CDataTransPackage *pack);
        
    protected:
        map_ex<string,string> m_mapCommData;
        map_ex<string,int> m_mapDBNameToSessionID;
        Json::Value* m_pModuleConfig = nullptr;
        map_ex<int64_t, EasySync*> m_mapIDToSync;
        unique_ptr<EasyConnect> m_SocketDBCenter=nullptr;

    public:
        static unique_ptr<CGlobalFunction> Instance;
    };
} // namespace Sloong
#endif // !CGLOBALFUNCTION_H
