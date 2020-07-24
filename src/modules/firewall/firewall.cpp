/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 14:16:47
 * @Description: file content
 */
#include "firewall.h"

#include "protocol/manager.pb.h"
using namespace Manager;


unique_ptr<SloongNetFirewall> Sloong::SloongNetFirewall::Instance = nullptr;

extern "C" PackageResult RequestPackageProcesser(void* pEnv,DataPackage* pack)
{
	return SloongNetFirewall::Instance->RequestPackageProcesser(pack);
}

extern "C" PackageResult ResponsePackageProcesser(void* pEnv,DataPackage* pack)
{
	return SloongNetFirewall::Instance->ResponsePackageProcesser(pack);
}
	
extern "C" CResult EventPackageProcesser(DataPackage* pack)
{
	SloongNetFirewall::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
}

extern "C" CResult NewConnectAcceptProcesser(SOCKET sock)
{
	return CResult::Succeed();
}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongNetFirewall::Instance = make_unique<SloongNetFirewall>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(SOCKET sock, IControl* iC){
	return SloongNetFirewall::Instance->Initialized(iC);
}


extern "C" CResult CreateProcessEnvironment(void** out_env)
{
	return SloongNetFirewall::Instance->CreateProcessEnvironmentHandler(out_env);
}


CResult SloongNetFirewall::Initialized(IControl* ic)
{
	m_iC = ic;
	return CResult::Succeed();
}


PackageResult Sloong::SloongNetFirewall::RequestPackageProcesser(DataPackage* pack)
{
    auto sender = pack->sender();
    auto func = (Functions)pack->function();
    m_pLog->Debug(Helper::Format("Porcess [%s] request: sender[%llu]", Functions_Name(func).c_str(), sender));

	return PackageResult::Succeed();
}

PackageResult Sloong::SloongNetFirewall::ResponsePackageProcesser(DataPackage* pack)
{
    auto sender = pack->sender();
    auto func = (Functions)pack->function();
    m_pLog->Debug(Helper::Format("Porcess [%s] request: sender[%llu]", Functions_Name(func).c_str(), sender));

	return PackageResult::Succeed();
}

inline CResult Sloong::SloongNetFirewall::CreateProcessEnvironmentHandler(void** out_env)
{
	return CResult::Succeed();
}



void Sloong::SloongNetFirewall::EventPackageProcesser(DataPackage* pack)
{
	auto event = Events_MIN;
	if(!Manager::Events_Parse(pack->content(),&event))
	{
		m_pLog->Error(Helper::Format("Receive event but parse error. content:[%s]",pack->content().c_str()));
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
