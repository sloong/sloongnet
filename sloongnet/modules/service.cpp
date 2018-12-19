/* File Name: server.c */
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
using namespace std;
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "serverconfig.h"
#include "CmdProcess.h"
#include "service.h"

#include "MessageCenter.h"
#include "DataCenter.h"
#include "ControlCenter.h"

SloongNetService::SloongNetService()
{
	m_pLog = make_unique<CLog>();
    m_pCC = make_unique<CControlCenter>();
	m_pDC = make_unique<CDataCenter>();
	m_pMC = make_unique<CMessageCenter>();
	m_bIsRunning = false;
}

SloongNetService::~SloongNetService()
{
	Exit();
	CThreadPool::Exit();
    m_pLog->End();
}


void SloongNetService::Initialize(CServerConfig* config)
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
	m_pMC->RegisterEventHandler(MSG_TYPE::ProgramExit, std::bind(&SloongNetService::EventHandler, this, std::placeholders::_1));

	try{
		m_pCC->Initialize(m_pMC.get(), m_pDC.get());
	}
	catch(exception e)
	{
		m_pLog->Error(string("Excepiton happened in initialize for ControlCenter. Message:")+  string(e.what()));
	}
}

void SloongNetService::Run()
{
	m_bIsRunning = true;
	m_pMC->Run();
	CThreadPool::Run();
	m_pMC->SendMessage(MSG_TYPE::ProgramStart);
	while (m_bIsRunning)
	{
		m_oSync.wait();
	}
}

void Sloong::SloongNetService::Exit()
{
	m_pMC->SendMessage(MSG_TYPE::ProgramExit);
	m_bIsRunning = false;
	m_oSync.notify_all();
}

void Sloong::SloongNetService::EventHandler(SmartEvent ev)
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

