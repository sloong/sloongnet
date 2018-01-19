#ifndef SOCKINFO_H
#define SOCKINFO_H

#include <queue>
#include <string>
#include <mutex>
#include <memory>
#include "lconnect.h"
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

    typedef struct _PrepareSendInfo
    {
        SENDINFO* pSendInfo;
        int nPriorityLevel;
    }PRESENDINFO;

	typedef struct _stRecvInfo
	{
		long long nSwiftNumber = -1;
		string strMD5 = "";
		string strMessage = "";
	}RECVINFO;

	using namespace Universal;
	class CSockInfo
	{
	private:
		CSockInfo(){}
	public:
		CSockInfo( int nPriorityLevel );
		~CSockInfo();

		queue<RECVINFO>* m_pReadList;
        queue<SENDINFO*>* m_pSendList; // the send list of the bytes.
        queue<PRESENDINFO>* m_pPrepareSendList;

		string m_Address;
		int m_nPort;
		time_t m_ActiveTime;
		shared_ptr<lConnect> m_pCon;

		//unique_ptr<CLuaPacket> m_pUserInfo;
		CLuaPacket* m_pUserInfo;
        mutex m_oSockReadMutex;
        mutex m_oSockSendMutex;
		mutex m_oReadListMutex;
		mutex m_oSendListMutex;
        mutex m_oPreSendMutex;
		// for the process thread. in epoll thread, it always add the event sock to event list. so in process thread
		// maybe happed two thread process one socket. 
		// this mutex number is base on Priority level numbers.
		mutex* m_pProcessMutexList;
		int m_nPriorityLevel;
		int m_nLastSentTags = -1;
        bool m_bIsSendListEmpty = true;
	};

}

#endif // SOCKINFO_H
