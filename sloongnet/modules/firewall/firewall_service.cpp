/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-24 17:53:06
 * @Description: file content
 */
#include "firewall_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
using namespace Sloong;
using namespace Sloong::Events;


unique_ptr<SloongNetFirewall> Sloong::SloongNetFirewall::Instance = nullptr;

extern "C" CResult MessagePackageProcesser(CDataTransPackage* pack)
{
	return SloongNetFirewall::Instance->MessagePackageProcesser(pack);
}
	
extern "C" CResult NewConnectAcceptProcesser(CSockInfo* info)
{

}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongNetFirewall::Instance = make_unique<SloongNetFirewall>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(IControl* iC){
	return SloongNetFirewall::Instance->Initialized(iC);
}

CResult SloongNetFirewall::Initialized(IControl*)
{
	m_oConfig.ParseFromString(m_pServerConfig->exconfig());
	m_pNetwork->RegisterMessageProcesser(std::bind(&SloongNetFirewall::MessagePackageProcesser, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetFirewall::OnSocketClose, this, std::placeholders::_1));
}



void Sloong::SloongNetFirewall::RegistFunctionHandler(Functions func, FuncHandler handler)
{
    m_oFunctionHandles[func] = handler;
}


CResult Sloong::SloongNetFirewall::MessagePackageProcesser(CDataTransPackage* pack)
{
    auto msgPack = pack->GetRecvPackage();
    auto sender = msgPack->sender();
    auto func = msgPack->function();
    m_pLog->Verbos(CUniversal::Format("Porcess [%s] request: sender[%d]", Functions_Name(func), sender));

	return CResult::Succeed();
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
