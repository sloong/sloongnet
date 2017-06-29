#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>
#include "structs.h"
using namespace std;
#include <glib.h>
namespace Sloong
{
	class CServerConfig
	{
	public:
		CServerConfig();
		~CServerConfig();

		bool Initialize(string path, string exConfig="");
		void LoadConfig();

	public: 
		// Load string config, if load error, the value will the origin value. if succeed, the value will change to config.
		static string GetStringConfig(string strSection, string strKey, string strDef, bool bThrowWhenFialed = false );
		static bool GetBoolenConfig(string strSection, string strKey, bool bDef, bool bThrowWhenFialed = false);
		static int GetIntConfig(string strSection, string strKey, int nDef, bool bThrowWhenFialed = false);
		static CServerConfig* g_pThis;

	private:
		GError* m_pErr;
		GKeyFile* m_pFile;
		GKeyFile* m_pExFile;
		string m_strConfigPath;
		string m_strExConfigPath;
		bool m_bExConfig;
	public:
		// DB config
		MySQLConnectInfo m_oConnectInfo;
		// Lua script config
		LuaScriptConfigInfo m_oLuaConfigInfo;
		// Log
		LogConfigInfo m_oLogInfo;

		// Server config
		int m_nPort;
		int m_nPriorityLevel;
		bool m_bEnableSwiftNumberSup;
		bool m_bEnableMD5Check;
		int m_nConnectTimeout;
		int m_nReceiveTimeout;

		// Security
		int m_nClientCheckTime;
		string m_strClientCheckKey;

		// Performance
		int m_nSleepInterval;
		int m_nProcessThreadQuantity;
		int m_nEPoolThreadQuantity;
		int m_nTimeoutInterval;
	};
}



#endif
