// MessageProcess hard file
#ifndef MSGPROC_H
#define MSGPROC_H

#define LUA_INT_TYPE LUA_INT_LONG

#include <univ/lua.h>
#include <univ/log.h>
using namespace Sloong::Universal;
class CMsgProc
{
public:
    CMsgProc( CLog* pLog );
    ~CMsgProc();
    string MsgProcess(string& msg);
    void InitLua();
    CLua*	m_pLua;

	static void HandleError(string err);
protected:
	static CLog*	g_pLog;
};

#endif // MSGPROC_H
