#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>
#include "structs.h"
using namespace std;
namespace Sloong
{
	class CServerConfig
	{
	public:
		CServerConfig();
		~CServerConfig();

		bool LoadConfigFile(string path);

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
