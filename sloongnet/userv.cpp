/* File Name: server.c */
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#define DEFAULT_PORT 8000
#define MAXLINE 4096
using namespace std;

#include "userv.h"
#include "epollex.h"
#include "msgproc.h"
#include <univ/log.h>
#include <univ/univ.h>

SloongWallUS::SloongWallUS()
{
    CLog::showLog(INF,"SloongWalls Linux Server object is build.");

}

SloongWallUS::~SloongWallUS()
{

}


void SloongWallUS::Initialize( int nPort )
{
    m_pEpoll = new CEpollEx();
    m_pEpoll->Initialize(1,nPort);

    m_pMsgProc = new CMsgProc();
}

void SloongWallUS::Run()
{
    while(true)
    {
        if ( m_pEpoll->m_EventSockList.size() > 0 )
        {
            // process read list.
            int sock = m_pEpoll->m_EventSockList.front();
            m_pEpoll->m_EventSockList.pop();
            CSockInfo* info = m_pEpoll->m_SockList[sock];
            if( !info ) continue;
            while ( info->m_ReadList.size() > 0)
            {
                string msg = info->m_ReadList.front();
                info->m_ReadList.pop();
                string res = m_pMsgProc->MsgProcess(msg);
                m_pEpoll->SendMessage(sock,res);
            }
        }
    }
}

