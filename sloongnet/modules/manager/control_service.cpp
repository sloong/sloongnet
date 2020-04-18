/*
 * @Author: WCB
 * @LastEditors: WCB
 * @Description: Control center service 
 * @Date: 2019-04-14 14:41:59
 * @LastEditTime: 2020-04-17 21:48:09
 */

#include "control_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "fstream_ex.hpp"


using namespace Sloong::Events;

unique_ptr<SloongControlService> Sloong::SloongControlService::Instance = nullptr;

extern "C" CResult MessagePackageProcesser(CDataTransPackage* pack)
{
	return SloongControlService::Instance->MessagePackageProcesser(pack);
}
	
extern "C" CResult NewConnectAcceptProcesser(CSockInfo* info)
{

}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongControlService::Instance = make_unique<SloongControlService>();
	return SloongControlService::Instance->Initialization(confiog);
}

extern "C" CResult ModuleInitialized(IControl* iC){
	return SloongControlService::Instance->Initialized(iC);
}


/**
 * @Remarks: If have errors, the error message will print to standard output device.
 * @Params: CMD line. from System.
 * @Return: return true if no error, service can continue to run.
 * 			return false if error. service must exit. 
 * 
 * 
 */
CResult SloongControlService::Initialization(GLOBAL_CONFIG* config)
{
	if( config == nullptr )
	{
		return CResult::Make_Error("Config object is nullptr.");
	}
	auto res = m_pServer->Initialize(0);
	if (res.IsFialed())
	{
		return CResult::Make_Error(CUniversal::Format("Init server manage fialed. error message:%s",res.Message()));
	}

	auto config_str = res.Message();
	// Here, this port is came from COMMAND LINE. 
	// So we need save it before parse other setting.
	auto port = config->listenport();
	if (config_str.length() == 0 || !config->ParseFromString(config_str))
	{
		// If parse config error, run with default config.
		cout <<  "Parser server config error. run with default setting." << endl;
		ResetControlConfig(config);
	}
	// When config parse done, revert the port setting. Because we always use the command line port.
	config->set_listenport(port);
	
	return CResult::Succeed();
}

CResult SloongControlService::Initialized(IControl* iC)
{
	m_pControl = iC;
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	m_pLog = IData::GetLog();
	
	m_pServer->SetLog(m_pLog);
	RegistFunctionHandler(Functions::RegisteServer, std::bind(&CServerManage::RegisterServerHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	RegistFunctionHandler(Functions::GetTemplateList, std::bind(&CServerManage::GetTemplateListHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	RegistFunctionHandler(Functions::GetServerList , std::bind(&CServerManage::GetServerListHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	RegistFunctionHandler(Functions::SetTemplateConfig, std::bind(&CServerManage::SetTemplateConfigHandler, m_pServer.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	return CResult::Succeed();
}

void Sloong::SloongControlService::ResetControlConfig(GLOBAL_CONFIG* config)
{
	config->set_logpath("/data/log");
	config->set_loglevel(Protocol::LogLevel::Info);
	config->set_debugmode(false);
	config->set_mqthreadquantity(1);
	config->set_enablessl(false);
	config->set_epollthreadquantity(1);
	config->set_processthreadquantity(1);
	config->set_receivetime(3);
}


void Sloong::SloongControlService::RegistFunctionHandler(Functions func, FuncHandler handler)
{
    m_oFunctionHandles[func] = handler;
}


CResult Sloong::SloongControlService::MessagePackageProcesser(CDataTransPackage* pack)
{
    auto msgPack = pack->GetRecvPackage();
    auto sender = msgPack->sender();
    auto func = msgPack->function();
    m_pLog->Verbos(CUniversal::Format("Porcess [%s] request: sender[%d]", Functions_Name(func), sender));
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
            m_pLog->Verbos(CUniversal::Format("No handler for [%s] request: sender[%d]", Functions_Name(func), sender));
            pack->ResponsePackage(ResultType::Error, "No hanlder to process request.");
        }
    }

	return CResult::Succeed();
}