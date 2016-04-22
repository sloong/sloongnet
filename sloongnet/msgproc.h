// MessageProcess hard file
#ifndef MSGPROC_H
#define MSGPROC_H

#define LUA_INT_TYPE LUA_INT_LONG

#include <univ/lua.h>
#include <univ/log.h>

namespace Sloong
{
	using namespace Universal;
	class CGlobalFunction;
	struct MySQLConnectInfo;
	struct LuaScriptConfigInfo;
	class CMsgProc
	{
	public:
        CMsgProc();
		~CMsgProc();
        void Initialize(CLog* pLog, MySQLConnectInfo* mysqlinfo, LuaScriptConfigInfo* luainfo, bool showSQLCmd, bool showSQLRes);
		int MsgProcess( int id, CLuaPacket* pUInfo, string& msg, string&res, char*& pBuf);
		int NewThreadInit();
		void InitLua(CLua* pLua, string folder);

	protected:
		vector<CLua*>	m_pLuaList;
		CLog*			m_pLog;
		CGlobalFunction*	m_pGFunc;
        mutex           m_luaMutex;
		LuaScriptConfigInfo* m_stLuaConfig;
	};
}

#endif // MSGPROC_H
