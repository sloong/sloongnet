#include <string>
#include <glib.h>
#include "serverconfig.h"
#include <univ/exception.h>
using namespace std;
using namespace Sloong;
using namespace Sloong::Universal;
CServerConfig::CServerConfig()
{
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
}

bool CServerConfig::LoadConfigFile(string path)
{
	if (0 != access(path.c_str(), ACC_R))
		return false;
	GError* err = NULL;
	GKeyFile* conf = g_key_file_new();
	g_key_file_load_from_file(conf, path.c_str(), G_KEY_FILE_NONE, &err);
	if (err)
	{
		string strErr = err->message;
		g_error_free(err);
		throw normal_except(strErr);
	}

	int nRes;
	string strRes;
	bool bRes;

	// load connect info
	nRes = g_key_file_get_integer(conf, "MySQL", "Port", &err);
	if (!err)
		m_oConnectInfo.Port = nRes;
	g_clear_error(&err);
	strRes = g_key_file_get_string(conf, "MySQL", "Address", &err);
	if (!err)
		m_oConnectInfo.Address = strRes;
	g_clear_error(&err);
	strRes = g_key_file_get_string(conf, "MySQL", "User", &err);
	if (!err)
		m_oConnectInfo.User = strRes;
	g_clear_error(&err);
	strRes = g_key_file_get_string(conf, "MySQL", "Password", &err);
	if (!err)
		m_oConnectInfo.Password = strRes;
	g_clear_error(&err);
	strRes = g_key_file_get_string(conf, "MySQL", "Database", &err);
	if (!err)
		m_oConnectInfo.Database = strRes;
	g_clear_error(&err);

	nRes = g_key_file_get_integer(conf, "Server", "Port", &err);
	if ( !err)
		m_nPort = nRes;
    g_clear_error(&err);

	strRes = g_key_file_get_string(conf, "Server", "LogPath", &err);
	if (!err)
		m_strLogPath = strRes;
    g_clear_error(&err);
	
	bRes = g_key_file_get_boolean(conf, "Server", "RunType", &err);
	if (!err)
		m_bDebug = bRes;
    g_clear_error(&err);

	nRes = g_key_file_get_integer(conf, "Server", "ProcessThreadQuantity", &err);
	if (!err)
		m_nProcessThreadQuantity = nRes;
    g_clear_error(&err);

	nRes = g_key_file_get_integer(conf, "Server", "EPoolThreadQuantity", &err);
	if (!err)
		m_nEPoolThreadQuantity = nRes;
    g_clear_error(&err);

	strRes = g_key_file_get_string(conf, "Server", "ScriptFolder", &err);
	if (!err)
		m_strScriptFolder = strRes;
    g_clear_error(&err);

	nRes = g_key_file_get_integer(conf, "Server", "PriorityLevel", &err);
	if (!err)
		m_nPriorityLevel = nRes;
    g_clear_error(&err);

	nRes = g_key_file_get_integer(conf, "Server", "SleepInterval", &err);
	if (!err)
		m_nSleepInterval = nRes;
    g_clear_error(&err);

	bRes = g_key_file_get_boolean(conf, "Server", "EnableMD5Check", &err);
	if (!err)
		m_bEnableMD5Check = bRes;
	g_clear_error(&err);

	bRes = g_key_file_get_boolean(conf, "Server", "EnableSwiftNumberSupport", &err);
	if (!err)
		m_bEnableSwiftNumberSup = bRes;
	g_clear_error(&err);


	bRes = g_key_file_get_boolean(conf, "Log", "ShowReceiveMessage", &err);
	if (!err)
		m_bShowReceiveMessage = bRes;
	g_clear_error(&err);

	bRes = g_key_file_get_boolean(conf, "Log", "ShowSendMessage", &err);
	if (!err)
		m_bShowSendMessage = bRes;
	g_clear_error(&err);

	bRes = g_key_file_get_boolean(conf, "Log", "WriteToOneFile", &err);
	if (!err)
		m_bLogWriteToOneFile = bRes;
	g_clear_error(&err);
	
	
	g_key_file_free(conf);
	return true;
}


CServerConfig::~CServerConfig()
{
}
