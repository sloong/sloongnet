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
#include "univ/log.h"
#include "univ/univ.h"
#define MAXRECVBUF 4096
#define MAXBUF MAXRECVBUF+10

CEpollEx* CEpollEx::g_pThis = NULL;

CEpollEx::CEpollEx()
{
    g_pThis = this;
    CLog::showLog(INF,"epollex is build.");
}

CEpollEx::~CEpollEx()
{
}

void on_sigint(int signal)
{
    exit(0);
}

// Initialize the epoll and the thread pool.
int CEpollEx::Initialize(int nThreadNums, int licensePort)
{
    CLog::showLog(INF,boost::format("epollex is initialize.license port is %d")%licensePort);
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

    // 创建线程池
    return InitThreadPool(nThreadNums);
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


/*************************************************
* Function: * init_thread_pool
* Description: * 初始化线程
* Input: * threadNum:用于处理epoll的线程数
* Output: *
* Others: * 此函数为静态static函数,
*************************************************/
int CEpollEx::InitThreadPool(int threadNum)
{
    pthread_t threadId;

    //初始化epoll线程池,
    for ( int i = 0; i < threadNum; i++)
    {
        errno = pthread_create(&threadId, 0, WorkLoop, (void *)0);
        if (errno != 0)
        {
            printf("pthread create failed!\n");
            return(errno);
        }
    }

    // errno = pthread_create(&threadId, 0, check_connect_timeout, (void *)0);

    return 0;
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
void* CEpollEx::WorkLoop(void* para)
{
    CLog::showLog(INF,"epoll is start work loop.");
    int n,i;
    while(true)
    {
        // 返回需要处理的事件数
        n=epoll_wait(g_pThis->m_EpollHandle,g_pThis->m_Events,1024,-1);
        if( n<=0 ) continue;

        for(i=0; i<n; ++i)
        {
            int ProcessSock =g_pThis->m_Events[i].data.fd;
            if(ProcessSock==g_pThis->m_ListenSock)
            {
                // 新连接接入
                while(true)
                {
                    // accept the connect and add it to the list
                    int conn_sock = -1;
                    while ((conn_sock = accept(g_pThis->m_ListenSock,NULL,NULL)) > 0)
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
                        g_pThis->m_SockList[conn_sock] = info;
                        CLog::showLog(INF,CUniversal::Format("accept client:%s.",info->m_Address));
                        //将接受的连接添加到Epoll的事件中.
                        // Add the recv event to epoll;
                        g_pThis->SetSocketNonblocking(conn_sock);
                        g_pThis->CtlEpollEvent(EPOLL_CTL_ADD,conn_sock,EPOLLIN|EPOLLET);
                    }
                    if (conn_sock == -1) {
                        if (errno == EAGAIN )
                            break;
                        else
                            CLog::showLog(INF,"accept error.");

                    }
                }
            }
            else if(g_pThis->m_Events[i].events&EPOLLIN)
            {
                CLog::showLog(INF,"Socket can read.");
                // 已经连接的用户,收到数据,可以开始读入
                bool bLoop = true;
                while(bLoop)
                {
                    // 先读取消息长度
                    int len = 8;//sizeof(long long);
                    int readLen;
                    char dataLeng[9] = {0};
                    readLen = recv(ProcessSock,dataLeng,len,0);
                    CLog::showLog(INF,boost::format("recv %d bytes.")%readLen);
                    if(readLen < 0)
                    {
                        //由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可//读在这里就当作是该次事件已处理过。
                        if(errno == EAGAIN)
                        {
                            printf("EAGAIN\n");
                            break;
                        }
                        else
                        {
                            // 读取错误,将这个连接从监听中移除并关闭连接
                            printf("recv error!\n");
                            //epoll_ctl(g_pThis->m_EpollHandle, EPOLL_CTL_DEL, ProcessSock, &g_pThis->m_Event);
                            close(ProcessSock);
                            g_pThis->CtlEpollEvent(EPOLL_CTL_DEL,ProcessSock,EPOLLIN|EPOLLOUT|EPOLLET);
                            break;
                        }
                    }
                    else if(readLen == 0)
                    {
                        // The connect is disconnected.
                        close(ProcessSock);
                        g_pThis->CtlEpollEvent(EPOLL_CTL_DEL,ProcessSock,EPOLLIN|EPOLLOUT|EPOLLET);
                        //epoll_ctl(g_pThis->m_EpollHandle,EPOLL_CTL_DEL, ProcessSock, &g_pThis->m_Event);
                        break;
                    }
                    else
                    {
                        long dtlen = atol(dataLeng);
                        CLog::showLog(INF,boost::format("dataLen=%s|%d")%dataLeng%dtlen);
                        dtlen++;
                        char* data = new char[dtlen];
                        memset(data,0,dtlen);

                        readLen = recv(ProcessSock,data,dtlen,0);//一次性接受所有消息
                        CLog::showLog(INF,boost::format("recv msg:%d|%s")%dtlen%data);

                        if(readLen >= dtlen)
                            bLoop = true; // 需要再次读取
                        else
                            bLoop = false;

                        // Add the msg to the sock info list
                        string msg(data);
                        CLog::showLog(INF,boost::format("data to string is %s")%msg);
                        CSockInfo* info = g_pThis->m_SockList[ProcessSock];
                        info->m_ReadList.push(msg);
                        delete[] data;

                        // Add the sock event to list
                        g_pThis->m_EventSockList.push(ProcessSock);
                    }
                }
            }
            else if(g_pThis->m_Events[i].events&EPOLLOUT)
            {
                // 可以写入事件
                CLog::showLog(INF,"Socket can write.");
                CSockInfo* info = g_pThis->m_SockList[ProcessSock];
                while (info->m_WriteList.size())
                {
                    string msg = info->m_WriteList.front();
                    info->m_WriteList.pop();
                    CLog::showLog(INF,boost::format("send message %1%")%msg);
                    if(!SendEx(ProcessSock,msg))
                    {
                        CLog::showLog(ERR,"write error.");
                    }
                }
            }
            else
            {
                //close(g_pThis->m_Events[i].data.fd);
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

void CEpollEx::SendMessage(int sock, string msg)
{
    if( false == SendEx(sock,msg,true) )
    {
        CSockInfo* info = m_SockList[sock];
        info->m_WriteList.push(msg);
        // Add to epoll list
        g_pThis->SetSocketNonblocking(sock);
        g_pThis->CtlEpollEvent(EPOLL_CTL_ADD,sock,EPOLLOUT|EPOLLET);
    }
}

bool CEpollEx::SendEx( int sock, string msg, bool eagain /* = false */ )
{
    long long len = msg.size() + 1;
    char* buf = new char[msg.size()+8+1];
    memcpy(buf,(void*)&len,8);
    memcpy(buf+8,msg.c_str(),msg.size()+1);
    //send(ProcessSock,temp,msg.size()+8+1,0);


    int nwrite, data_size = msg.length() +8 +1;
    //const char* buf = msg.c_str();
    int n = data_size;
    while (n > 0) {
        nwrite = write(sock, buf + data_size - n, n);
        if (nwrite < n)
        {
            // if errno != EAGAIN or again for error and return is -1, return false
            if (nwrite == -1 && (errno != EAGAIN  || eagain == true )) {
                delete[] buf;
                return false;
            }
            break;
        }
        n -= nwrite;
    }
    delete[] buf;
    return true;
}
