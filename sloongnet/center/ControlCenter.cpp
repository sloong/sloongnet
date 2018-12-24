#include "ControlCenter.h"
#include "NetworkCenter.h"
#include "LuaProcessCenter.h"
#include "globalfunction.h"
#include "serverconfig.h"
#include "NetworkEvent.h"
#include "sockinfo.h"
#include "DataTransPackage.h"
#include "IData.h"
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

void Sloong::CControlCenter::Initialize(IControl* iMsg)
{
	IObject::Initialize(iMsg);

	m_pConfig = IData::GetServerConfig();
	
	m_pNetwork->Initialize(m_iC);
	m_pProcess->Initialize(m_iC);
	
	// 在所有的成员都初始化之后，在注册处理函数
	m_iC->RegisterEventHandler(ProgramStart, std::bind(&CControlCenter::Run, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(ProgramExit, std::bind(&CControlCenter::Exit, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(ReveivePackage, std::bind(&CControlCenter::OnReceivePackage, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(SocketClose, std::bind(&CControlCenter::OnSocketClose, this, std::placeholders::_1));
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
	SmartPackage pack = net_evt->GetDataPackage();

	net_evt->SetEvent(MSG_TYPE::SendMessage);
	
	string strRes("");
	char* pExData = nullptr;
	int nExSize;
	string strMsg = pack->GetRecvMessage();
	if (m_pProcess->MsgProcess(info, strMsg , strRes, pExData, nExSize)){
		pack->ResponsePackage(strRes,pExData,nExSize);
	}else{
		m_pLog->Error("Error in process");
		pack->ResponsePackage("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
	}
	
	net_evt->SetDataPackage(pack);
	m_iC->SendMessage(net_evt);
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