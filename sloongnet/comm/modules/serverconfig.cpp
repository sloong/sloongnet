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

	// Log config init
	m_oLogInfo.DebugMode = true;
	m_oLogInfo.ShowSendMessage = false;
	m_oLogInfo.ShowReceiveMessage = false;
	m_oLogInfo.LogWriteToOneFile = false;
	m_oLogInfo.LogPath = "./sloongnet.log";
	m_oLogInfo.LogLevel = 0;
	m_oLogInfo.NetworkPort = 0;
}

bool CServerConfig::Initialize(string path, string exConfig)
{
	m_strConfigPath = path;
	m_strExConfigPath = exConfig;
	if (exConfig != "")
		m_bExConfig = true;

	if (0 != access(m_strConfigPath.c_str(), ACC_R))
		return false;
	if (m_bExConfig && 0 != access(m_strExConfigPath.c_str(), ACC_R))
		throw normal_except("cannot read the exConfig file:" + exConfig);
		//return false;

	m_pFile = g_key_file_new();
	g_key_file_load_from_file(m_pFile, m_strConfigPath.c_str(), G_KEY_FILE_NONE, &m_pErr);
	if (m_pErr)
	{
		string strErr = m_pErr->message;
		g_error_free(m_pErr);
		throw normal_except(strErr);
	}

	if (m_bExConfig)
	{
		m_pExFile = g_key_file_new();
		g_key_file_load_from_file(m_pExFile, m_strExConfigPath.c_str(), G_KEY_FILE_NONE, &m_pErr);
	}
	
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
	if( m_pFile != NULL )
		g_key_file_free(m_pFile);
	if( m_pExFile != NULL )
		g_key_file_free(m_pExFile);
}

string Sloong::CServerConfig::GetStringConfig(string strSection, string strKey, string strDef, bool bThrowWhenFialed /*= false */)
{
	if (g_pThis == NULL || g_pThis->m_pFile == NULL  )
		throw normal_except("Config object no initialize");

	if (strSection == "" || strKey == "")
		throw normal_except("Param empty.");

	bool bHas = g_key_file_has_key(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), NULL);
	bool bExHas = false;
	if(g_pThis->m_bExConfig )
		bExHas = g_key_file_has_key(g_pThis->m_pExFile, strSection.c_str(), strKey.c_str(), NULL);
	if (!bHas && !bExHas)
	{
		return strDef;
	}

	string strRes;
	if (bExHas)
		strRes = g_key_file_get_string(g_pThis->m_pExFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);
	else
		strRes = g_key_file_get_string(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);

	if (g_pThis->m_pErr)
	{
		if (bThrowWhenFialed && bExHas )
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strExConfigPath, strSection, strKey));
		else if(bThrowWhenFialed)
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strConfigPath, strSection, strKey));
		else
			strRes = strDef;
	}
	g_clear_error(&g_pThis->m_pErr);
	return strRes;
}

bool Sloong::CServerConfig::GetBoolenConfig(string strSection, string strKey, bool bDef, bool bThrowWhenFialed /*= false*/)
{
	if (g_pThis == NULL || g_pThis->m_pFile == NULL)
		throw normal_except("Config object no initialize");

	bool bHas = g_key_file_has_key(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), NULL);
	bool bExHas = false;
	if (g_pThis->m_bExConfig)
		bExHas = g_key_file_has_key(g_pThis->m_pExFile, strSection.c_str(), strKey.c_str(), NULL);
	if (!bHas && !bExHas)
	{
		return bDef;
	}

	bool bRes;

	if (bExHas)
		bRes = g_key_file_get_boolean(g_pThis->m_pExFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);
	else
		bRes = g_key_file_get_boolean(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);
	if (g_pThis->m_pErr)
	{
		if (bThrowWhenFialed && bExHas)
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strExConfigPath, strSection, strKey));
		else if (bThrowWhenFialed)
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strConfigPath, strSection, strKey));
		else
			bRes = bDef;
	}
	g_clear_error(&g_pThis->m_pErr);
	return bRes;
}

