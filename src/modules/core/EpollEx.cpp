/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-11-12 15:56:50
 * @LastEditTime: 2021-10-22 12:38:45
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/EpollEx.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: Epoll extend object. 
 */

// load system file
#include "EpollEx.h"
#include "EasyConnect.h"
#include "ConnectSession.h"
#include "IData.h"
#include "utility.h"
// System file
#include <arpa/inet.h>
#include <netdb.h>
// for TCP_NODELAY
#include <netinet/tcp.h>

Sloong::CEpollEx::CEpollEx()
{
	m_emStatus = RUN_STATUS::Created;
}

Sloong::CEpollEx::~CEpollEx()
{
}

U64Result Sloong::CEpollEx::CreateListenSocket(const string &addr, int port)
{
	// 初始化socket
	auto listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	int sock_op = 1;
	// SOL_SOCKET:在socket层面设置
	// SO_REUSEADDR:允许套接字和一个已在使用中的地址捆绑
	if (0 != setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &sock_op, sizeof(sock_op)))
		return U64Result::Make_Error(format("Set socket property to [SO_REUSEADDR] field. Error info: [{}]{}", errno, strerror(errno)));

#ifdef SO_REUSEPORT
	if (0 != setsockopt(listen_sock, SOL_SOCKET, SO_REUSEPORT, &sock_op, sizeof(sock_op)))
		return U64Result::Make_Error(format("Set socket property to [SO_REUSEPORT] field. Error info: [{}]{}", errno, strerror(errno)));
#endif

	// 初始化地址结构
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(addr.c_str());
	address.sin_port = htons((uint16_t)port);

	// 设置socket为非阻塞模式
	SetSocketNonblocking(listen_sock);

	// 绑定端口
	if (-1 == bind(listen_sock, (struct sockaddr *)&address, sizeof(address)))
		return U64Result::Make_Error(format("Bind to {} field. Error info: [{}]{}", port, errno, strerror(errno)));

	// 监听端口,定义的SOMAXCONN大小为128,太小了,这里修改为1024
	if (-1 == listen(listen_sock, 1024))
		return U64Result::Make_Error(format("Listen to {} field. Error info: [{}]{}", port, errno, strerror(errno)));

	return U64Result::Make_OKResult(listen_sock);
}

// Initialize the epoll and the thread pool.
CResult Sloong::CEpollEx::Initialize(IControl *iMsg)
{
	IObject::Initialize(iMsg);
	int nPort = IData::GetGlobalConfig()->listenport();
	m_pLog->info(format("epollex is initialize.license port is {}", nPort));

	// For Test the port cannot used. when test done, close it.
	auto nRes = CreateListenSocket("0.0.0.0", nPort);
	if (nRes.IsFialed())
		return CResult(nRes.GetResult(), nRes.GetMessage());
	else
	{
		shutdown(nRes.GetResultObject(), SHUT_RDWR);
		close(nRes.GetResultObject());
	}

	int workThread = IData::GetGlobalConfig()->epollthreadquantity();
	if (workThread < 1)
		return CResult::Make_Error("Epoll work thread must be big than 0");

	m_pLog->info(format("epollex is running with {} threads", workThread));

#ifdef SO_REUSEPORT
	m_pLog->debug("System support SO_REUSEPORT. epoll will run with SO_REUSEPORT mode.");
#endif

	m_EpollHandle = epoll_create(65535);

	// Init the thread pool
	ThreadPool::AddWorkThread(std::bind(&CEpollEx::MainWorkLoop, this), workThread);
	m_emStatus = RUN_STATUS::Running;
	
	return CResult::Succeed;
}

CResult Sloong::CEpollEx::Run()
{
	return CResult::Succeed;
}


