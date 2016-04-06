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

		bool Initialize(string path);
		void LoadConfig();

	public: 
		// Load string config, if load error, the value will the origin value. if succeed, the value will change to config.
		static string GetStringConfig(string strSection, string strKey, string strDef, bool bThrowWhenFialed = false );
		static bool GetBoolenConfig(string strSection, string strKey, bool& bDef, bool bThrowWhenFialed = false);
		static int GetIntConfig(string strSection, string strKey, int& nDef, bool bThrowWhenFialed = false);
		static CServerConfig* g_pThis;

	private:
		GError* m_pErr;
		GKeyFile* m_pFile;
		string m_strConfigPath;
	public:
		// DB config
		MySQLConnectInfo m_oConnectInfo;

		// Server config
		int m_nPort;
		string m_strLogPath;
		bool m_bDebug;
		int m_nProcessThreadQuantity;
		int m_nEPoolThreadQuantity;
		int m_nPriorityLevel;
		string m_strScriptFolder;
		int m_nSleepInterval;
		bool m_bEnableSwiftNumberSup;
		bool m_bEnableMD5Check;

		// Message show/hide
		bool m_bShowSendMessage;
		bool m_bShowReceiveMessage;
		bool m_bLogWriteToOneFile;
	};
}



#endif
