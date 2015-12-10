// MessgeProcess source file
#include "msgproc.h"
#include "main.h"
#include <boost/format.hpp>
#include "univ/luapacket.h"

CLog* CMsgProc::g_pLog = NULL;

CMsgProc::CMsgProc( CLog* pLog )
{
	g_pLog = pLog;
    m_pLua = new CLua();
	m_pLua->SetErrorHandle(CMsgProc::HandleError);
    InitLua();
}

CMsgProc::~CMsgProc()
{
    delete m_pLua;
}

void CMsgProc::InitLua()
{
    m_pLua->RunScript("init.lua");
    // get current path
    char szDir[MAX_PATH] = {0};

    getcwd(szDir,MAX_PATH);
    string strDir(szDir);
    strDir += "/";
    m_pLua->RunFunction("Init",CUniversal::Format("'%s'",strDir));
}

string CMsgProc::MsgProcess(string& msg)
{
    // In process, need add the lua script runtime and call lua to process.
    // In here, just show log to test.
	g_pLog->Log("Message is processed. call lua func.");

    CLuaPacket userinfo;
    CLuaPacket request, response;
    request.SetData("message",msg);
    if( !m_pLua->RunFunction("OnRecvMessage",&userinfo,&request,&response))
		g_pLog->Log(m_pLua->GetErrorString());

    // check the return ;
    string opt = response.GetData("operation");
    if( opt == "reload")
    {
        InitLua();
        return "Reload succeed";
    }
    else if( opt == "sendmessage")
    {
        string msg = response.GetData("message");
    }
    string funcid = response.GetData("funcid");
    string res = response.GetData("result");
    string resmsg = CUniversal::Format("Message is processed. function id is %s.result is :%s",funcid,res);
	g_pLog->Log(resmsg);
    return resmsg;
}

void CMsgProc::HandleError(string err)
{
	g_pLog->Log( err, ERR, -2);
}
