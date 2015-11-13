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


void SloongWallUS::Initialize()
{
    m_pEpoll = new CEpollEx();
    m_pEpoll->Initialize(1,8000);

    m_pMsgProc = new CMsgProc();
}

void SloongWallUS::Run()
{
    while(true)
    {
        if ( m_pEpoll->m_ReadList.size() > 0 )
        {
            // process read list.
            string msg = m_pEpoll->m_ReadList.front();
            m_pEpoll->m_ReadList.pop();
            if (true == m_pMsgProc->MsgProcess(msg))
                m_pEpoll->m_WriteList.push("succedss");
            else
                m_pEpoll->m_WriteList.push("field");
        }
    }
}
