/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-12-11 15:05:40
 * @LastEditTime: 2020-08-12 15:50:41
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/middleLayer/lua/globalfunction.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description:
 */
/***
  * @......................................&&.........................
  * @....................................&&&..........................
  * @.................................&&&&............................
  * @...............................&&&&..............................
  * @.............................&&&&&&..............................
  * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
  * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
  * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
  * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
  * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
  * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
  * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
  * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
  * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
  * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
  * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
  * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
  * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
  * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
  * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
  * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
  * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
  * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
  * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
  * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
  * @.....&&&&&&&&&&&&&&&&............................&&..............
  * @....&&&&&&&&&&&&&&&.................&&...........................
  * @...&&&&&&&&&&&&&&&.....................&&&&......................
  * @...&&&&&&&&&&.&&&........................&&&&&...................
  * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
  * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
  * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
  * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
  * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
  * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
  * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
  * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
  * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
  * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
  * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
  * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
  * @........&&...................&&&&&&.........................&&&..
  * @.........&.....................&&&&........................&&....
  * @...............................&&&.......................&&......
  * @................................&&......................&&.......
  * @.................................&&..............................
  * @..................................&..............................
  */

#pragma once

#include "core.h"
#include "IObject.h"
#include "lua_ex.h"
#include "EasyConnect.h"

#include "protocol/manager.pb.h"
using namespace Manager;
namespace Sloong
{

    enum HashType
    {
        MD5 = 0,
        SHA_1 = 1,
        SHA_256 = 2,
        SHA_512 = 3,
    };

    enum RecvStatus
    {
        Wait = 0,
        Receiving = 1,
        Saveing = 2,
        Done = 3,
        VerificationError = 4,
        OtherError = 5,
    };
    struct RecvDataPackage
    {
        string strMD5 = "";
        RecvStatus emStatus = RecvStatus::Wait;
        string strName = "";
        string strPath = "";
    };

    class CGlobalFunction : public IObject
    {
    public:
        CResult Initialize(IControl *iMsg);
        void RegistFuncToLua(CLua *pLua);
        CResult EnableDataReceive(int,int);

    protected:
        void ClearReceiveInfoByUUID(string uuid);

    private:
        void RecvDataConnFunc();
        void RecvFileFunc(int);

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
        static int Lua_MoveFile(lua_State *l);
        static int Lua_SendFile(lua_State *l);
        static int Lua_ReceiveFile(lua_State *l);
        static int Lua_CheckRecvStatus(lua_State *l);
        static int Lua_SetExtendData(lua_State *l);
        static int Lua_SetExtendDataByFile(lua_State *l);
        static int Lua_ConnectToDBCenter(lua_State *l);
        static int Lua_SQLQueryToDBCenter(lua_State *l);
        static int Lua_SQLInsertToDBCenter(lua_State *l);
        static int Lua_SQLDeleteToDBCenter(lua_State *l);
        static int Lua_SQLUpdateToDBCenter(lua_State *l);
        static int Lua_PrepareUpload(lua_State *l);
        static int Lua_UploadEnd(lua_State *l);
        static int Lua_GetThumbnail(lua_State *l);
        static int Lua_ConvertImageFormat(lua_State *l);
        static int Lua_SetTimeout(lua_State *l);
        static int Lua_PushEvent(lua_State *l);

    protected:
        void OnStart(SharedEvent);
        void OnStop(SharedEvent);
        void OnReferenceModuleOnline(SharedEvent);
        void OnReferenceModuleOffline(SharedEvent);
        void QueryReferenceInfoResponseHandler(IEvent *, Package *);
        static CResult RunSQLFunction(uint64_t, const string &, int);
        static U64Result SQLFunctionPrepareCheck(lua_State *, int, const string &);
        void AddConnection(uint64_t, const string &, int);
        U64Result GetConnectionID(int);
        void SetTimeout(int n)
        {
            m_nTimeout = n;
        }

    protected:
        map_ex<string, string> m_mapCommData;
        map_ex<string, int> m_mapDBNameToSessionID;
        Json::Value *m_pModuleConfig = nullptr;

        map<int, string> g_RecvDataConnList;
        map<string, map<string, RecvDataPackage *>> g_RecvDataInfoList;

        static constexpr int g_data_pack_len = 8;
        static constexpr int g_uuid_len = 36;
        static constexpr int g_md5_len = 32;
        static constexpr int FILE_TRANS_MAX_SIZE = 20 * 1024 * 1024; //20mb


        map_ex<int32_t, list_ex<uint64_t>> m_mapTemplateIDToUUIDs;
        map_ex<uint64_t, NodeItem> m_mapUUIDToNode;
        map_ex<uint64_t, uint64_t> m_mapUUIDToConnectionID;

        atomic_int32_t m_DataCenterTemplateID = 0;
        atomic_int32_t m_FileCenterTemplateID = 0;

        int m_ListenSock;
        int m_nRecvDataTimeoutTime;

        int m_nTimeout = 5000;

        RUN_STATUS m_emStatus = RUN_STATUS::Created;

    public:
        static unique_ptr<CGlobalFunction> Instance;
    };
} // namespace Sloong
