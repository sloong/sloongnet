// MessgeProcess source file
#include "msgproc.h"
#include "main.h"
#include <boost/format.hpp>
#include <univ/luapacket.h>
#include "globalfunction.h"
#include "utility.h"
#include <univ/exception.h>
#include "structs.h"

CLog* CMsgProc::m_pLog = NULL;

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

void CMsgProc::Initialize(CLog *pLog, MySQLConnectInfo* mysqlinfo, LuaScriptConfigInfo* luainfo, bool showSQLCmd, bool showSQLRes)
{
    m_pLog = pLog;
	m_pLuaConfig = luainfo;
	m_pGFunc->Initialize(m_pLog, mysqlinfo, showSQLCmd, showSQLRes);
}

void CMsgProc::InitLua(CLua* pLua, string path)
{
	if (!pLua->RunScript(m_pLuaConfig->EntryFile))
	{
		throw normal_except("Run Script Fialed.");
	}
    // get current path
    char szDir[MAX_PATH] = {0};

    getcwd(szDir,MAX_PATH);
    string strDir(szDir);
    strDir += "/" + path;
	pLua->RunFunction(m_pLuaConfig->EntryFunction, CUniversal::Format("'%s'", strDir));
}

int CMsgProc::MsgProcess( int id, CLuaPacket* pUInfo, string& msg, string&res, char*& pBuf)
{
    // In process, need add the lua script runtime and call lua to process.
    // In here, just show log to test.

	// process msg, get the md5 code and the swift number.
	CLua* pLua = m_pLuaList[id];

	if (m_pGFunc->m_pReloadTagList[id] == true)
	{
		InitLua(pLua, m_pLuaConfig->ScriptFolder);
		m_pGFunc->m_pReloadTagList[id] = false;
	}

	int nRes = pLua->RunFunction(m_pLuaConfig->ProcessFunction, pUInfo, msg, res);
	if (nRes >= 0 )
	{
		if (nRes >= (int)m_pGFunc->m_oSendExMapList.size())
		{
			m_pLog->Log(CUniversal::Format("Call function end, but the res is error: res [%d], SendMapList size[%d]", nRes, m_pGFunc->m_oSendExMapList.size()), LOGLEVEL::ERR);
			return 0;
		}
		pBuf = m_pGFunc->m_oSendExMapList[nRes].m_pData;
		int nSize = m_pGFunc->m_oSendExMapList[nRes].m_nDataSize;
		m_pLog->Log(CUniversal::Format("Send Ex Data, Size[%d], Message[%s]", nSize,msg.c_str()), LOGLEVEL::ERR);
		unique_lock<mutex> lck(m_pGFunc->m_oListMutex);
		m_pGFunc->m_oSendExMapList[nRes].m_pData = NULL;
		m_pGFunc->m_oSendExMapList[nRes].m_nDataSize = 0;
		m_pGFunc->m_oSendExMapList[nRes].m_bIsEmpty = true;
		lck.unlock();
		return nSize;
	}
	else
	{
		m_pLog->Log(res);
		return 0;
	}
}

void Sloong::CMsgProc::CloseSocket(int id, CLuaPacket* pUInfo)
{
	CLua* pLua = m_pLuaList[id];
	pLua->RunFunction(m_pLuaConfig->SocketCloseFunction, pUInfo);
}

void Sloong::CMsgProc::HandleError(string err)
{
	m_pLog->Log(err, ERR, -2);
}

int Sloong::CMsgProc::NewThreadInit()
{
	CLua* pLua = new CLua();
	pLua->SetErrorHandle(HandleError);
	pLua->SetScriptFolder(m_pLuaConfig->ScriptFolder);
	m_pGFunc->InitLua(pLua);
	InitLua(pLua, m_pLuaConfig->ScriptFolder);
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

