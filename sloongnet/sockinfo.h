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
		int nPackSize;
    }SENDINFO;

    typedef struct _PrepareSendInfo
    {
        SENDINFO* pSendInfo;
        int nPriorityLevel;
    }PRESENDINFO;


	using namespace Universal;
	class CSockInfo
	{
	private:
		CSockInfo(){}
	public:
		CSockInfo( int nPriorityLevel );
		~CSockInfo();

        queue<SENDINFO*>* m_pSendList; // the send list of the bytes.
        queue<PRESENDINFO>* m_pPrepareSendList;

		string m_Address;
		int m_nPort;
		time_t m_ActiveTime;
		shared_ptr<lConnect> m_pCon;

		unique_ptr<CLuaPacket> m_pUserInfo;
		//CLuaPacket* m_pUserInfo;
        mutex m_oSockReadMutex;
        mutex m_oSockSendMutex;
		mutex m_oSendListMutex;
        mutex m_oPreSendMutex;
		int m_nPriorityLevel;
		int m_nLastSentTags = -1;
        bool m_bIsSendListEmpty = true;
	};

}

#endif // SOCKINFO_H
