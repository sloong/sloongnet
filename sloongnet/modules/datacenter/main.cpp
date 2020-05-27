/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 18:25:58
 * @Description: file content
 */
#include "main.h"
#include "IData.h"
#include "utility.h"
#include "NetworkEvent.hpp"
using namespace Sloong;
using namespace Sloong::Events;

unique_ptr<SloongNetDataCenter> Sloong::SloongNetDataCenter::Instance = nullptr;

extern "C" CResult RequestPackageProcesser(CDataTransPackage* pack)
{
	return SloongNetDataCenter::Instance->RequestPackageProcesser(pack);
}
	
extern "C" CResult NewConnectAcceptProcesser(CSockInfo* info)
{

}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongNetDataCenter::Instance = make_unique<SloongNetDataCenter>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(IControl* iC){
	return SloongNetDataCenter::Instance->Initialized(iC);
}

CResult SloongNetDataCenter::Initialized(IControl*)
{
	m_oConfig.ParseFromString(m_oServerConfig->exconfig());
	m_pNetwork->RegisterRequestProcesser(std::bind(&SloongNetDataCenter::RequestPackageProcesser, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetDataCenter::OnSocketClose, this, std::placeholders::_1));
}


CResult Sloong::SloongNetDataCenter::RequestPackageProcesser(CDataTransPackage* pack)
{
    auto msgPack = pack->GetDataPackage();
    auto sender = msgPack->sender();
    auto func = msgPack->function();
    m_pLog->Debug(Helper::Format("Porcess [%s] request: sender[%llu]", Functions_Name(func).c_str(), sender));
    if (m_oFunctionHandles.exist(func))
    {
        if (!m_oFunctionHandles[func](func, sender, pack))
        {
            return CResult::Succeed();
        }
    }
    else
    {
        switch (func)
        {
        case Functions::RestartService:
        {
           
            return CResult::Succeed();
        }break;
        default:
            m_pLog->Debug(Helper::Format("No handler for [%s] request: sender[%llu]", Functions_Name(func).c_str(), sender));
            pack->ResponsePackage(ResultType::Error, "No hanlder to process request.");
        }
    }

	return CResult::Succeed();
}

void Sloong::SloongNetDataCenter::OnSocketClose(SmartEvent event)
{
}
