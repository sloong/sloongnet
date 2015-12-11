#ifndef CEPOLLEX_H
#define CEPOLLEX_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <map>
#include <queue>
#include "sockinfo.h"
using namespace std; //std 命名空间


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
		CEpollEx(CLog* pLog);
		virtual ~CEpollEx();
		int Initialize(int listenPort, int nThreadNum);
		void SendMessage(int sock, string msg);
	protected:
		int SetSocketNonblocking(int socket);
		void CtlEpollEvent(int opt, int sock, int events);

	public:
		static void* WorkLoop(void* params);
		static bool SendEx(int sock, string msg, bool eagagin = false);
	protected:
		int     m_ListenSock;
		int 	m_EpollHandle;
		//struct epoll_event m_Event;
		epoll_event m_Events[1024];
		CLog*		m_pLog;
	public:
		map<int, CSockInfo*> m_SockList;
		queue<int> m_EventSockList;
	};
}


#endif // CEPOLLEX_H
