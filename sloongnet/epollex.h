#ifndef CEPOLLEX_H
#define CEPOLLEX_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <queue>
using namespace std; //std 命名空间

class CEpollEx
{
public:
    CEpollEx();
    virtual ~CEpollEx();
    int Initialize( int nThreadNums, int listenPort);
protected:
    int InitThreadPool(int ThreadNum);
    int SetSocketNonblocking(int socket);
    static void* WorkLoop(void* params);
protected:
    static CEpollEx* g_pThis;
    int     m_ListenSock;
    int 	m_EpollHandle;
    struct epoll_event m_Event;
    struct epoll_event m_Events[1024];

public:
    queue<string>	m_ReadList;
    queue<string>	m_WriteList;
};

#endif // CEPOLLEX_H
