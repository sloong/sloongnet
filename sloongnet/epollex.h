#ifndef CEPOLLEX_H
#define CEPOLLEX_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "sockinfo.h"
#include "structs.h"
using namespace std; //std 命名空间
typedef unsigned char byte;

namespace Sloong
{
	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;
	class CEpollEx
	{
	public:
        CEpollEx();
		virtual ~CEpollEx();
        int Initialize(CLog* pLog,int listenPort, int nThreadNum, int nPriorityLevel, bool bSwiftNumSupprot, bool bMD5Support, 
				int nTimeout, int nTimeoutInterval, int nRecvTimeout, int nCheckTimeout,string strCheckKey);
		void SetLogConfiguration(bool bShowSendMessage, bool bShowReceiveMessage);
		void Exit();
        void SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData = NULL, int nSize = 0 );
        bool SendMessageEx( int sock, int nPriority, const char* pData, int nSize);
        void SetEvent( condition_variable* pCV );
		void ProcessPrepareSendList( CSockInfo* info );
		/************************************************************************/
		/* If need listen write event, return false, else return true
		*/
		/************************************************************************/
		bool ProcessSendList(CSockInfo* pInfo);
		//////////////////////////////////////////////////////////////////////////
		// Add for #20 [https://git.sloong.com/public/sloongnet/issues/20] 
		// 在下面的CloseConnect函数中，将只关闭socket连接和发送event，并不删除相对应的信息
		// 直到外层监听该事件的处理者处理之后调用该函数才移除对应的信息
		//////////////////////////////////////////////////////////////////////////
		void CloseSocket(int socket);
	protected:
		int SetSocketNonblocking(int socket);
		void CtlEpollEvent(int opt, int sock, int events);
		// close the connected socket and remove the resources.
		void CloseConnect(int socket);
		void AddToSendList(int socket, int nPriority, const char* pBuf, int nSize, int nStart, const char* pExBuf, int nExSize);
		
		// event function
		void OnNewAccept();
		void OnDataCanReceive( int nSocket );
		void OnCanWriteData( int nSocket );
	public:
		static void* WorkLoop(void* params);
		static void* CheckTimeoutConnect(void* params);
	protected:
		int     m_ListenSock;
		int 	m_EpollHandle;
		//struct epoll_event m_Event;
		epoll_event m_Events[1024];
		CLog*		m_pLog;
	public:
		map<int, CSockInfo*> m_SockList;
		queue<EventListItem> m_EventSockList;
        mutex m_oEventListMutex;
        mutex m_oSockListMutex;
		int m_nPriorityLevel;
        bool m_bShowSendMessage;
		bool m_bShowReceiveMessage;
		bool m_bIsRunning;
		bool m_bSwiftNumberSupport;
		bool m_bMD5Support;
		bool m_bEnableClientCheck;
		string m_strClientCheckKey;
		int m_nCheckKeyLength;
		// client check timeout num.
		int m_nClientCheckTime;
		int m_nConnectTimeout;
		int m_nReceiveTimeout;
		int m_nTimeoutInterval;
		mutex m_oExitMutex;
		condition_variable m_oExitCV;
        condition_variable* m_pEventCV;
	};
}


#endif // CEPOLLEX_H
