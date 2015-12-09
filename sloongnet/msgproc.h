// MessageProcess hard file
#ifndef MSGPROC_H
#define MSGPROC_H

#define LUA_INT_TYPE LUA_INT_LONG

#include <univ/lua.h>
using namespace Sloong::Universal;
class CMsgProc
{
public:
    CMsgProc();
    ~CMsgProc();
    string MsgProcess(string& msg);
    void InitLua();
    CLua*	m_pLua;
};

#endif // MSGPROC_H
