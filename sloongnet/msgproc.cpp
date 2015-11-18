// MessgeProcess source file
#include "msgproc.h"
#include "main.h"
#include <boost/format.hpp>
#include "univ/luapacket.h"
CMsgProc::CMsgProc()
{
    m_pLua = new CLua();
    m_pLua->RunScript("init.lua");
    // get current path
    char szDir[MAX_PATH] = {0};

    getcwd(szDir,MAX_PATH);
    string strDir(szDir);
    strDir += "/";
    m_pLua->RunFunction("Init","'"+strDir+"'");

}

CMsgProc::~CMsgProc()
{
    delete m_pLua;
}

bool CMsgProc::MsgProcess(string& msg)
{
    // In process, need add the lua script runtime and call lua to process.
    // In here, just show log to test.
    CLog::showLog(INF,"Message is processed. call lua func.");

    CLuaPacket userinfo;
    CLuaPacket request, response;
    request.SetData("message",msg);
    m_pLua->RunFunction("OnRecvMessage",&userinfo,&request,&response);
    string funcid = response.GetData("funcid");
    CLog::showLog(INF,boost::format("Message is processed. function id is %s.")%funcid);
    return true;
}
