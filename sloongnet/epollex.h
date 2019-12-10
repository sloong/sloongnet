#ifndef CEPOLLEX_H
#define CEPOLLEX_H

#include <sys/epoll.h>

#include "IObject.h"
typedef unsigned char byte;

namespace Sloong
{	
	typedef std::function<ResultEnum(int)> EpollEventHandlerFunc;
	class CEpollEx : IObject
	{
	public:
        CEpollEx();
		virtual ~CEpollEx();
        void Initialize(IControl* iM);
		void Run(int nPort, int nWorkThreadNum);
		void Exit();

		void AddMonitorSocket(int socket);
		void SetEventHandler(EpollEventHandlerFunc,EpollEventHandlerFunc,EpollEventHandlerFunc,EpollEventHandlerFunc);
        
		void MonitorSendStatus(int socket);
		void UnmonitorSendStatus(int socket);

	protected:
		/// 设置socket到非阻塞模式
		int SetSocketNonblocking(int socket);

		/// 修改socket的epoll监听事件
		void CtlEpollEvent(int opt, int sock, int events);
		
		// event function
		void CloseConnectEventHandler(SmartEvent event);

		void MainWorkLoop(SMARTER params);

	protected:
		EpollEventHandlerFunc OnDataCanReceive = nullptr;
		EpollEventHandlerFunc OnNewAccept = nullptr;
		EpollEventHandlerFunc OnCanWriteData = nullptr;
		EpollEventHandlerFunc OnOtherEventHappened = nullptr;
		
	protected:
		int     m_ListenSock;
		int 	m_EpollHandle;
		//struct epoll_event m_Event;
		epoll_event m_Events[1024];
	public:
		
		bool m_bIsRunning;
	};
}


#endif // CEPOLLEX_H
