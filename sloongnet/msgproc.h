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
        void Initialize(CLog* pLog);
		int MsgProcess(CLuaPacket* pUInfo, string& msg, string&res, char*& pBuf);
		void InitLua();

	protected:
		CLua*	m_pLua;
		CLog*	m_pLog;
		CGlobalFunction*	m_pGFunc;
	};
}

#endif // MSGPROC_H
