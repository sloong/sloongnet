/*** 
 * @Author: Chuanbin Wang
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-29 17:37:01
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/middleLayer/lua/globalfunction.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
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
        static int Lua_SQLInsertToDBCenter(lua_State *l);
        static int Lua_SQLDeleteToDBCenter(lua_State *l);
        static int Lua_SQLUpdateToDBCenter(lua_State *l);

    protected:
        void OnStart(SharedEvent);
        void ReferenceDataCenterConnection();
        void OnQueryDBCenterTemplateResponse(IEvent *event, DataPackage *pack);
        static CResult RunSQLFunction( const string& req, int func );
        static bool SQLFunctionPrepareCheck( lua_State* l, int sessionid, const string& sql );
        
    protected:
        map_ex<string,string> m_mapCommData;
        map_ex<string,int> m_mapDBNameToSessionID;
        Json::Value* m_pModuleConfig = nullptr;
        int64_t m_SocketDBCenter=0;

    public:
        static unique_ptr<CGlobalFunction> Instance;
    };
} // namespace Sloong
#endif // !CGLOBALFUNCTION_H
