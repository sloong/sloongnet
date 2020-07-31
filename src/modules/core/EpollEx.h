/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-11-12 15:56:50
 * @LastEditTime: 2020-07-31 15:00:04
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/EpollEx.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#pragma once

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
		CResult Initialize(IControl *);
		CResult Run();
		void Exit();

		/*** 
		 * @description: Registe a socket to epoll with data can receive event(EPOLLIN).
		 * @param: 
		 * 		1 > Socket handle.
		 */
		void AddMonitorSocket(SOCKET);

		/*** 
		 * @description: Unregiste the epoll socket.
		 * @param:
		 * 		1 > Socket handle.
		 */
		void DeleteMonitorSocket(SOCKET);

		/*** 
		 * @description: Monitor the socket cannot send status in epoll.
		 * @param:
		 * 		1 > Socket handle.
		 */
		void MonitorSendStatus(SOCKET);

		/*** 
		 * @description: Cancel the send status monitor.
		 * @param:
		 * 		1 > Socket handle.
		 */
		void UnmonitorSendStatus(SOCKET);

		/*** 
		 * @description: Set event callback function. Epoll will call it in epoll work thread when the event is happened.
		 * @param:
		 * 		1 > Accept new connection function.
		 * 		2 > Data can receive function.
		 * 		3 > Can send data function.
		 * 		4 > Other event function.
		 */
		inline void SetEventHandler(EpollEventHandlerFunc accept, EpollEventHandlerFunc recv, EpollEventHandlerFunc send, EpollEventHandlerFunc other)
		{
			OnNewAccept = accept;
			OnCanWriteData = send;
			OnDataCanReceive = recv;
			OnOtherEventHappened = other;
		}

	protected:
		/*** 
		 * @description: Set socket to no blocking mode.
		 * @param：
		 * 		1 > the socket handle.
		 * @return: 
		 * 		0 for succeed, -1 for erros. it's same as setsockopt function.
		 */
		int SetSocketNonblocking(SOCKET);

		/*** 
		 * @description: Create new listen socket.
		 * @param: 
		 * 		1 > address for listen socket bind.
		 * 		2 > listen port.
		 * @return: 
		 * 		New listen socket handle in TResult<int>
		 */
		NResult CreateListenSocket(const string &, int);

		/*** 
		 * @description: Control socket in epoll operation.
		 * @param:
		 * 		1 > Valid opcodes ( "op" parameter ) to issue to epoll_ctl() 
		 * 			EPOLL_CTL_ADD/EPOLL_CTL_DEL/EPOLL_CTL_MOD
		 * 		2 > Socket handle for contorled
		 * 		3 > Enum for epoll events. 
		 * 			EPOLL_EVENTS
		 */
		void CtlEpollEvent(int, SOCKET, int);
		void MainWorkLoop();

	protected:
		EpollEventHandlerFunc OnDataCanReceive = nullptr;
		EpollEventHandlerFunc OnNewAccept = nullptr;
		EpollEventHandlerFunc OnCanWriteData = nullptr;
		EpollEventHandlerFunc OnOtherEventHappened = nullptr;

	protected:
		int m_EpollHandle;
		map_ex<int, thread::id> m_mapAcceptSocketToPID;
		mutex m_acceptMutex;
		//struct epoll_event m_Event;
		epoll_event m_Events[1024];

	public:
		RUN_STATUS m_emStatus;
	};
} // namespace Sloong
