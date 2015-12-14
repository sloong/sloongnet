// MessgeProcess source file
#include "msgproc.h"
#include "main.h"
#include <boost/format.hpp>
#include <univ/luapacket.h>
#include "globalfunction.h"

CMsgProc::CMsgProc()
{
    m_pLua = new CLua();
    m_pGFunc = new CGlobalFunction();
}

CMsgProc::~CMsgProc()
{
    delete m_pLua;
}

void CMsgProc::Initialize(CLog *pLog)
{
    m_pLog = pLog;
    m_pGFunc->Initialize(m_pLog,m_pLua);
    InitLua();
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

string CMsgProc::MsgProcess( CLuaPacket* pUInfo, string& msg)
{
    // In process, need add the lua script runtime and call lua to process.
    // In here, just show log to test.
	m_pLog->Log("Message is processed. call lua func.");

	// process msg, get the md5 code and the swift number.
    CLuaPacket request, response;
	request.SetData("message", msg);
	if (!m_pLua->RunFunction("OnRecvMessage", pUInfo, &request, &response))
		m_pLog->Log(m_pLua->GetErrorString());

    // check the return ;
    string opt = response.GetData("operation");
	string res;
    if( opt == "reload")
    {
        InitLua();
        res = "0|succeed|Reload succeed";
    }
	else
	{
		 res = response.GetData("errno") + "|" + response.GetData("errmsg") + "|" + response.GetData("message");
	}
	
	m_pLog->Log(res);
	return res;
}


