#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>
using namespace std;
namespace Sloong
{
	class CServerConfig
	{
	public:
		CServerConfig();
		~CServerConfig();

		bool LoadConfigFile(string path);

		int m_nPort;
		string m_strLogPath;
		bool m_bDebug;
		int m_nProcessThreadQuantity;
		int m_nEPoolThreadQuantity;
		int m_nPriorityLevel;
		string m_strScriptFolder;
		int m_nSleepInterval;
	};
}



#endif