int Sloong::CServerConfig::GetIntConfig(string strSection, string strKey, int nDef, bool bThrowWhenFialed /*= false*/)
{
	if (g_pThis == NULL|| g_pThis->m_pFile == NULL)
		throw normal_except("Config object no initialize");

	bool bHas = g_key_file_has_key(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), NULL);
	bool bExHas = false;
	if (g_pThis->m_bExConfig)
		bExHas = g_key_file_has_key(g_pThis->m_pExFile, strSection.c_str(), strKey.c_str(), NULL);
	if (!bHas && !bExHas)
	{
		return nDef;
	}

	int nRes;
	if (bExHas)
		nRes = g_key_file_get_integer(g_pThis->m_pExFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);
	else
		nRes = g_key_file_get_integer(g_pThis->m_pFile, strSection.c_str(), strKey.c_str(), &g_pThis->m_pErr);
	if (g_pThis->m_pErr)
	{
		if (bThrowWhenFialed && bExHas)
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strExConfigPath, strSection, strKey));
		else if (bThrowWhenFialed)
			throw normal_except(CUniversal::Format("Load config error.Message:[%s].Path:[%s].Section:[%s].Key:[%s].", g_pThis->m_pErr->message, g_pThis->m_strConfigPath, strSection, strKey));
		else
			nRes = nDef;
	}
	g_clear_error(&g_pThis->m_pErr);
	return nRes;
}

void Sloong::CServerConfig::LoadConfig()
{	// load lua config 
	m_oLuaConfigInfo.ScriptFolder = GetStringConfig("Lua", "ScriptFolder", "./");
	m_oLuaConfigInfo.EntryFile = GetStringConfig("Lua", "EntryFile", "init.lua");
	m_oLuaConfigInfo.EntryFunction = GetStringConfig("Lua", "EntryFunction", "Init");
	m_oLuaConfigInfo.ProcessFunction = GetStringConfig("Lua", "ProcessFunction", "MessageProcess");
	m_oLuaConfigInfo.SocketCloseFunction = GetStringConfig("Lua", "SocketCloseFunction", "SocketCloseProcess");

	// Load Log config info
	m_oLogInfo.DebugMode = GetBoolenConfig("Log", "DebugMode", m_oLogInfo.DebugMode);
	m_oLogInfo.LogPath = GetStringConfig("Log", "LogPath", m_oLogInfo.LogPath);
	m_oLogInfo.ShowReceiveMessage = GetBoolenConfig("Log", "ShowReceiveMessage", m_oLogInfo.ShowReceiveMessage);
	m_oLogInfo.ShowSendMessage = GetBoolenConfig("Log", "ShowSendMessage", m_oLogInfo.ShowSendMessage);
	m_oLogInfo.LogWriteToOneFile = GetBoolenConfig("Log", "WriteToOneFile", m_oLogInfo.LogWriteToOneFile);
	m_oLogInfo.LogLevel = GetIntConfig("Log", "LogLevel", m_oLogInfo.LogLevel);
	m_oLogInfo.NetworkPort = GetIntConfig("Log", "NetworkLogPort", m_oLogInfo.NetworkPort);

	// Load server info
	m_nPort = GetIntConfig("Server", "Port", m_nPort);
	m_bEnableSSL = GetBoolenConfig("Server", "EnableSSL", m_bEnableSSL);
	if (m_bEnableSSL)
	{
		m_strCertFile = GetStringConfig("Server", "SSLCertFilePath", m_strCertFile);
		m_strKeyFile = GetStringConfig("Server", "SSLKeyFilePath", m_strKeyFile);
		m_strPasswd = GetStringConfig("Server", "SSLPassword", m_strPasswd);
	}
	m_nConnectTimeout = GetIntConfig("Server", "ConnectTimeout", m_nConnectTimeout);
	m_nReceiveTimeout = GetIntConfig("Server", "ReceiveTimeout", m_nReceiveTimeout);
	m_bEnableDataReceive = GetBoolenConfig("Server", "EnableDataReceive", m_bEnableDataReceive);
	if (m_bEnableDataReceive)
	{
		m_nDataReceivePort = GetIntConfig("Server", "DataReceivePort", m_nDataReceivePort);
		m_nWaitRecvTimeout = GetIntConfig("Server", "WaitRecvTimeout", m_nWaitRecvTimeout);
	}


	// Security
	m_nClientCheckTime = GetIntConfig("Security", "ClientCkeckTime", m_nClientCheckTime);
	m_strClientCheckKey = GetStringConfig("Security", "ClientCheckKey", m_strClientCheckKey);
	

	// Performance
	m_nMessageCenterThreadQuantity = GetIntConfig("Performance", "MessageCenterThreadQuantity", m_nMessageCenterThreadQuantity);
	m_nLuaProcessThreadQuantity = GetIntConfig("Performance", "LuaProcessThreadQuantity", m_nLuaProcessThreadQuantity);
	m_nEPoolThreadQuantity = GetIntConfig("Performance", "EPoolThreadQuantity", m_nEPoolThreadQuantity);
	m_nTimeoutInterval = GetIntConfig("Performance", "TimeoutInterval", m_nTimeoutInterval);	
}
