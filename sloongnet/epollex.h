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

class CEpollEx
{
public:
    CEpollEx();
    virtual ~CEpollEx();
    int Initialize( int nThreadNums, int listenPort);
    void SendMessage( int sock, string msg );
protected:
    int InitThreadPool(int ThreadNum);
    int SetSocketNonblocking(int socket);
    static void* WorkLoop(void* params);
    static bool SendEx( int sock, string msg, bool eagagin = false );
    void CtlEpollEvent( int opt, int sock, int events );
protected:
    static CEpollEx* g_pThis;
    int     m_ListenSock;
    int 	m_EpollHandle;
    //struct epoll_event m_Event;
    struct epoll_event m_Events[1024];

public:
    map<int,CSockInfo*> m_SockList;
    queue<int> m_EventSockList;
};

#endif // CEPOLLEX_H