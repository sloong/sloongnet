#include "ControlCenter.h"
#include "defines.h"
#include "NetworkCenter.h"
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
	m_pNetwork = make_unique<CNetworkCenter>();
	m_pProcess = make_unique<CLuaProcessCenter>();
}


CControlCenter::~CControlCenter()
{
}

void Sloong::CControlCenter::Initialize(IMessage* iMsg,IData* iData)
{
	IObject::Initialize(iMsg,iData);

	m_pConfig = (CServerConfig*)m_iData->Get(Configuation);
	
	m_pNetwork->Initialize(m_iMsg,m_iData);
	m_pProcess->Initialize(m_iMsg, m_iData);
	
	// 在所有的成员都初始化之后，在注册处理函数
	m_iMsg->RegisterEventHandler(ProgramStart, std::bind(&CControlCenter::Run, this, std::placeholders::_1));
	m_iMsg->RegisterEventHandler(ProgramExit, std::bind(&CControlCenter::Exit, this, std::placeholders::_1));
	m_iMsg->RegisterEventHandler(ReveivePackage, std::bind(&CControlCenter::OnReceivePackage, this, std::placeholders::_1));
	m_iMsg->RegisterEventHandler(SocketClose, std::bind(&CControlCenter::OnSocketClose, this, std::placeholders::_1));
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
			m_iMsg->SendMessage(send_evt);
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
	m_iMsg->SendMessage(send_evt);
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
	m_pLog->Info("Application begin running.");
}

void Sloong::CControlCenter::Exit(SmartEvent event)
{
	m_pLog->Info("Application will exit.");
}