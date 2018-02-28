#ifndef CGLOBALFUNCTION_H
#define CGLOBALFUNCTION_H

#include <univ/lua.h>
#include "IData.h"
#include "IMessage.h"
namespace Sloong
{
	struct SendExDataInfo
	{
		int m_nDataSize;
		char* m_pData;
		bool m_bIsEmpty;
	};

	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;
	class CUtility;
	class CDBProc;
	struct MySQLConnectInfo;
	using namespace Interface;
	class CGlobalFunction
	{
	public:
        CGlobalFunction();
		~CGlobalFunction();

		void Initialize(IMessage* iMsg,IData* iData);
		void InitLua(CLua* pLua);
	protected:
        CUtility * m_pUtility;
		CDBProc* m_pDBProc;
		CLog*	m_pLog;
		IMessage* m_iMsg;
		IData*		m_iData;
	public:


		static int Lua_showLog(lua_State* l);
		static int Lua_querySql(lua_State* l);
        static int Lua_getSqlError(lua_State* l);
		static int Lua_getThumbImage(lua_State* l);
		static int Lua_getEngineVer(lua_State* l);
		static int Lua_Base64_Encode(lua_State* l);
		static int Lua_Base64_Decode(lua_State* l);
		static int Lua_MD5_Encode(lua_State* l);
		static int Lua_SendFile(lua_State* l);
		static int Lua_ReloadScript(lua_State* l);
		static int Lua_GetConfig(lua_State* l);
		static int Lua_MoveFile(lua_State* l);
		static int Lua_GenUUID(lua_State* l);
		static int Lua_ReceiveFile(lua_State* l);
		static int Lua_SetCommData(lua_State* l);
		static int Lua_GetCommData(lua_State* l);
		
	public:
		static CGlobalFunction* g_pThis;
		bool		m_bShowSQLCmd;
		bool		m_bShowSQLResult;
		MySQLConnectInfo* m_pSQLInfo;
	};
}
#endif // !CGLOBALFUNCTION_H



