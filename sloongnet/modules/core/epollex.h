/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-08 10:59:47
 * @Description: file content
 */
#ifndef CEPOLLEX_H
#define CEPOLLEX_H

#include <sys/epoll.h>

#include "IObject.h"
#include "core.h"
typedef unsigned char byte;

namespace Sloong
{	
	typedef std::function<ResultType(int)> EpollEventHandlerFunc;
	class CEpollEx : IObject
	{
	public:
        CEpollEx();
		virtual ~CEpollEx();
        CResult Initialize(IControl*);
		CResult Run();
		void Exit();

		void AddMonitorSocket(int);
		void DeleteMonitorSocket(int);

		void MonitorSendStatus(int);
		void UnmonitorSendStatus(int);

		void SetEventHandler(EpollEventHandlerFunc,EpollEventHandlerFunc,EpollEventHandlerFunc,EpollEventHandlerFunc);
	protected:
		/// 设置socket到非阻塞模式
		int SetSocketNonblocking(int);
		TResult<int> CreateListenSocket( const string&, int);

		/// 修改socket的epoll监听事件
		void CtlEpollEvent(int, int, int);
		
		// event function
		void CloseConnectEventHandler(IEvent*);

		void MainWorkLoop();

	protected:
		EpollEventHandlerFunc OnDataCanReceive = nullptr;
		EpollEventHandlerFunc OnNewAccept = nullptr;
		EpollEventHandlerFunc OnCanWriteData = nullptr;
		EpollEventHandlerFunc OnOtherEventHappened = nullptr;
		
	protected:
		int 	m_EpollHandle;
		map_ex<int,thread::id> m_mapAcceptSocketToPID;
		mutex	m_acceptMutex;
		//struct epoll_event m_Event;
		epoll_event m_Events[1024];
	public:
		
		RUN_STATUS m_emStatus;
	};
}


#endif // CEPOLLEX_H
