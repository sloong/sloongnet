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
    
}

CMsgProc::~CMsgProc()
{
	int nLen = m_pLuaList.size();
	for (int i = 0; i < nLen; i++)
    {
		SAFE_DELETE(m_pLuaList[i]);
    }
	SAFE_DELETE(m_pGFunc);
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

	if (m_pGFunc->m_pReloadTagList[id] == true)
	{
		InitLua(pLua, m_strScriptFolder);
		m_pGFunc->m_pReloadTagList[id] = false;
	}

	int nRes = pLua->RunFunction("ProgressMessage", pUInfo, msg, res);
	if (nRes >= 0 && nRes < m_pGFunc->m_oSendExMapList.size())
	{
		pBuf = m_pGFunc->m_oSendExMapList[nRes].m_pData;
		int nSize = m_pGFunc->m_oSendExMapList[nRes].m_nDataSize;
		unique_lock<mutex> lck(m_pGFunc->m_oListMutex);
		m_pGFunc->m_oSendExMapList[nRes].m_pData = NULL;
		m_pGFunc->m_oSendExMapList[nRes].m_nDataSize = 0;
		m_pGFunc->m_oSendExMapList[nRes].m_bIsEmpty = true;
		lck.unlock();
		return nSize;
	}
	else if( nRes == -2) 
		m_pLog->Log(res);

    //m_pLog->Log(res);
	return nRes;
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
	SAFE_DELETE_ARR(m_pGFunc->m_pReloadTagList);
	m_pGFunc->m_nTagSize = m_pLuaList.size();
	m_pGFunc->m_pReloadTagList = new bool[m_pGFunc->m_nTagSize];
	memset(m_pGFunc->m_pReloadTagList, 0, m_pGFunc->m_nTagSize*sizeof(bool));
    m_luaMutex.unlock();
    return id;
}

