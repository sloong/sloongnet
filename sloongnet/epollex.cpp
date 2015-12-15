#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <queue>
#include "epollex.h"
#include "utility.h"
#include <univ/log.h>
#include <univ/univ.h>
#include <univ/threadpool.h>
#define MAXRECVBUF 4096
#define MAXBUF MAXRECVBUF+10

using namespace Sloong;
using namespace Sloong::Universal;

CEpollEx::CEpollEx()
{
}

CEpollEx::~CEpollEx()
{
}

void on_sigint(int signal)
{
    exit(0);
}

// Initialize the epoll and the thread pool.
int CEpollEx::Initialize( CLog* plog, int licensePort, int nThreadNum )
{
    m_pLog = plog;
	m_pLog->Log(CUniversal::Format("epollex is initialize.license port is %d", licensePort));
    //SIGPIPE:在reader终止之后写pipe的时候发生
    //SIG_IGN:忽略信号的处理程序
    //SIGCHLD: 进程Terminate或Stop的时候,SIGPIPE会发送给进程的父进程,缺省情况下该Signal会被忽略
    //SIGINT:由Interrupt Key产生,通常是Ctrl+c或者Delete,发送给所有的ForeGroundGroup进程.
    signal(SIGPIPE,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGINT,&on_sigint);

    // 初始化socket
    m_ListenSock=socket(AF_INET,SOCK_STREAM,0);
    int sock_op = 1;
    // SOL_SOCKET:在socket层面设置
    // SO_REUSEADDR:允许套接字和一个已在使用中的地址捆绑
    setsockopt(m_ListenSock,SOL_SOCKET,SO_REUSEADDR,&sock_op,sizeof(sock_op));

    // 初始化地址结构
    struct sockaddr_in address;
    memset(&address,0,sizeof(address));
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=htons(licensePort);

    // 绑定端口
    errno = bind(m_ListenSock,(struct sockaddr*)&address,sizeof(address));
    if( errno == -1 )
        cout<<"bind to "<<licensePort<<" field. errno = "<<errno<<endl;

    // 监听端口,监听队列大小为1024.可修改为SOMAXCONN
    errno = listen(m_ListenSock,1024);
    // 设置socket为非阻塞模式
    SetSocketNonblocking(m_ListenSock);
    // 创建epoll
    m_EpollHandle=epoll_create(65535);
    // 创建epoll事件对象
    CtlEpollEvent(EPOLL_CTL_ADD,m_ListenSock,EPOLLIN|EPOLLET|EPOLLOUT);

    // Init the thread pool
	CThreadPool::AddWorkThread(WorkLoop, this, nThreadNum);
	
	return true;
}

void CEpollEx::CtlEpollEvent(int opt, int sock, int events)
{
    struct epoll_event ent;
    memset(&ent,0,sizeof(ent));
    ent.data.fd=sock;
    ent.events=events;

    // 设置事件到epoll对象
    epoll_ctl(m_EpollHandle,opt,sock,&ent);
}


// 设置套接字为非阻塞模式
int CEpollEx::SetSocketNonblocking(int socket)
{
    int op;

    op=fcntl(socket,F_GETFL,0);
    fcntl(socket,F_SETFL,op|O_NONBLOCK);

    return op;
}

