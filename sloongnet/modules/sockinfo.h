#ifndef SOCKINFO_H
#define SOCKINFO_H

#include <queue>
#include <string>
#include <mutex>
#include <memory>
#include "lconnect.h"
#include "defines.h"
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
    class CSendInfo
    {
		public:
		CSendInfo(){};
		~CSendInfo(){
			SAFE_DELETE_ARR(pSendBuffer);
			SAFE_DELETE_ARR(pExBuffer);
		}
        const char* pSendBuffer=nullptr;
        int nSize=0;
		const char* pExBuffer=nullptr;
		int nExSize=0;
        int nSent=0;  // is send
		int nPackSize=0;
    };

    typedef struct _PrepareSendInfo
    {
        shared_ptr<CSendInfo> pSendInfo;
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

        queue<shared_ptr<CSendInfo>>* m_pSendList; // the send list of the bytes.
        queue<PRESENDINFO> m_oPrepareSendList;

		string m_Address;
		int m_nPort;
		time_t m_ActiveTime;
		shared_ptr<lConnect> m_pCon;

		unique_ptr<CLuaPacket> m_pUserInfo;
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
