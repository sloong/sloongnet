#ifndef SOCKINFO_H
#define SOCKINFO_H

#include<queue>
#include <string>
#include <mutex>
using std::string;
using std::mutex;
using std::vector;
using std::queue;
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
		const char* pExBuffer;
		int nExSize;
        int nSent;  // is send
    }SENDINFO;

	using namespace Universal;
	class CSockInfo
	{
	private:
		CSockInfo(){}
	public:
		CSockInfo( int nPriorityLevel );
		~CSockInfo();

		queue<string>* m_pReadList;
        queue<SENDINFO*>* m_pSendList; // the send list of the bytes.

		string m_Address;
		int m_nPort;
		time_t m_ConnectTime;
		int m_sock;
		CLuaPacket* m_pUserInfo;
        mutex m_oReadMutex;
        mutex m_oSendMutex;
		int m_nPriorityLevel;
		int m_nLastSentTags;
	};

}

#endif // SOCKINFO_H
