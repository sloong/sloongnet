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
#include "CmdProcess.h"
#include "userv.h"

SloongWallUS::SloongWallUS()
{
	m_pLog = new CLog();
    m_pEpoll = new CEpollEx();
    m_pMsgProc = new CMsgProc();
	m_bIsRunning = false;
}

SloongWallUS::~SloongWallUS()
{
	Exit();
	CThreadPool::End();
	SAFE_DELETE(m_pEpoll);
	SAFE_DELETE(m_pMsgProc);
    m_pLog->End();
	SAFE_DELETE(m_pLog);
}


void SloongWallUS::Initialize(CServerConfig* config)
{
    m_pConfig = config;
	m_nPriorityLevel = config->m_nPriorityLevel;
	m_pLog->Initialize(config->m_strLogPath, config->m_bDebug);
	if (config->m_bLogWriteToOneFile)
	{
		LOGTYPE oType = LOGTYPE::DAY;
		m_pLog->SetConfiguration(NULL, NULL, &oType, NULL, config->m_bDebug);
	}
    m_pLog->SetWorkInterval(config->m_nSleepInterval);
    m_pEpoll->Initialize(m_pLog,config->m_nPort,config->m_nEPoolThreadQuantity,config->m_nPriorityLevel, 
				config->m_bEnableSwiftNumberSup, config->m_bEnableMD5Check, config->m_nTimeout, config->m_nTimeoutInterval);
	m_pEpoll->SetLogConfiguration(m_pConfig->m_bShowSendMessage, m_pConfig->m_bShowReceiveMessage);
    m_pEpoll->SetEvent(&m_oEventCV);
    m_pMsgProc->Initialize(m_pLog,&config->m_oConnectInfo, &config->m_oLuaConfigInfo, config->m_bShowSQLCmd, config->m_bShowSQLResult);
}

void SloongWallUS::Run()
{
	m_bIsRunning = true;
	CThreadPool::AddWorkThread(SloongWallUS::HandleEventWorkLoop, this, m_pConfig->m_nProcessThreadQuantity);
	unique_lock<mutex> lck(m_oEventMutex);
	while (m_bIsRunning)
	{
		m_oEventCV.wait(lck);
	}
	Exit();
}

void* SloongWallUS::HandleEventWorkLoop(void* pParam)
{
	SloongWallUS* pThis = (SloongWallUS*)pParam;
    auto log = pThis->m_pLog;
    auto pid = this_thread::get_id();
    string spid = CUniversal::ntos(pid);
    log->Log("Event process thread is running." + spid);
	int id = pThis->m_pMsgProc->NewThreadInit();
    unique_lock<mutex> lck(pThis->m_oEventMutex);
	while (pThis->m_bIsRunning)
	{
		if (!pThis->m_pEpoll->m_EventSockList.empty())
        {
			unique_lock<mutex> eventLoc(pThis->m_pEpoll->m_oEventListMutex);
			if (pThis->m_pEpoll->m_EventSockList.empty())
				continue;

			// process read list.
			int sock = pThis->m_pEpoll->m_EventSockList.front();
			pThis->m_pEpoll->m_EventSockList.pop();
			eventLoc.unlock();

			CSockInfo* info = pThis->m_pEpoll->m_SockList[sock];
			if (!info)
			{
				log->Log(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", sock), LOGLEVEL::WARN);
				continue;
			}

			for (int i = 0; i < pThis->m_nPriorityLevel || i == 0; i++)
			{
				// try to lock the process mutex in current priority level.
				if (info->m_pProcessMutexList[i].try_lock() == false)
				{
					// this sock is processed in other thread. continue
					continue;
				}

				unique_lock<mutex> lock(info->m_pProcessMutexList[i], std::adopt_lock);

				pThis->ProcessEventList(id, &info->m_pReadList[i], info->m_oReadListMutex, sock, i, info->m_pUserInfo, pThis->m_pEpoll, pThis->m_pMsgProc);

				// unlock current level.
				lock.unlock();
			}

		}
		else
		{
            pThis->m_oEventCV.wait_for(lck,chrono::minutes(1));
        }
	}
	return NULL;
}

void Sloong::SloongWallUS::ProcessEventList(int id, queue<RECVINFO>* pList, mutex& oLock, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc)
{
	// if not empty
	while (!pList->empty())
	{
		if (oLock.try_lock() == false)
		{
			continue;
		}
		unique_lock<mutex> readLoc(oLock, std::adopt_lock);
		// recheck is not empty when lock done.
		if (pList->empty())
		{
			oLock.unlock();
			continue;
		}

		RECVINFO msg = pList->front();
		pList->pop();
		oLock.unlock();
		ProcessEvent(id, &msg, sock, nPriorityLevel, pUserInfo, pEpoll,pMsgProc);
	}
}


void Sloong::SloongWallUS::ProcessEvent(int id, RECVINFO* info, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc)
{
	if (m_pConfig->m_bEnableMD5Check)
	{
		string rmd5 = CUniversal::MD5_Encoding(info->strMessage);
		CUniversal::touper(info->strMD5);
		CUniversal::touper(rmd5);
		if (info->strMD5 != rmd5)
		{
			// handle error.
			pEpoll->SendMessage(sock, nPriorityLevel, info->nSwiftNumber, "{\"errno\": \"-1\",\"errmsg\" : \"package check error\"}");
			return;
		}
	}
	
	string strRes("");
	char* pBuf = NULL;
	int nSize = pMsgProc->MsgProcess(id, pUserInfo, info->strMessage, strRes, pBuf);
	pEpoll->SendMessage(sock, nPriorityLevel, info->nSwiftNumber, strRes, pBuf, nSize);
}

void Sloong::SloongWallUS::Exit()
{
	m_pEpoll->Exit();
	m_bIsRunning = false;
    unique_lock<mutex> lck(m_oEventMutex);
    m_oEventCV.notify_all();
}

