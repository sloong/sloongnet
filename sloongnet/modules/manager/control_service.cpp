/*
 * @Author: WCB
 * @LastEditors: WCB
 * @Description: Control center service 
 * @Date: 2019-04-14 14:41:59
 * @LastEditTime: 2020-04-16 20:45:07
 */

#include "control_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "fstream_ex.hpp"


using namespace Sloong::Events;

unique_ptr<SloongControlService> Sloong::SloongControlService::Instance = nullptr;

extern "C" CResult MessagePackageProcesserHandler(CDataTransPackage* pack)
{
	return SloongControlService::Instance->MessagePackageProcesser(pack);
}
	
extern "C" CResult NewConnectAcceptProcesserHandler(CSockInfo* info)
{

}
	
extern "C" CResult ModuleInitializeHandler(IControl* iC){
	SloongControlService::Instance = make_unique<SloongControlService>();
	return SloongControlService::Instance->Initialize(iC);
}


/**
 * @Remarks: If have errors, the error message will print to standard output device.
 * @Params: CMD line. from System.
 * @Return: return true if no error, service can continue to run.
 * 			return false if error. service must exit. 
 * 
 * 
 */
CResult SloongControlService::Initialize(IControl* iC)
{
	m_pControl = iC;
	m_pConfig = (GLOBAL_CONFIG*)(iC->Get(DATA_ITEM::ServerConfiguation));
	m_pLog = (CLog*)(iC->Get(DATA_ITEM::Logger));
	
	auto res = m_pServer->Initialize(0);
	if (res.IsFialed())
	{
		cout << "Init server manage fialed. error message:" << res.Message() << endl;
		return res;
	}

	auto config_str = res.Message();
	// Here, this port is came from COMMAND LINE. 
	// So we need save it before parse other setting.
	auto port = m_pConfig->listenport();
	if (config_str.length() == 0 || !m_pConfig->ParseFromString(config_str))
	{
		// If parse config error, run with default config.
		cout <<  "Parser server config error. run with default setting." << endl;
		ResetControlConfig(m_pConfig);
	}
	// When config parse done, revert the port setting. Because we always use the command line port.
	m_pConfig->set_listenport(port);

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