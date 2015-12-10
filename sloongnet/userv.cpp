/* File Name: server.c */
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
using namespace std;

#include <univ/log.h>
#include <univ/univ.h>
#include <univ/threadpool.h>
using namespace Sloong;
using namespace Sloong::Universal;

#include "epollex.h"
#include "msgproc.h"
#include "serverconfig.h"
#include "utility.h"

#define MAXLINE 4096
#include "userv.h"

SloongWallUS::SloongWallUS()
{
	m_pLog = new CLog();
	m_pEpoll = new CEpollEx(m_pLog);
	m_pMsgProc = new CMsgProc(m_pLog);
	m_pThreadPool = new CThreadPool();
}

SloongWallUS::~SloongWallUS()
{
	SAFE_DELETE(m_pEpoll);
	SAFE_DELETE(m_pMsgProc);
	m_pThreadPool->End();
	SAFE_DELETE(m_pThreadPool);
	SAFE_DELETE(m_pLog);
}


void SloongWallUS::Initialize(CServerConfig* config)
{
    m_pLog->Initialize(config->m_strLogPath);
	m_pLog->g_bDebug = config->m_bDebug;
    m_pEpoll->Initialize(config->m_nThreadNum,config->m_nPort);
	m_pThreadPool->Initialize(config->m_nThreadNum);

	CUtility uti;
	int n1, n2;
	uti.GetMemory(n1, n2);
	uti.GetCpuUsed();
}

void SloongWallUS::Run()
{
	m_pThreadPool->AddTask(SloongWallUS::HandleEventWorkLoop, this, true);
	m_pThreadPool->Start();
	char buff[256];
	while (true)
	{
		
		cin >> buff;
		cout << buff;
		sleep(10);
	}
}

void* SloongWallUS::HandleEventWorkLoop( void* pParam )
{
	SloongWallUS* pThis = (SloongWallUS*)pParam;

		if (pThis->m_pEpoll->m_EventSockList.size() > 0)
		{
			// process read list.
			int sock = pThis->m_pEpoll->m_EventSockList.front();
			pThis->m_pEpoll->m_EventSockList.pop();
			CSockInfo* info = pThis->m_pEpoll->m_SockList[sock];
            if (!info) return NULL;
			while (info->m_ReadList.size() > 0)
			{
				string msg = info->m_ReadList.front();
				info->m_ReadList.pop();
				string res = pThis->m_pMsgProc->MsgProcess(msg);
				pThis->m_pEpoll->SendMessage(sock, res);
			}
		}

}



