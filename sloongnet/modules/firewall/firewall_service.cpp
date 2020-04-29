/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-29 15:52:18
 * @Description: file content
 */
#include "firewall_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
using namespace Sloong;
using namespace Sloong::Events;


unique_ptr<SloongNetFirewall> Sloong::SloongNetFirewall::Instance = nullptr;

extern "C" CResult MessagePackageProcesser(void* pEnv,CDataTransPackage* pack)
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


extern "C" CResult CreateProcessEnvironment(void** out_env)
{
	return SloongNetFirewall::Instance->CreateProcessEnvironmentHandler(out_env);
}


CResult SloongNetFirewall::Initialized(IControl* ic)
{
	m_pControl = ic;
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetFirewall::OnSocketClose, this, std::placeholders::_1));
	return CResult::Succeed();
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


inline CResult Sloong::SloongNetFirewall::CreateProcessEnvironmentHandler(void** out_env)
{
	return CResult::Succeed();
}