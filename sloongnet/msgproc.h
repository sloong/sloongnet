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
	class CMsgProc
	{
	public:
        CMsgProc();
		~CMsgProc();
        void Initialize(CLog* pLog, string scriptFolder);
		int MsgProcess( int id, CLuaPacket* pUInfo, string& msg, string&res, char*& pBuf);
		int NewThreadInit();
		void InitLua(CLua* pLua, string folder);

	protected:
		vector<CLua*>	m_pLuaList;
		CLog*			m_pLog;
		CGlobalFunction*	m_pGFunc;
        string m_strScriptFolder;
	};
}

#endif // MSGPROC_H
