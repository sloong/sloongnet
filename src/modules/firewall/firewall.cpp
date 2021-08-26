/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 14:16:47
 * @Description: file content
 */
#include "firewall.h"
#include "IData.h"

#include "protocol/manager.pb.h"
using namespace Manager;


unique_ptr<SloongNetFirewall> Sloong::SloongNetFirewall::Instance = nullptr;

extern "C" PackageResult RequestPackageProcesser(void* pEnv,Package* pack)
{
	return SloongNetFirewall::Instance->RequestPackageProcesser(pack);
}

extern "C" PackageResult ResponsePackageProcesser(void* pEnv,Package* pack)
{
	return SloongNetFirewall::Instance->ResponsePackageProcesser(pack);
}

extern "C" CResult EventPackageProcesser(Package* pack)
{
	SloongNetFirewall::Instance->EventPackageProcesser(pack);
	return CResult::Succeed;
}

extern "C" CResult NewConnectAcceptProcesser(SOCKET sock)
{
	return CResult::Succeed;
}
	
extern "C" CResult ModuleInitialization(IControl* ic){
	SloongNetFirewall::Instance = make_unique<SloongNetFirewall>();
	return SloongNetFirewall::Instance->Initialization(ic);
}

extern "C" CResult ModuleInitialized(){
	return SloongNetFirewall::Instance->Initialized();
}


extern "C" CResult CreateProcessEnvironment(void** out_env)
{
	return SloongNetFirewall::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult SloongNetFirewall::Initialization(IControl* ic)
{
	IObject::Initialize(ic);
	IData::Initialize(ic);
	return CResult::Succeed;
}


CResult SloongNetFirewall::Initialized()
{
	return CResult::Succeed;
}


PackageResult Sloong::SloongNetFirewall::RequestPackageProcesser(Package* pack)
{
    auto sender = pack->sender();
    auto func = (Functions)pack->function();
    m_pLog->Debug(format("Porcess [{}] request: sender[{}]", Functions_Name(func), sender));

	return PackageResult::Succeed();
}

PackageResult Sloong::SloongNetFirewall::ResponsePackageProcesser(Package* pack)
{
    auto sender = pack->sender();
    auto func = (Functions)pack->function();
    m_pLog->Debug(format("Porcess [{}] request: sender[{}]", Functions_Name(func), sender));

	return PackageResult::Succeed();
}

inline CResult Sloong::SloongNetFirewall::CreateProcessEnvironmentHandler(void** out_env)
{
	return CResult::Succeed;
}



void Sloong::SloongNetFirewall::EventPackageProcesser(Package* pack)
{
	auto event = Events_MIN;
	if(!Manager::Events_Parse(pack->content(),&event))
	{
		m_pLog->Error(format("Receive event but parse error. content:[{}]",pack->content()));
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
