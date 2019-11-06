// load system file

#include "epollex.h"
#include "EasyConnect.h"
#include "sockinfo.h"
#include "NetworkEvent.hpp"


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h> 


#define MAXRECVBUF 4096
#define MAXBUF MAXRECVBUF+10

using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;


Sloong::CEpollEx::CEpollEx()
{
	m_bIsRunning = false;

}

Sloong::CEpollEx::~CEpollEx()
{
}


// Initialize the epoll and the thread pool.
void Sloong::CEpollEx::Initialize(IControl* iMsg)
{
	IObject::Initialize(iMsg);

	// 初始化socket
	m_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
	int sock_op = 1;
	// SOL_SOCKET:在socket层面设置
	// SO_REUSEADDR:允许套接字和一个已在使用中的地址捆绑
	setsockopt(m_ListenSock, SOL_SOCKET, SO_REUSEADDR, &sock_op, sizeof(sock_op));
}

void Sloong::CEpollEx::Run(int nPort, int nWorkThreadNum)
{
	m_pLog->Info(CUniversal::Format("epollex is initialize.license port is %d", nPort ));
	// 初始化地址结构
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons((uint16_t)nPort);

	// 绑定端口
	errno = bind(m_ListenSock, (struct sockaddr*)&address, sizeof(address));
	if (errno == -1)
		throw normal_except(CUniversal::Format("bind to %d field. errno = %d", nPort, errno));

	// 监听端口,监听队列大小为1024.可修改为SOMAXCONN
	errno = listen(m_ListenSock, 1024);
	if (errno == -1)
		throw normal_except(CUniversal::Format("listen to %d field. errno = %d", nPort, errno));

	// 设置socket为非阻塞模式
	SetSocketNonblocking(m_ListenSock);
	// 创建epoll
	m_EpollHandle = epoll_create(65535);
	// 创建epoll事件对象
	CtlEpollEvent(EPOLL_CTL_ADD, m_ListenSock, EPOLLIN | EPOLLOUT);
	m_bIsRunning = true;
	// Init the thread pool
	CThreadPool::AddWorkThread( std::bind(&CEpollEx::MainWorkLoop, this, std::placeholders::_1), nullptr, nWorkThreadNum);
}


void Sloong::CEpollEx::AddMonitorSocket(int nSocket)
{
	SetSocketNonblocking(nSocket);
	CtlEpollEvent(EPOLL_CTL_ADD, nSocket, EPOLLIN);
}

void Sloong::CEpollEx::SetEventHandler(EpollEventHandlerFunc accept,EpollEventHandlerFunc recv,EpollEventHandlerFunc send,EpollEventHandlerFunc other)
{
	OnNewAccept = accept;
	OnCanWriteData = send;
	OnDataCanReceive = recv;
	OnOtherEventHappened = other;
}


void Sloong::CEpollEx::MonitorSendStatus(int socket)
{
	CtlEpollEvent(EPOLL_CTL_MOD, socket, EPOLLIN | EPOLLOUT);
}

void Sloong::CEpollEx::UnmonitorSendStatus(int socket)
{
	CtlEpollEvent(EPOLL_CTL_MOD, socket, EPOLLIN );
}

void Sloong::CEpollEx::CtlEpollEvent(int opt, int sock, int events)
{
	struct epoll_event ent;
	memset(&ent, 0, sizeof(ent));
	ent.data.fd = sock;
	// LT模式时，事件就绪时，假设对事件没做处理，内核会反复通知事件就绪	  	EPOLLLT
	// ET模式时，事件就绪时，假设对事件没做处理，内核不会反复通知事件就绪  	EPOLLET
	ent.events = events | EPOLLERR | EPOLLHUP | EPOLLET;

	m_pLog->Verbos(CUniversal::Format("Control epoll opt: Socket[%d] opt[%d]",sock, opt));
	// 设置事件到epoll对象
	epoll_ctl(m_EpollHandle, opt, sock, &ent);
}


// 设置套接字为非阻塞模式
int Sloong::CEpollEx::SetSocketNonblocking(int socket)
{
	int op;

	op = fcntl(socket, F_GETFL, 0);
	fcntl(socket, F_SETFL, op | O_NONBLOCK);

	return op;
}

/*************************************************
* Function: * epoll_loop
* Description: * epoll检测循环
* Input: *
* Output: *
* Others: *
*************************************************/
void Sloong::CEpollEx::MainWorkLoop(SMARTER param)
{
	auto pid = this_thread::get_id();
	string spid = CUniversal::ntos(pid);
	m_pLog->Info("epoll work thread is running." + spid);
	int n, i;
	while (m_bIsRunning)
	{
		// 返回需要处理的事件数
		n = epoll_wait(m_EpollHandle, m_Events, 1024, 500);

		if (n <= 0)
			continue;

		for (i = 0; i < n; ++i)
		{
			int fd = m_Events[i].data.fd;
			if (fd == m_ListenSock)
			{
				m_pLog->Verbos("EPoll Accept event happened.");
				// accept the connect and add it to the list
				int conn_sock = -1;
				do
				{
					conn_sock = accept(m_ListenSock, NULL, NULL);
					if (conn_sock == -1)
					{
						if (errno == EAGAIN){
							m_pLog->Verbos("Accept end.");
						}else{
							m_pLog->Warn("Accept error.");
						}
						continue;
					}
					auto res = OnNewAccept(conn_sock);
					if( res  == NetworkResult::Error){
						shutdown(conn_sock,SHUT_RDWR);
						close(conn_sock);
					}else{
						//将接受的连接添加到Epoll的事件中.
						// Add the recv event to epoll;
						SetSocketNonblocking(conn_sock);
						// 刚接收连接，所以只关心可读状态。
						CtlEpollEvent(EPOLL_CTL_ADD, conn_sock, EPOLLIN);
					}
				}while(conn_sock > 0);
			}
			// EPOLLIN 可读消息
			else if (m_Events[i].events&EPOLLIN)
			{
				m_pLog->Verbos(CUniversal::Format("EPoll EPOLLIN event happened. Socket[%d] Data Can Receive.",fd));
				auto res = OnDataCanReceive(fd);
				if( res  != NetworkResult::Error)
					MonitorSendStatus(fd);
			}
			// EPOLLOUT 可写消息
			else if (m_Events[i].events&EPOLLOUT)
			{
				m_pLog->Verbos(CUniversal::Format("EPoll EPOLLOUT event happened.Socket[%d] Can Write Data.",fd));
				auto res = OnCanWriteData(fd);
				// 所有消息全部发送完毕后只需要监听可读消息就可以了。
				if( res == NetworkResult::Succeed)
					UnmonitorSendStatus(fd);			
				else if( res == NetworkResult::Retry )
					MonitorSendStatus(fd);			
			}
			else
			{
				m_pLog->Verbos(CUniversal::Format("EPoll unkuown event happened.Socket[%d] close this connnect.",fd));
				OnOtherEventHappened(fd);
			}
		}
	}
	m_pLog->Info("epoll work thread is exit " + spid);
}

void Sloong::CEpollEx::CloseConnectEventHandler(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto socket = net_evt->GetSocketID();
	CtlEpollEvent(EPOLL_CTL_DEL, socket, EPOLLIN | EPOLLOUT);
}


void Sloong::CEpollEx::Exit()
{
	m_bIsRunning = false;
}

