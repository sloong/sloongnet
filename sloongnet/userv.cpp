/* File Name: server.c */
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
using namespace std;
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "serverconfig.h"
#include "CmdProcess.h"
#include "userv.h"
#include "structs.h"

#include "MessageCenter.h"
#include "DataCenter.h"
#include "ControlCenter.h"

SloongWallUS::SloongWallUS()
{
	m_pLog = new CLog();
    m_pCC = new CControlCenter();
	m_pDC = new CDataCenter();
	m_pMC = new CMessageCenter();
	m_bIsRunning = false;
}

SloongWallUS::~SloongWallUS()
{
	Exit();
	CThreadPool::Exit();
	SAFE_DELETE(m_pCC);
	SAFE_DELETE(m_pDC);
	SAFE_DELETE(m_pMC);
    m_pLog->End();
	SAFE_DELETE(m_pLog);
}


void SloongWallUS::Initialize(CServerConfig* config)
{
	LOGTYPE oType = LOGTYPE::ONEFILE;
	if (!config->m_oLogInfo.LogWriteToOneFile)
	{
		oType = LOGTYPE::DAY;
	}
	m_pLog->Initialize(config->m_oLogInfo.LogPath, config->m_oLogInfo.DebugMode, LOGLEVEL(config->m_oLogInfo.LogLevel), oType);
	m_pLog->SetWorkInterval(config->m_nSleepInterval);
	if (config->m_oLogInfo.NetworkPort != 0)
		m_pLog->EnableNetworkLog(config->m_oLogInfo.NetworkPort);

	m_pDC->Add(Configuation, config);
	m_pDC->Add(Logger, m_pLog);
	
	m_pMC->Initialize(1, 1);
	m_pMC->RegisterEvent(ProgramExit);
	m_pMC->RegisterEvent(ProgramStart);
	m_pMC->RegisterEventHandler(MSG_TYPE::ProgramExit, this, EventHandler);

	m_pCC->Initialize(m_pMC, m_pDC);
}

void SloongWallUS::Run()
{
	m_bIsRunning = true;
	m_pMC->Run();
	CThreadPool::Run();
	unique_lock<mutex> lck(m_oExitEventMutex);
	m_pMC->SendMessage(MSG_TYPE::ProgramStart);
	while (m_bIsRunning)
	{
		m_oExitEventCV.wait(lck);
	}
}

void Sloong::SloongWallUS::Exit()
{
	m_pMC->SendMessage(MSG_TYPE::ProgramExit);
	m_bIsRunning = false;
	unique_lock<mutex> lck(m_oExitEventMutex);
	m_oExitEventCV.notify_all();
}

void * Sloong::SloongWallUS::EventHandler(void * t, void* object)
{
	IEvent* ev = (IEvent*)t;
	auto type = ev->GetEvent();
	auto pThis = TYPE_TRANS<SloongWallUS*>(object);
	switch (type)
	{
	case ProgramExit:
		pThis->Exit();
		break;
	}
	SAFE_RELEASE_EVENT(ev);
	return nullptr;
}