/*************************************************
* Function: * epoll_loop
* Description: * epoll检测循环
* Input: *
* Output: *
* Others: *
*************************************************/
void* CEpollEx::WorkLoop(void* pParam)
{
	CEpollEx* pThis = (CEpollEx*)pParam;
	auto log = pThis->m_pLog;
	log->Log("epoll is start work loop.");
	int sockListen = pThis->m_ListenSock;
	auto& rSockList = pThis->m_SockList;
    int n,i;
    while(true)
    {
        // 返回需要处理的事件数
		n = epoll_wait(pThis->m_EpollHandle, pThis->m_Events, 1024, -1);
        if( n<=0 ) continue;

        for(i=0; i<n; ++i)
        {
            int fd =pThis->m_Events[i].data.fd;
			if (fd == sockListen)
            {
                // 新连接接入
                while(true)
                {
                    // accept the connect and add it to the list
                    int conn_sock = -1;
					while ((conn_sock = accept(sockListen, NULL, NULL)) > 0)
                    {
                        CSockInfo* info = new CSockInfo();
                        struct sockaddr_in add;
                        int nSize = sizeof(add);
                        memset(&add,0,sizeof(add));
                        getpeername(conn_sock, (sockaddr*)&add, (socklen_t*)&nSize );

                        info->m_Address = inet_ntoa(add.sin_addr);
                        info->m_nPort = add.sin_port;
                        time_t tm;
                        time(&tm);
                        info->m_ConnectTime = tm;
                        info->m_sock = conn_sock;
                        rSockList[conn_sock] = info;
						log->Log(CUniversal::Format("accept client:%s.", info->m_Address));
                        //将接受的连接添加到Epoll的事件中.
                        // Add the recv event to epoll;
                        pThis->SetSocketNonblocking(conn_sock);
                        pThis->CtlEpollEvent(EPOLL_CTL_ADD,conn_sock,EPOLLIN|EPOLLET);
                    }
                    if (conn_sock == -1) {
                        if (errno == EAGAIN )
                            break;
                        else
							log->Log("accept error.");

                    }
                }
            }
            else if(pThis->m_Events[i].events&EPOLLIN)
            {
                // 已经连接的用户,收到数据,可以开始读入
                bool bLoop = true;
                while(bLoop)
                {
                    // 先读取消息长度
                    int len = sizeof(long long);
                    int readLen;
					char dataLeng[sizeof(long long)+1] = { 0 };
                    readLen = recv(fd,dataLeng,len,0);
                    if(readLen < 0)
                    {
                        //由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读在这里就当作是该次事件已处理过。
                        if(errno == EAGAIN)
                        {
                            break;
                        }
                        else
                        {
                            // 读取错误,将这个连接从监听中移除并关闭连接
							pThis->CloseConnect(fd);
                            break;
                        }
                    }
                    else if(readLen == 0)
                    {
                        // The connect is disconnected.
						pThis->CloseConnect(fd);
                        break;
                    }
                    else
                    {
                        long dtlen = atol(dataLeng);
                        dtlen++;
                        char* data = new char[dtlen];
                        memset(data,0,dtlen);

                        readLen = recv(fd,data,dtlen,0);//一次性接受所有消息
                        if(readLen >= dtlen)
                            bLoop = true; // 需要再次读取
                        else
                            bLoop = false;

                        // Add the msg to the sock info list
                        string msg(data);
                        CSockInfo* info = rSockList[fd];
                        info->m_ReadList.push(msg);
                        delete[] data;

                        // Add the sock event to list
                        pThis->m_EventSockList.push(fd);
                    }
                }
            }
            else if(pThis->m_Events[i].events&EPOLLOUT)
            {
                // 可以写入事件
                CSockInfo* info = rSockList[fd];
                while (info->m_WriteList.size())
                {
                    string msg = info->m_WriteList.front();
                    info->m_WriteList.pop();
					log->Log(CUniversal::Format("send message %1%", msg));
                    if(!SendEx(fd,msg))
                    {
						log->Log("write error.", ERR);
                    }
                }
            }
            else
            {
				pThis->CloseConnect(fd);
            }
        }
    }
    return 0;
}
/*************************************************
* Function: * check_connect_timeout
* Description: * 检测长时间没反应的网络连接，并关闭删除
* Input: *
* Output: *
* Others: *
*************************************************/
void *check_connect_timeout(void* para)
{
    return 0;
}

void CEpollEx::SendMessage(int sock, const string& nSwift, string msg)
{
	// process msg
	string md5 = CUniversal::MD5_Encoding(msg);
	msg = md5 + "|" + nSwift + "|" + msg;
    if( false == SendEx(sock,msg,true) )
    {
        CSockInfo* info = m_SockList[sock];
        info->m_WriteList.push(msg);
        // Add to epoll list
        SetSocketNonblocking(sock);
        CtlEpollEvent(EPOLL_CTL_ADD,sock,EPOLLOUT|EPOLLET);
    }
}

bool CEpollEx::SendEx( int sock, string msg, bool eagain /* = false */ )
{
	long long len = msg.size() + 1;
	char* buf = new char[msg.size() + 8 + 1];
	memcpy(buf, (void*)&len, 8);
	memcpy(buf + 8, msg.c_str(), msg.size() + 1);
	SendEx(sock, buf, msg.size() + 8 + 1, eagain);
}

void Sloong::CEpollEx::CloseConnect(int socket)
{
	CSockInfo* info = m_SockList[socket];
	m_pLog->Log(CUniversal::Format("close connect:%s.",info->m_Address));
	CtlEpollEvent(EPOLL_CTL_DEL, socket, EPOLLIN | EPOLLOUT | EPOLLET);
	SAFE_DELETE(info);
	for (map<int, CSockInfo*>::iterator i = m_SockList.begin(); i != m_SockList.end();)
	{
		if (i->first == socket)
		{
			m_SockList.erase(i);
			break;
		}
		else
		{
			i++;
		}
	}
}

bool Sloong::CEpollEx::SendEx(int sock,const char* buf, int nSize, bool eagain /*= false*/)
{	
	int nSentSize = 0;
	int nNosendSize = nSize;
	while (nNosendSize > 0)
	{
		nSentSize = write(sock, buf + nSize - nNosendSize, nNosendSize);
		// if errno != EAGAIN or again for error and return is -1, return false
		if (nSentSize == -1 && (errno != EAGAIN || eagain == true)) 
		{
			return false;
		}

		nNosendSize -= nSentSize;
	}

	return true;
}

void Sloong::CEpollEx::SendMessage(int sock, const string& nSwift, char* msg, int nSize)
{
	SendEx(sock, msg, nSize);
}
