#ifndef SOCKINFO_H
#define SOCKINFO_H

#include<queue>
using namespace std;
#include <string>
using std::string;
namespace Sloong
{
	namespace Universal
	{
		class CLuaPacket;
	}
	using namespace Universal;
	class CSockInfo
	{
	public:
		CSockInfo();
		~CSockInfo();

		queue<string> m_ReadList;
		queue<string> m_WriteList;
		string m_Address;
		int m_nPort;
		time_t m_ConnectTime;
		int m_sock;
		CLuaPacket* m_pUserInfo;
	};

}

#endif // SOCKINFO_H
