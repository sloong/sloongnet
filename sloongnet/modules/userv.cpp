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

#include "MessageCenter.h"
#include "DataCenter.h"
#include "ControlCenter.h"

SloongWallUS::SloongWallUS()
{
	m_pLog = make_unique<CLog>();
    m_pCC = make_unique<CControlCenter>();
	m_pDC = make_unique<CDataCenter>();
	m_pMC = make_unique<CMessageCenter>();
	m_bIsRunning = false;
}

SloongWallUS::~SloongWallUS()
{
	Exit();
	CThreadPool::Exit();
    m_pLog->End();
}


void SloongWallUS::Initialize(CServerConfig* config)
{
	LOGTYPE oType = LOGTYPE::ONEFILE;
	if (!config->m_oLogInfo.LogWriteToOneFile)
	{
		oType = LOGTYPE::DAY;
	}
	m_pLog->Initialize(config->m_oLogInfo.LogPath, config->m_oLogInfo.DebugMode, LOGLEVEL(config->m_oLogInfo.LogLevel), oType);
	if (config->m_oLogInfo.NetworkPort != 0)
		m_pLog->EnableNetworkLog(config->m_oLogInfo.NetworkPort);

	m_pDC->Add(Configuation, config);
	m_pDC->Add(Logger, m_pLog.get());
	
	m_pMC->Initialize(m_pDC.get(),config->m_nMessageCenterThreadQuantity);
	m_pMC->RegisterEvent(ProgramExit);
	m_pMC->RegisterEvent(ProgramStart);
	m_pMC->RegisterEventHandler(MSG_TYPE::ProgramExit, std::bind(&SloongWallUS::EventHandler, this, std::placeholders::_1));

	try{
		m_pCC->Initialize(m_pMC.get(), m_pDC.get());
	}
	catch(exception e)
	{
		m_pLog->Error("Excepiton happened in initialize for ControlCenter. Message:"+  e.what());
	}
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

void Sloong::SloongWallUS::EventHandler(SmartEvent ev)
{
	auto type = ev->GetEvent();
	switch (type)
	{
	case ProgramExit:
		Exit();
		break;
	default:
		break;
	}
}

