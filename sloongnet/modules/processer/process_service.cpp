/* File Name: server.c */
#include "process_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "IData.h"
using namespace Sloong;
using namespace Sloong::Events;


unique_ptr<SloongNetProcess> Sloong::SloongNetProcess::Instance = nullptr;


extern "C" CResult MessagePackageProcesser(CDataTransPackage* pack)
{
	return SloongNetProcess::Instance->MessagePackageProcesser(pack);
}
	
extern "C" CResult NewConnectAcceptProcesser(CSockInfo* info)
{
	return CResult::Succeed();
}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongNetProcess::Instance = make_unique<SloongNetProcess>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(IControl* iC){
	return SloongNetProcess::Instance->Initialized(iC);
}


CResult SloongNetProcess::Initialized(IControl* iC)
{
	m_pControl = iC;
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	Json::Reader reader;
	if ( m_pConfig->moduleconfig().length() > 0 && reader.parse(m_pConfig->moduleconfig(), m_oExConfig))
	{
		
	}
	m_pLog = IData::GetLog();
	
	RegistFunctionHandler(Functions::ProcessMessage, std::bind(&SloongNetProcess::ProcessMessageHanlder, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetProcess::OnSocketClose, this, std::placeholders::_1));
	m_pProcess->Initialize(m_pControl);
	return CResult::Succeed();
}


void Sloong::SloongNetProcess::RegistFunctionHandler(Functions func, FuncHandler handler)
{
    m_oFunctionHandles[func] = handler;
}


CResult Sloong::SloongNetProcess::MessagePackageProcesser(CDataTransPackage* pack)
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
            m_pControl->SendMessage(EVENT_TYPE::ProgramRestart);
            return CResult::Succeed();
        }break;
        default:
            m_pLog->Verbos(CUniversal::Format("No handler for [%s] request: sender[%d]", Functions_Name(func), sender));
            pack->ResponsePackage(ResultType::Error, "No hanlder to process request.");
        }
    }

	return CResult::Succeed();
}

bool Sloong::SloongNetProcess::ProcessMessageHanlder(Functions func, string sender, CDataTransPackage* pack)
{	
	string strRes("");
	char* pExData = nullptr;
	int nExSize;

	auto msg = pack->GetRecvPackage();
	string uuid = msg->extend();
	auto infoItem = m_mapUserInfoList.find(uuid);
	if (infoItem == m_mapUserInfoList.end())
	{
		m_mapUserInfoList[uuid] = make_unique<CLuaPacket>();
		infoItem = m_mapUserInfoList.find(uuid);
	}
	if (m_pProcess->MsgProcess(infoItem->second.get(), msg->content(), strRes, pExData, nExSize)) {
		msg->set_content(strRes);
	}
	else {
		m_pLog->Error("Error in process");
		msg->set_content("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
	}
	pack->ResponsePackage(msg);
	return true;
}

void Sloong::SloongNetProcess::OnSocketClose(SmartEvent event)
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