void Sloong::CEpollEx::RegisterConnection( EasyConnect* conn )
{
	conn->SetOnReconnectCallback([&](uint64_t id, int old_sock, int cur_sock){
		if( m_mapSocketToID.exist(old_sock) ) 
		{
			DeleteMonitorSocket(old_sock);
			m_mapSocketToID.erase(old_sock);
		}
		
		AddMonitorSocket(cur_sock);
		m_mapSocketToID.insert(cur_sock,id);
	});
	AddMonitorSocket(conn->GetSocketID());
	m_mapSocketToID.insert(conn->GetSocketID(),conn->GetHashCode());
	m_mapIDToConnection.insert(conn->GetHashCode(),conn);
}

void Sloong::CEpollEx::UnregisteConnection( uint64_t id )
{
	if( !m_mapIDToConnection.exist(id) )	
		return;

	auto conn = m_mapIDToConnection.get(id);
	if( conn != nullptr )
	{
		DeleteMonitorSocket(conn->GetSocketID());
		m_mapSocketToID.erase(conn->GetSocketID());
	}
	m_mapIDToConnection.erase(id);
}

void Sloong::CEpollEx::ModifySendMonitorStatus( uint64_t id, bool monitor )
{
	if( !m_mapIDToConnection.exist(id) )	
		return;
	auto sock = m_mapIDToConnection.get(id)->GetSocketID();
	if( monitor )
	{
		MonitorSendStatus(sock);
	}
	else
	{
		UnmonitorSendStatus(sock);
	}
}


void Sloong::CEpollEx::AddMonitorSocket(SOCKET nSocket)
{
	SetSocketNonblocking(nSocket);
	CtlEpollEvent(EPOLL_CTL_ADD, nSocket, EPOLLIN);
}

void Sloong::CEpollEx::DeleteMonitorSocket(SOCKET nSocket)
{
	CtlEpollEvent(EPOLL_CTL_DEL, nSocket, 0);
}

void Sloong::CEpollEx::MonitorSendStatus(SOCKET socket)
{
	CtlEpollEvent(EPOLL_CTL_MOD, socket, EPOLLIN | EPOLLOUT);
}

void Sloong::CEpollEx::UnmonitorSendStatus(SOCKET socket)
{
	CtlEpollEvent(EPOLL_CTL_MOD, socket, EPOLLIN);
}

void Sloong::CEpollEx::CtlEpollEvent(int opt, SOCKET sock, int events)
{
	struct epoll_event ent;
	memset(&ent, 0, sizeof(ent));
	ent.data.fd = sock;
	// LT模式时，事件就绪时，假设对事件没做处理，内核会反复通知事件就绪	  	EPOLLLT
	// ET模式时，事件就绪时，假设对事件没做处理，内核不会反复通知事件就绪  	EPOLLET
	ent.events = events | EPOLLERR | EPOLLHUP | EPOLLET;

	epoll_ctl(m_EpollHandle, opt, sock, &ent);
}

// 设置套接字为非阻塞模式
int Sloong::CEpollEx::SetSocketNonblocking(SOCKET socket)
{
	int op;

	op = fcntl(socket, F_GETFL, 0);
	fcntl(socket, F_SETFL, op | O_NONBLOCK);

	int enable = 1;
	op = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));

	return op;
}

