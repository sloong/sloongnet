// MessgeProcess source file
#include "msgproc.h"
#include "main.h"
#include "boost/format.hpp"
CMsgProc::CMsgProc()
{
    m_pLua = new CLua();

//	luaL_openlibs(m_pLua);
}

CMsgProc::~CMsgProc()
{
    delete m_pLua;
//	lua_close(m_pLua);
}

bool CMsgProc::MsgProcess(vector<string>& msg)
{
    // In process, need add the lua script runtime and call lua to process.
    // In here, just show log to test.
    CLog::showLog(INF,(boost::format("Message is processed. function id is %s.")%msg[0]).str());
    //luaL_loadfile(m_pLua,"main.lua");
    //lua_pcall(m_pLua,0,LUA_MULTRET,0);
    m_pLua->RunScript("main");
    return true;
}
