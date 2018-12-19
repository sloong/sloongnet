#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>
using namespace std;

#include <glib.h>

#include "defines.h"

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
		GError* m_pErr = nullptr;
		GKeyFile* m_pFile = nullptr;
		GKeyFile* m_pExFile = nullptr;
		string m_strConfigPath = "";
		string m_strExConfigPath = "";
		bool m_bExConfig = false;

	public:
		// Lua script config
		LuaScriptConfigInfo m_oLuaConfigInfo;
		// Log
		LogConfigInfo m_oLogInfo;

		// Server config
		int m_nPort = 9009;		
		bool m_bEnableSSL = false;
		string m_strCertFile = "";
		string m_strKeyFile = "";
		string m_strPasswd = "";
		int m_nPriorityLevel = 0;
		bool m_bEnableSwiftNumberSup = false;
		bool m_bEnableMD5Check = false;
		int m_nConnectTimeout = 2;
		int m_nReceiveTimeout = 20;

		// Security
		int m_nClientCheckTime = 0;
		string m_strClientCheckKey = "";
		

		// Performance
		int m_nLuaProcessThreadQuantity = 10;
		int m_nMessageCenterThreadQuantity = 5;
		int m_nEPoolThreadQuantity = 3;
		int m_nTimeoutInterval = 5;

		// Data receive defines
		bool m_bEnableDataReceive = false;
		int m_nDataReceivePort = 0;
		int m_nRecvDataTimeout = 180;
		int m_nWaitRecvTimeout = 20;
	};
}



#endif