/*************************************************
* Function: * epoll_loop
* Description: * epoll检测循环
* Input: *
* Output: *
* Others: *
*************************************************/
void Sloong::CEpollEx::MainWorkLoop()
{
	auto pid = this_thread::get_id();
	string spid = Helper::ntos(pid);
	
	m_pLog->info("epoll work thread is started. PID:" + spid);
	int port = IData::GetGlobalConfig()->listenport();
	auto res = CreateListenSocket("0.0.0.0", port);
	if (res.IsFialed())
	{
		m_pLog->critical(res.GetMessage());
		m_iC->SendMessage(EVENT_TYPE::ProgramStop);
		return;
	}
	int sock = res.GetResultObject();
	unique_lock<mutex> lock(m_acceptMutex);
	m_mapAcceptSocketToPID.insert(sock,pid);
	lock.unlock();

	// 创建epoll事件对象
	CtlEpollEvent(EPOLL_CTL_ADD, sock, EPOLLIN | EPOLLOUT);

	while (m_emStatus == RUN_STATUS::Created)
	{
		this_thread::sleep_for(std::chrono::microseconds(10));
	}

	int n, i;
	while (m_emStatus == RUN_STATUS::Running)
	{
		try
		{
			// 返回需要处理的事件数
			n = epoll_wait(m_EpollHandle, m_Events, 1024, 500);

			if (n <= 0)
				continue;

			for (i = 0; i < n; ++i)
			{
				int fd = m_Events[i].data.fd;
				if (m_mapAcceptSocketToPID.exist(fd))
				{
					m_pLog->debug("EPoll Accept event happened.");
					// accept the connect and add it to the list
					int conn_sock = -1;
					do
					{
						conn_sock = accept(fd, NULL, NULL);
						if (conn_sock == -1)
						{
							if (errno == EAGAIN)
								m_pLog->debug("Accept end.");
							else
								m_pLog->warn("Accept error.");
							continue;
						}
						auto res = OnNewAccept(conn_sock);
						if (res == ResultType::Error)
						{
							shutdown(conn_sock, SHUT_RDWR);
							close(conn_sock);
						}
					} while (conn_sock > 0);
				}
				// EPOLLIN 可读消息
				else if (m_Events[i].events & EPOLLIN)
				{
					if( !m_mapSocketToID.exist(fd))
					{
						m_pLog->error(format("EPoll EPOLLIN event happened. Socket[{}][{}] Data Can Receive.", fd, CUtility::GetSocketAddress(fd)));
					}
					else
					{
						auto id = m_mapSocketToID.get(fd);
						m_pLog->debug(format("EPoll EPOLLIN event happened. Socket[{}][{}] Data Can Receive.", fd, CUtility::GetSocketAddress(fd)));
						auto res = OnDataCanReceive(id);
						if (res == ResultType::Error)
							UnregisteConnection(id);
					}
				}
				// EPOLLOUT 可写消息
				else if (m_Events[i].events & EPOLLOUT)
				{
					
					if( !m_mapSocketToID.exist(fd))
					{
						m_pLog->error(format("EPoll EPOLLOUT event happened. Socket[{}][{}] Data Can Receive.", fd, CUtility::GetSocketAddress(fd)));
					}
					else
					{
						auto id = m_mapSocketToID.get(fd);
						m_pLog->debug(format("EPoll EPOLLOUT event happened.Socket[{}][{}] Can Write Data.", fd, CUtility::GetSocketAddress(fd)));
						auto res = OnCanWriteData(id);
						// 所有消息全部发送完毕后只需要监听可读消息就可以了。
						if (res == ResultType::Succeed)
							ModifySendMonitorStatus(id,false);
					}
				}
				else
				{
					
					if( !m_mapSocketToID.exist(fd))
					{
						m_pLog->error(format("EPoll EPOLLIN event happened. Socket[{}][{}] Data Can Receive.", fd, CUtility::GetSocketAddress(fd)));
					}
					else
					{
						auto id = m_mapSocketToID.get(fd);
						m_pLog->debug(format("EPoll unkuown event happened. Socket[{}][{}] close this connnect.", fd, CUtility::GetSocketAddress(fd)));
						OnOtherEventHappened(id);
					}
				}
			}
		}
		catch (exception &ex)
		{
			m_pLog->error(format("Error happened in Epoll work thead. message:", ex.what()));
		}
		catch (...)
		{
			m_pLog->error("Unkown error happened in Epoll work thead.");
		}
	}
	m_pLog->info(format("epoll work thread[{}] is exit " , spid));
}

void Sloong::CEpollEx::Exit()
{
	m_emStatus = RUN_STATUS::Exit;
}
