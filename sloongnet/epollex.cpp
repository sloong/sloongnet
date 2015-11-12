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
    memset(&m_Event,0,sizeof(m_Event));
    m_Event.data.fd=m_ListenSock;
    m_Event.events=EPOLLIN|EPOLLET|EPOLLOUT;

    CLog::showLog(INF,boost::format("EpollHanled=%d")%m_EpollHandle);
    // 设置事件到epoll对象
    epoll_ctl(m_EpollHandle,EPOLL_CTL_ADD,m_ListenSock,&m_Event);

    // 创建线程池
    return InitThreadPool(nThreadNums);
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
                    CLog::showLog(INF,"one client is accpet.");
                    g_pThis->m_Event.data.fd=accept(g_pThis->m_ListenSock,NULL,NULL);
                    if(g_pThis->m_Event.data.fd>0)
                    {
                        //将接受的连接添加到Epoll的事件中.
                        g_pThis->SetSocketNonblocking(g_pThis->m_Event.data.fd);
                        g_pThis->m_Event.events=EPOLLIN|EPOLLET|EPOLLOUT;
                        epoll_ctl(g_pThis->m_EpollHandle,EPOLL_CTL_ADD,g_pThis->m_Event.data.fd,&g_pThis->m_Event);
                    }
                    else
                    {
                        if(errno==EAGAIN)
                            break;
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
                            epoll_ctl(g_pThis->m_EpollHandle, EPOLL_CTL_DEL, ProcessSock, &g_pThis->m_Event);
                            close(ProcessSock);
                            break;
                        }
                    }
                    else if(readLen == 0)
                    {
                        // The connect is disconnected.
                        close(ProcessSock);
                        epoll_ctl(g_pThis->m_EpollHandle,EPOLL_CTL_DEL, ProcessSock, &g_pThis->m_Event);
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

                        string msg(data);
                        CLog::showLog(INF,boost::format("data to string is %s")%msg);
                        g_pThis->m_ReadList.push(msg);
                        delete[] data;
                    }
                }
                string res("success");
                long long len = res.size();
                char* temp = new char[res.size()+8+1];
                memcpy(temp,(void*)&len,8);
                memcpy(temp+8,res.c_str(),res.size()+1);
                send(ProcessSock,temp,res.size()+8+1,0);
                delete[] temp;
            }
            else if(g_pThis->m_Events[i].events&EPOLLOUT)
            {
                // 可以写入事件
                // CLog::showLog(INF,"Socket can write.");
                // if ( g_pThis->m_WriteList.size() > 0 )
                // {
                // CLog::showLog(INF,"Read to write message");
                // process read list.
                // string msg = g_pThis->m_WriteList.front();
                // CLog::showLog(INF,boost::format("send message %1%")%msg);
                // g_pThis->m_WriteList.pop();
                // send(ProcessSock,msg.c_str(),msg.size(),0);
                // }
                //
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
