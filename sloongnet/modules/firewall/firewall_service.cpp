/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 14:16:47
 * @Description: file content
 */
#include "firewall_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"

#include "protocol/manager.pb.h"
using namespace Manager;

using namespace Sloong;
using namespace Sloong::Events;


unique_ptr<SloongNetFirewall> Sloong::SloongNetFirewall::Instance = nullptr;

extern "C" CResult RequestPackageProcesser(void* pEnv,CDataTransPackage* pack)
{
	return SloongNetFirewall::Instance->RequestPackageProcesser(pack);
}

extern "C" CResult ResponsePackageProcesser(void* pEnv,CDataTransPackage* pack)
{
	return SloongNetFirewall::Instance->ResponsePackageProcesser(pack);
}
	
extern "C" CResult EventPackageProcesser(CDataTransPackage* pack)
{
	SloongNetFirewall::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
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


CResult Sloong::SloongNetFirewall::RequestPackageProcesser(CDataTransPackage* pack)
{
    auto msgPack = pack->GetDataPackage();
    auto sender = msgPack->sender();
    auto func = (Functions)msgPack->function();
    m_pLog->Debug(CUniversal::Format("Porcess [%s] request: sender[%d]", Functions_Name(func), sender));

	return CResult::Succeed();
}

CResult Sloong::SloongNetFirewall::ResponsePackageProcesser(CDataTransPackage* pack)
{
    auto msgPack = pack->GetDataPackage();
    auto sender = msgPack->sender();
    auto func = (Functions)msgPack->function();
    m_pLog->Debug(CUniversal::Format("Porcess [%s] request: sender[%d]", Functions_Name(func), sender));

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



void Sloong::SloongNetFirewall::EventPackageProcesser(CDataTransPackage* trans_pack)
{
	auto event = Events_MIN;
	auto data_pack = trans_pack->GetDataPackage();
	if(!Manager::Events_Parse(data_pack->content(),&event))
	{
		m_pLog->Error(CUniversal::Format("Receive event but parse error. content:[%s]",data_pack->content()));
		return;
	}

	switch (event)
	{
	case Manager::Events::ReferenceModuleOnline:{
		m_pLog->Info("Receive ReferenceModuleOnline event");
		}break;
	case Manager::Events::ReferenceModuleOffline:{
		m_pLog->Info("Receive ReferenceModuleOffline event");
		}break;
	default:{
		}break;
	}
}
