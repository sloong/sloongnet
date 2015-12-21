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
		int m_nThreadNum;
		string m_strScriptFolder;
	};
}



#endif
