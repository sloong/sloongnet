#include "ControlCenter.h"
#include "defines.h"
#include "epollex.h"
#include "LuaProcessCenter.h"
#include "globalfunction.h"
#include "serverconfig.h"
#include "NetworkEvent.h"
#include "SendMessageEvent.h"
#include "sockinfo.h"
#include "DataTransPackage.h"
using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;

CControlCenter::CControlCenter()
{
	m_pEpoll = make_unique<CEpollEx>();
	m_pProcess = make_unique<CLuaProcessCenter>();
}


CControlCenter::~CControlCenter()
{
}

void Sloong::CControlCenter::Initialize(IMessage* iM,IData* iData)
{
	m_iM = iM;
	m_iData = iData;
	
	m_pConfig = (CServerConfig*)m_iData->Get(Configuation);
	m_pLog = (CLog*)m_iData->Get(Logger);
	m_pEpoll->Initialize(m_iM,m_iData);

	m_pProcess->Initialize(m_iM, m_iData);
	if (m_pConfig->m_bEnableSSL)
	{
		m_pEpoll->EnableSSL(m_pConfig->m_strCertFile, m_pConfig->m_strKeyFile, m_pConfig->m_strPasswd);
	}

	m_pEpoll->SetLogConfiguration(m_pConfig->m_oLogInfo.ShowSendMessage, m_pConfig->m_oLogInfo.ShowReceiveMessage);

	// 在所有的成员都初始化之后，在注册处理函数
	iM->RegisterEventHandler(ProgramStart, std::bind(&CControlCenter::Run, this, std::placeholders::_1));
	iM->RegisterEventHandler(ProgramExit, std::bind(&CControlCenter::Exit, this, std::placeholders::_1));
	iM->RegisterEventHandler(ReveivePackage, std::bind(&CControlCenter::OnReceivePackage, this, std::placeholders::_1));
	iM->RegisterEventHandler(SocketClose, std::bind(&CControlCenter::OnSocketClose, this, std::placeholders::_1));
}


void Sloong::CControlCenter::OnReceivePackage(SmartEvent evt)
{	
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(evt);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	SmartPackage pack = net_evt->GetRecvPackage();
	auto send_evt = make_shared<CSendMessageEvent>(net_evt->GetSocketID(), net_evt->GetPriority(), pack->nSwiftNumber);
	if (m_pConfig->m_bEnableMD5Check)
	{
		string rmd5 = CMD5::Encode(pack->strMessage);
		CUniversal::touper(pack->strMD5);
		CUniversal::touper(rmd5);
		if (pack->strMD5 != rmd5)
		{
			// handle error.
			string strSend = CUniversal::Format("{\"errno\": \"-1\",\"errmsg\" : \"package check error\",\"server_md5\":\"%s\",\"client_md5\":\"%s\",\"check_string\":\"%s\"}", rmd5, pack->strMD5, pack->strMessage);

			send_evt->SetMessage(strSend);
			m_iM->SendMessage(send_evt);
			return;
		}
	}

	string strRes("");
	char* pExData = nullptr;
	int nExSize;
	if (m_pProcess->MsgProcess(info, pack->strMessage, strRes, pExData, nExSize))
	{
		if (pExData && nExSize > 0 )
		{
			send_evt->SetSendExData(pExData,nExSize);
		}
		send_evt->SetMessage(strRes);
	}
	else
	{
		m_pLog->Error("Error in process");
		send_evt->SetMessage("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
	}
	m_iM->SendMessage(send_evt);
}

void Sloong::CControlCenter::OnSocketClose(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	// call close function.
	m_pProcess->CloseSocket(info);
	net_evt->CallCallbackFunc(net_evt);
}

void Sloong::CControlCenter::Run(SmartEvent event)
{
	m_pEpoll->Run();
}

void Sloong::CControlCenter::Exit(SmartEvent event)
{
	m_pEpoll->Exit();
}