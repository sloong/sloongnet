#include <string>
#include <glib.h>
#include "serverconfig.h"

using namespace std;
using namespace Sloong;

CServerConfig::CServerConfig()
{
	m_nPort = 9009;
	m_bDebug = true;
	m_strLogPath = "./log.log";
	m_nThreadNum = 1;
}

bool CServerConfig::LoadConfigFile(string path)
{
	GKeyFile* conf = g_key_file_new();
	g_key_file_load_from_file(conf, path.c_str(), G_KEY_FILE_NONE, NULL);
	m_nPort = g_key_file_get_integer(conf, "Server", "Port", NULL);
	m_strLogPath = g_key_file_get_string(conf, "Server", "LogPath", NULL);
	m_bDebug = g_key_file_get_boolean(conf, "Server", "RunType", NULL);
	m_nThreadNum = g_key_file_get_integer(conf, "Server", "ThreadNum", NULL);
	g_key_file_free(conf);
}


CServerConfig::~CServerConfig()
{
}
