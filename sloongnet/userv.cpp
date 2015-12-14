/* File Name: server.c */
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
using namespace std;
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <univ/log.h>
#include <univ/univ.h>
#include <univ/threadpool.h>
using namespace Sloong;
using namespace Sloong::Universal;

#include "epollex.h"
#include "msgproc.h"
#include "serverconfig.h"
#include "utility.h"

#include "userv.h"

SloongWallUS::SloongWallUS()
{
	m_pLog = new CLog();
    m_pEpoll = new CEpollEx();
    m_pMsgProc = new CMsgProc();
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
	m_pLog->Initialize(config->m_strLogPath, config->m_bDebug);
    m_pEpoll->Initialize(m_pLog,config->m_nPort,config->m_nThreadNum);
    m_pMsgProc->Initialize(m_pLog);
	m_pThreadPool->Initialize(config->m_nThreadNum);
}

void SloongWallUS::Run()
{
	m_pThreadPool->AddTask(SloongWallUS::HandleEventWorkLoop, this, true);
	m_pThreadPool->Start();
	char buff[256];
	while (true)
	{
// 		cin >> buff;
// 		cout << buff;
		SLEEP(100);
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

				auto md5index = msg.find("|");
				auto swiftindex = msg.find("|", md5index+1);
				string md5 = msg.substr(0, md5index);
				//int swift = boost::lexical_cast<int>(msg.substr(md5index, swiftindex - md5index));
				string swift = (msg.substr(md5index+1, swiftindex - (md5index+1)));
				string tmsg = msg.substr(swiftindex+1);
				string rmd5 = CUtility::MD5_Encoding(tmsg);
				CUtility::tolower(md5);
				CUtility::tolower(rmd5);
				if (md5 != rmd5)
				{
					// handle error.
					pThis->m_pEpoll->SendMessage(sock, swift, "-1|package check error");
					continue;
				}

				string res = pThis->m_pMsgProc->MsgProcess( info->m_pUserInfo ,tmsg);
				pThis->m_pEpoll->SendMessage(sock, swift, res);
			}
		}

}



