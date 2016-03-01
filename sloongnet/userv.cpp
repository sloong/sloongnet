/* File Name: server.c */
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include "semaphore.h"
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
	m_bIsRunning = false;
	CThreadPool::End();
	SAFE_DELETE(m_pEpoll);
	SAFE_DELETE(m_pMsgProc);
	SAFE_DELETE(m_pLog);
}


void SloongWallUS::Initialize(CServerConfig* config)
{
	sem_init(&m_oSem, 0, 0);
    m_pConfig = config;
	m_nPriorityLevel = config->m_nPriorityLevel;
	m_pLog->Initialize(config->m_strLogPath, config->m_bDebug);
    m_pLog->SetWorkInterval(config->m_nSleepInterval);
    m_pEpoll->Initialize(m_pLog,config->m_nPort,config->m_nEPoolThreadQuantity,config->m_nPriorityLevel);
	m_pEpoll->SetSEM(&m_oSem);
    m_pMsgProc->Initialize(m_pLog,config->m_strScriptFolder);
}

void SloongWallUS::Run()
{
	m_bIsRunning = true;
	CThreadPool::AddWorkThread(SloongWallUS::HandleEventWorkLoop, this, m_pConfig->m_nProcessThreadQuantity);
	string cmd;
	while (m_bIsRunning)
	{
		cmd.clear();
		cin >> cmd;
		if (cmd == "exit")
		{
			Exit();
			return;
		}
		else
		{
			CCmdProcess::Parser(cmd);
		}
	}
}

void* SloongWallUS::HandleEventWorkLoop(void* pParam)
{
	SloongWallUS* pThis = (SloongWallUS*)pParam;
    auto log = pThis->m_pLog;
    auto pid = this_thread::get_id();
    string spid = CUniversal::ntos(pid);
    log->Log("Event process thread is running." + spid);
	int id = pThis->m_pMsgProc->NewThreadInit();
	struct timespec ts;
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

				ProcessEventList(id, &info->m_pReadList[i], info->m_oReadListMutex, sock, i, info->m_pUserInfo, pThis->m_pEpoll, pThis->m_pMsgProc);

				// unlock current level.
				lock.unlock();
			}

		}
		else
		{
			ts.tv_sec = time(NULL) + 1;
			sem_timedwait(&pThis->m_oSem,&ts);
		}
	}
	return NULL;
}

void Sloong::SloongWallUS::ProcessEventList(int id, queue<string>* pList, mutex& oLock, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc)
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

		string msg = pList->front();
		pList->pop();
		oLock.unlock();
		ProcessEvent(id, msg, sock, nPriorityLevel, pUserInfo, pEpoll,pMsgProc);
	}
}


void Sloong::SloongWallUS::ProcessEvent(int id, string& strMsg, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc)
{
	auto md5index = strMsg.find("|");
	auto swiftindex = strMsg.find("|", md5index + 1);
	string md5 = strMsg.substr(0, md5index);
	string swift = (strMsg.substr(md5index + 1, swiftindex - (md5index + 1)));
	string tmsg = strMsg.substr(swiftindex + 1);
	string rmd5 = CUniversal::MD5_Encoding(tmsg);
	CUniversal::touper(md5);
	CUniversal::touper(rmd5);
	if (md5 != rmd5)
	{
		// handle error.
		pEpoll->SendMessage(sock, nPriorityLevel, swift, "-1|package check error");
		return;
	}

	string strRes;
	char* pBuf = NULL;
	int nSize = pMsgProc->MsgProcess(id, pUserInfo, tmsg, strRes, pBuf);
	pEpoll->SendMessage(sock, nPriorityLevel, swift, strRes, pBuf, nSize);
}

void Sloong::SloongWallUS::Exit()
{
	m_pEpoll->Exit();
	m_bIsRunning = false;
	sem_post(&m_oSem);
}

