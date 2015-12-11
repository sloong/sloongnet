// MessgeProcess source file
#include "msgproc.h"
#include "main.h"
#include <boost/format.hpp>
#include <univ/luapacket.h>
#include "globalfunction.h"
CMsgProc::CMsgProc( CLog* pLog )
{
	m_pLog = pLog;
    m_pLua = new CLua();
	m_pGFunc = new CGlobalFunction();
    m_pGFunc->Initialize(m_pLua);
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
	m_pLog->Log("Message is processed. call lua func.");

    CLuaPacket userinfo;
    CLuaPacket request, response;
    request.SetData("message",msg);
    if( !m_pLua->RunFunction("OnRecvMessage",&userinfo,&request,&response))
		m_pLog->Log(m_pLua->GetErrorString());

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
	m_pLog->Log(resmsg);
    return resmsg;
}


