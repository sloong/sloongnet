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
	m_pThreadPool->End();
	SAFE_DELETE(m_pEpoll);
	SAFE_DELETE(m_pMsgProc);
	SAFE_DELETE(m_pThreadPool);
	SAFE_DELETE(m_pLog);
}


void SloongWallUS::Initialize(CServerConfig* config)
{
	m_nPriorityLevel = config->m_nPriorityLevel;
	m_pLog->Initialize(config->m_strLogPath, config->m_bDebug);
    m_pEpoll->Initialize(m_pLog,config->m_nPort,config->m_nThreadNum,config->m_nPriorityLevel);
    m_pMsgProc->Initialize(m_pLog,config->m_strScriptFolder);
	m_pThreadPool->Initialize(config->m_nThreadNum);
}

void SloongWallUS::Run()
{
	m_pThreadPool->AddTask(SloongWallUS::HandleEventWorkLoop, this, true);
	m_pThreadPool->Start();
	string cmd;
	while (true)
	{
		cmd.clear();
		cin >> cmd;
		if (cmd == "exit")
			return;
		else
			SLEEP(1000);
	}
}

void* SloongWallUS::HandleEventWorkLoop(void* pParam)
{
	SloongWallUS* pThis = (SloongWallUS*)pParam;

	if (!pThis->m_pEpoll->m_EventSockList.empty())
	{
		unique_lock<mutex> eventLoc(pThis->m_pEpoll->m_oEventListMutex);
		if (pThis->m_pEpoll->m_EventSockList.empty())
			return NULL;

		// process read list.
		int sock = pThis->m_pEpoll->m_EventSockList.front();
		pThis->m_pEpoll->m_EventSockList.pop();
		eventLoc.unlock();

		CSockInfo* info = pThis->m_pEpoll->m_SockList[sock];
		if (!info) return NULL;

        for (int i = 0; i < pThis->m_nPriorityLevel || i == 0; i++ )
		{
			// if not empty
			while (!info->m_pReadList[i].empty())
			{
                if( info->m_oReadMutex.try_lock() == false )
                {
                    continue;
                }
                unique_lock<mutex> readLoc(info->m_oReadMutex,std::adopt_lock);
				// recheck is not empty when lock done.
				if (info->m_pReadList[i].empty())
				{
					readLoc.unlock();
					continue;
				}

				string msg = info->m_pReadList[i].front();
				info->m_pReadList[i].pop();
				readLoc.unlock();
				auto md5index = msg.find("|");
				auto swiftindex = msg.find("|", md5index + 1);
				string md5 = msg.substr(0, md5index);
				//int swift = boost::lexical_cast<int>(msg.substr(md5index, swiftindex - md5index));
				string swift = (msg.substr(md5index + 1, swiftindex - (md5index + 1)));
				string tmsg = msg.substr(swiftindex + 1);
				string rmd5 = CUniversal::MD5_Encoding(tmsg);
				CUniversal::touper(md5);
				CUniversal::touper(rmd5);
				if (md5 != rmd5)
				{
					// handle error.
					pThis->m_pEpoll->SendMessage(sock, i, swift, "-1|package check error");
					continue;
				}

				string strRes;
				char* pBuf = NULL;
				int nSize = pThis->m_pMsgProc->MsgProcess(info->m_pUserInfo, tmsg, strRes, pBuf);
				pThis->m_pEpoll->SendMessage(sock, i, swift, strRes, pBuf, nSize);
			}
		}
		
	}
	else
	{
		SLEEP(500);
	}
	return NULL;
}



