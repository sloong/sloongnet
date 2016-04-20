#include <string>
#include "serverconfig.h"
#include <univ/exception.h>
using namespace std;
using namespace Sloong;
using namespace Sloong::Universal;

CServerConfig* Sloong::CServerConfig::g_pThis;

CServerConfig::CServerConfig()
{
	g_pThis = this;

	m_pErr = NULL;
	m_pFile = NULL;

	// DB init
	m_oConnectInfo.Port = 3306;
	m_oConnectInfo.Address = "localhost";
	m_oConnectInfo.User = "root";
	m_oConnectInfo.Password = "sloong";
	m_oConnectInfo.Database = "sloong";

	// Server init
	m_nPort = 9009;
	m_bDebug = true;
	m_strLogPath = "./log.log";
	m_strScriptFolder = "./";
	m_nEPoolThreadQuantity = 1;
	m_nProcessThreadQuantity = 1;
    m_nPriorityLevel = 0;
	m_nSleepInterval = 100;
	m_bShowSendMessage = false;
	m_bShowReceiveMessage = false;
	m_bLogWriteToOneFile = false;
	m_bEnableMD5Check = false;
	m_bEnableSwiftNumberSup = false;
	m_bShowSQLCmd = false;
	m_bShowSQLResult = false;
}

bool CServerConfig::Initialize(string path)
{
	m_strConfigPath = path;
	if (0 != access(path.c_str(), ACC_R))
		return false;

	m_pFile = g_key_file_new();
	g_key_file_load_from_file(m_pFile, path.c_str(), G_KEY_FILE_NONE, &m_pErr);
	if (m_pErr)
	{
		string strErr = m_pErr->message;
		g_error_free(m_pErr);
		throw normal_except(strErr);
	}

	return true;
}


CServerConfig::~CServerConfig()
{
	g_key_file_free(m_pFile);
}

string Sloong::CServerConfig::GetStringConfig(string strSection, string strKey, string strDef, bool bThrowWhenFialed /*= false */)
{
	if (g_pThis == NULL || g_pThis->m_pFile == NULL  )
		throw normal_except("Config object no initialize");

	if (strSection == "" || strKey == "")
		throw normal_except("Param empty.");

	string strRes;

	// load connect info
	strRes = g_key_file_get_string(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);
	if (g_pThis->m_pErr)
	{
		if (bThrowWhenFialed)
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strConfigPath, strSection, strKey));
		else
			strRes = strDef;
	}
	g_clear_error(&g_pThis->m_pErr);
	return strRes;
}

bool Sloong::CServerConfig::GetBoolenConfig(string strSection, string strKey, bool& bDef, bool bThrowWhenFialed /*= false*/)
{
	if (g_pThis == NULL || g_pThis->m_pFile == NULL)
		throw normal_except("Config object no initialize");

	bool bRes;

	// load connect info
	bRes = g_key_file_get_boolean(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);
	if (g_pThis->m_pErr)
	{
		if (bThrowWhenFialed)
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strConfigPath, strSection, strKey));
		else
			bRes = bDef;
	}
	g_clear_error(&g_pThis->m_pErr);
	return bRes;
}

int Sloong::CServerConfig::GetIntConfig(string strSection, string strKey, int& nDef, bool bThrowWhenFialed /*= false*/)
{
	if (g_pThis == NULL|| g_pThis->m_pFile == NULL)
		throw normal_except("Config object no initialize");

	int nRes;
	
	// load connect info
	nRes = g_key_file_get_integer(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);
	if (g_pThis->m_pErr)
	{
		if (bThrowWhenFialed)
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strConfigPath, strSection, strKey));
		else
			nRes = nDef;
	}
	g_clear_error(&g_pThis->m_pErr);
	return nRes;
}

void Sloong::CServerConfig::LoadConfig()
{
	// load connect info
	m_oConnectInfo.Port = GetIntConfig("MySQL", "Port", m_oConnectInfo.Port);
	m_oConnectInfo.Address = GetStringConfig("MySQL", "Address", m_oConnectInfo.Address);
	m_oConnectInfo.User = GetStringConfig("MySQL", "User", m_oConnectInfo.User);
	m_oConnectInfo.Password = GetStringConfig("MySQL", "Password", m_oConnectInfo.Password);
	m_oConnectInfo.Database = GetStringConfig("MySQL", "Database", m_oConnectInfo.Database);

	// Load server info
	m_nPort = GetIntConfig("Server", "Port", m_nPort);
	m_bDebug = GetBoolenConfig("Server", "RunType", m_bDebug);
	m_nPriorityLevel = GetIntConfig("Server", "PriorityLevel", m_nPriorityLevel);
	m_bEnableMD5Check = GetBoolenConfig("Server", "EnableMD5Check", m_bEnableMD5Check);
	m_bEnableSwiftNumberSup = GetBoolenConfig("Server", "EnableSwiftNumberSupport", m_bEnableSwiftNumberSup);
	m_nSleepInterval = GetIntConfig("Performance", "SleepInterval", m_nSleepInterval);
	m_nProcessThreadQuantity = GetIntConfig("Performance", "ProcessThreadQuantity", m_nProcessThreadQuantity);
	m_nEPoolThreadQuantity = GetIntConfig("Performance", "EPoolThreadQuantity", m_nEPoolThreadQuantity);

	// path
	m_strScriptFolder = GetStringConfig("Path", "ScriptFolder", m_strScriptFolder);
	m_strLogPath = GetStringConfig("Path", "LogPath", m_strLogPath);

	// Load log config
	m_bShowReceiveMessage = GetBoolenConfig("Log", "ShowReceiveMessage", m_bShowReceiveMessage);
	m_bShowSendMessage = GetBoolenConfig("Log", "ShowSendMessage", m_bShowSendMessage);
	m_bLogWriteToOneFile = GetBoolenConfig("Log", "WriteToOneFile", m_bLogWriteToOneFile);
	m_bShowSQLCmd = GetBoolenConfig("Log", "ShowSQLCmd", m_bShowSQLCmd);
	m_bShowSQLResult = GetBoolenConfig("Log", "ShowSQLResult", m_bShowSQLResult);
}
