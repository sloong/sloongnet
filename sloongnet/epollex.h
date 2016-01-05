#ifndef CEPOLLEX_H
#define CEPOLLEX_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <map>
#include <queue>
#include <mutex>
#include "sockinfo.h"
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
        int Initialize(CLog* pLog,int listenPort, int nThreadNum, int nPriorityLevel);
        void SendMessage(int sock, int nPriority, const string& nSwift, string msg, const char* pExData = NULL, int nSize = 0 );
        bool SendMessageEx( int sock, int nPriority, const char* pData, int nSize);
	protected:
		int SetSocketNonblocking(int socket);
		void CtlEpollEvent(int opt, int sock, int events);
		// close the connected socket and remove the resources.
		void CloseConnect(int socket);
		void AddToSendList(int socket, int nPriority, const char* pBuf, int nSize, int nStart, const char* pExBuf, int nExSize);
	public:
		static void* WorkLoop(void* params);
        static int SendEx(int sock, const char* buf, int nSize, int nStart, bool eagain = false);
        static int RecvEx( int sock, char** buf, int nSize, bool eagain = false );
	protected:
		int     m_ListenSock;
		int 	m_EpollHandle;
		//struct epoll_event m_Event;
		epoll_event m_Events[1024];
		CLog*		m_pLog;
	public:
		map<int, CSockInfo*> m_SockList;
		queue<int> m_EventSockList;
        mutex m_oEventListMutex;
        mutex m_oSockListMutex;
		int m_nPriorityLevel;
	};
}


#endif // CEPOLLEX_H
