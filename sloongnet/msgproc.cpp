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

string CMsgProc::MsgProcess(string& msg)
{
    // In process, need add the lua script runtime and call lua to process.
    // In here, just show log to test.
    CLog::showLog(INF,"Message is processed. call lua func.");

    CLuaPacket userinfo;
    CLuaPacket request, response;
    request.SetData("message",msg);
    if( !m_pLua->RunFunction("OnRecvMessage",&userinfo,&request,&response))

    // check the return ;
    string type = response.GetData("type");

    string funcid = response.GetData("funcid");
    string res = response.GetData("result");
    string resmsg = CUniversal::Format("Message is processed. function id is %s.result is :%s",funcid,res);
    CLog::showLog(INF,resmsg);
    return resmsg;
}
