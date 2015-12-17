#ifndef SOCKINFO_H
#define SOCKINFO_H

#include<queue>
using namespace std;
#include <string>
#include <mutex>
using std::string;
using std::mutex;
namespace Sloong
{
	namespace Universal
	{
		class CLuaPacket;
	}
    typedef struct _SendInfo
    {
        const char* pSendBuffer;
        int nSize;
        int nSent;  // is send
    }SENDINFO;

	using namespace Universal;
	class CSockInfo
	{
	public:
		CSockInfo();
		~CSockInfo();

		queue<string> m_ReadList;
        queue<SENDINFO*> m_SendList; // the send list of the bytes.

		string m_Address;
		int m_nPort;
		time_t m_ConnectTime;
		int m_sock;
		CLuaPacket* m_pUserInfo;
        mutex m_oReadMutex;
        mutex m_oSendMutex;
	};

}

#endif // SOCKINFO_H
