#include "firewall_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
using namespace Sloong;
using namespace Sloong::Events;

void SloongNetFirewall::AfterInit()
{
	m_oConfig.ParseFromString(m_pServerConfig->exconfig());
	m_pNetwork->RegisterMessageProcesser(std::bind(&SloongNetFirewall::MessagePackageProcesser, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetFirewall::OnSocketClose, this, std::placeholders::_1));
}



void Sloong::SloongNetFirewall::MessagePackageProcesser(SmartPackage pack)
{
	pack->ResponsePackage(CResult::Make_Error("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}"));
	auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(pack->GetSocketID());
	response_event->SetDataPackage(pack);
	m_pControl->CallMessage(response_event);
}

void Sloong::SloongNetFirewall::OnSocketClose(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	// call close function.
	//m_pProcess->CloseSocket(info);
	//net_evt->CallCallbackFunc(net_evt);
}
