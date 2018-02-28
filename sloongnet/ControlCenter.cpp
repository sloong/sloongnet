#include "ControlCenter.h"
#include <univ/MD5.h>
#include "epollex.h"
#include "LuaProcessCenter.h"
#include "globalfunction.h"
#include "serverconfig.h"
#include "NetworkEvent.h"
#include "SendMessageEvent.h"
#include "sockinfo.h"
using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;

CControlCenter* CControlCenter::g_pCC = nullptr;

CControlCenter::CControlCenter()
{
	if ( g_pCC != nullptr )
	{
		throw normal_except("ControlCenter can only have one instance!");
	}
	
	g_pCC = this;
	m_pEpoll = new CEpollEx();
	m_pProcess = new CLuaProcessCenter();
	m_pGFunc = new CGlobalFunction();
}


CControlCenter::~CControlCenter()
{
	SAFE_DELETE(m_pEpoll);
	SAFE_DELETE(m_pProcess);
	SAFE_DELETE(m_pGFunc);
}

void Sloong::CControlCenter::Initialize(IMessage* iM,IData* iData)
{
	m_iM = iM;
	m_iData = iData;
	iM->RegisterEventHandler(ProgramStart, this, EventHandler);
	iM->RegisterEventHandler(ProgramExit, this, EventHandler);
	iM->RegisterEventHandler(ReveivePackage, this, EventHandler);
	iM->RegisterEventHandler(SocketClose, this, EventHandler);

	m_pConfig = (CServerConfig*)m_iData->Get(Configuation);
	m_pLog = (CLog*)m_iData->Get(Logger);
	m_pEpoll->Initialize(m_iM,m_iData);
	m_pProcess->Initialize(m_iM, m_iData);
	m_pGFunc->Initialize(m_iM, m_iData);

	m_iData->Add(GlobalFunctions, m_pGFunc);
	if (m_pConfig->m_bEnableSSL)
	{
		m_pEpoll->EnableSSL(m_pConfig->m_strCertFile, m_pConfig->m_strKeyFile, m_pConfig->m_strPasswd);
	}

	m_pEpoll->SetLogConfiguration(m_pConfig->m_oLogInfo.ShowSendMessage, m_pConfig->m_oLogInfo.ShowReceiveMessage);
}


void Sloong::CControlCenter::OnReceivePackage(IEvent* evt)
{	
	CNetworkEvent* net_evt = (CNetworkEvent*)evt;
	CSockInfo* info = net_evt->GetSocketInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	RECVINFO* pack = net_evt->GetRecvPackage();
	CSendMessageEvent* send_msg = new CSendMessageEvent(net_evt->GetSocketID(), net_evt->GetPriority(), pack->nSwiftNumber);
	if (m_pConfig->m_bEnableMD5Check)
	{
		string rmd5 = CMD5::Encoding(pack->strMessage);
		CUniversal::touper(pack->strMD5);
		CUniversal::touper(rmd5);
		if (pack->strMD5 != rmd5)
		{
			// handle error.
			string strSend = CUniversal::Format("{\"errno\": \"-1\",\"errmsg\" : \"package check error\",\"server_md5\":\"%s\",\"client_md5\":\"%s\",\"check_string\":\"%s\"}", rmd5, pack->strMD5, pack->strMessage);

			send_msg->SetMessage(strSend);
			m_iM->SendMessage(send_msg);
			return;
		}
	}

	string strRes("");
	string strExUUID;
	int nExSize;
	if (m_pProcess->MsgProcess(info->m_pUserInfo.get(), pack->strMessage, strRes, strExUUID, nExSize))
	{
		if ( !strExUUID.empty())
		{
			const char* exData = TYPE_TRANS<const char*>(m_iData->GetTemp(strExUUID));
			send_msg->SetSendExData(exData,nExSize);
		}
		send_msg->SetMessage(strRes);
		m_iM->SendMessage(send_msg);
	}
	else
	{
		m_pLog->Error("Error in process");
		send_msg->SetMessage("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
		m_iM->SendMessage(send_msg);
	}
	// 这里处理模块也改为处理中心的模式。在这里发送处理请求，然后由处理中心调用回调函数。
	/*CNormalEvent* proc_msg = new CNormalEvent();
	proc_msg->SetHandler(this);
	proc_msg->SetEvent(MSG_TYPE::ProcessMessage);
	proc_msg->SetMessage(pack->strMessage);
	proc_msg->SetParams(info,false);
	proc_msg->SetCallbackFunc(net_evt->GetCallbackFunc());
	m_iM->SendMessage(proc_msg);*/
	/*int nSize = pMsgProc->MsgProcess(id, pUserInfo, info->strMessage, strRes, pBuf);
	send_msg->SetSendMessage(strRes);
	send_msg->SetSendExData(pBuf, nSize);
	auto func = send_msg->GetCallbackFunc();
	m_iM->SendMessage(evt);*/
	

}

void Sloong::CControlCenter::OnSocketClose(IEvent* evt)
{
	CNetworkEvent* net_evt = (CNetworkEvent*)evt;
	CSockInfo* info = net_evt->GetSocketInfo();
	CControlCenter* pThis = TYPE_TRANS<CControlCenter*>(evt->GetHandler());
	//TRANS_TYPE(evt->GetHandler(),CControlCenter*,pThis)
	if (!info)
	{
		pThis->m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	// call close function.
	pThis->m_pProcess->CloseSocket(info->m_pUserInfo.get());
	net_evt->CallCallbackFunc(net_evt);
}

LPVOID Sloong::CControlCenter::EventHandler(LPVOID t, LPVOID object)
{
	IEvent* ev = (IEvent*)t;
	auto type = ev->GetEvent();
	switch (type)
	{
	case ProgramStart:
		g_pCC->Run();
		break;
	case ProgramExit:
		
		break;
	case ReveivePackage:
		g_pCC->OnReceivePackage(ev);
		break;
	case SocketClose:
		g_pCC->OnSocketClose(ev);
		break;
	}
	SAFE_RELEASE_EVENT(ev);
}


void Sloong::CControlCenter::Run()
{
	m_pEpoll->Run();
}

void Sloong::CControlCenter::Exit()
{
	m_pEpoll->Exit();
}