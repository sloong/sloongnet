// MessgeProcess source file
#include "msgproc.h"
#include "main.h"
#include <boost/format.hpp>
#include <univ/luapacket.h>
#include "globalfunction.h"
#include "utility.h"
#include <univ/exception.h>
CMsgProc::CMsgProc()
{
    m_pGFunc = new CGlobalFunction();
    m_pReloadTagList = NULL;
}

CMsgProc::~CMsgProc()
{
	int nLen = m_pLuaList.size();
	for (int i = 0; i < nLen; i++)
    {
		SAFE_DELETE(m_pLuaList[i]);
    }
	SAFE_DELETE(m_pGFunc);
	SAFE_DELETE_ARR(m_pReloadTagList);
}

void CMsgProc::Initialize(CLog *pLog, string scriptFolder)
{
    m_pLog = pLog;
    m_strScriptFolder = scriptFolder;
	
    m_pGFunc->Initialize(m_pLog);
}

void CMsgProc::InitLua(CLua* pLua, string path)
{
	pLua->RunScript("init.lua");
    // get current path
    char szDir[MAX_PATH] = {0};

    getcwd(szDir,MAX_PATH);
    string strDir(szDir);
    strDir += "/" + path;
	pLua->RunFunction("Init", CUniversal::Format("'%s'", strDir));
}

int CMsgProc::MsgProcess( int id, CLuaPacket* pUInfo, string& msg, string&res, char*& pBuf)
{
    // In process, need add the lua script runtime and call lua to process.
    // In here, just show log to test.

	// process msg, get the md5 code and the swift number.
	CLua* pLua = m_pLuaList[id];

	if (m_pReloadTagList[id] == true)
	{
		InitLua(pLua, m_strScriptFolder);
		m_pReloadTagList[id] = false;
	}
		

    CLuaPacket request, response;
	request.SetData("message", msg);
	if (!pLua->RunFunction("ProgressMessage", pUInfo, &request, &response))
		m_pLog->Log(pLua->GetErrorString());

    // check the return ;
    string opt = response.GetData("operation");
	int nSize = 0;
    if( opt == "ReloadScript")
    {
		int nlen = m_pLuaList.size();
		for (int i = 0; i < nlen; i++)
		{
			m_pReloadTagList[i] = true;
		}
		InitLua(pLua,m_strScriptFolder);
		m_pReloadTagList[id] = false;
        res = "0|succeed|Reload succeed";
    }
	else if (opt == "loadfile")
	{
        auto filename = response.GetData("filepath");
        try
        {
            nSize = CUtility::ReadFile(filename, pBuf);
        }
        catch( normal_except e )
        {
            m_pLog->Log(e.what(),ERR);
            res = string("-2|") + e.what();
            return 0;
        }
    }

    res = response.GetData("errno") + "|" + response.GetData("errmsg") + "|" + response.GetData("message");

    m_pLog->Log(res);
	return nSize;
}

int Sloong::CMsgProc::NewThreadInit()
{

	CLua* pLua = new CLua();
	pLua->SetScriptFolder(m_strScriptFolder);
	m_pGFunc->InitLua(pLua);
	InitLua(pLua,m_strScriptFolder);
    m_luaMutex.lock();
    m_pLuaList.push_back(pLua);
    int id = m_pLuaList.size()-1;
	SAFE_DELETE_ARR(m_pReloadTagList);
	m_pReloadTagList = new bool[m_pLuaList.size()];
	memset(m_pReloadTagList, 0, m_pLuaList.size()*sizeof(bool));
    m_luaMutex.unlock();
    return id;
}

