/*
 * @Author: WCB
 * @Date: 2020-04-24 20:39:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-19 18:54:46
 * @Description: file content
 */
/* File Name: server.c */
#include "process_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "IData.h"
using namespace Sloong;
using namespace Sloong::Events;


unique_ptr<SloongNetProcess> Sloong::SloongNetProcess::Instance = nullptr;


extern "C" CResult RequestPackageProcesser(void* pEnv, CDataTransPackage* pack)
{
	auto pProcess = TYPE_TRANS<CLuaProcessCenter*>(pEnv);
	if( !pProcess)
	{
		SloongNetProcess::Instance->m_pLog->Error("Environment convert error. cannot process message.");
		return CResult::Invalid();
	}
	return SloongNetProcess::Instance->RequestPackageProcesser(pProcess,pack);
}

extern "C" CResult ResponsePackageProcesser(void* pEnv, CDataTransPackage* pack)
{
	auto pProcess = TYPE_TRANS<CLuaProcessCenter*>(pEnv);
	if( !pProcess)
	{
		SloongNetProcess::Instance->m_pLog->Error("Environment convert error. cannot process message.");
		return CResult::Invalid();
	}
	return SloongNetProcess::Instance->ResponsePackageProcesser(pProcess,pack);
}

extern "C" CResult EventPackageProcesser(CDataTransPackage* pack)
{
	SloongNetProcess::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
}
	
extern "C" CResult NewConnectAcceptProcesser(CSockInfo* info)
{
	return CResult::Succeed();
}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongNetProcess::Instance = make_unique<SloongNetProcess>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(SOCKET sock,IControl* iC){
	return SloongNetProcess::Instance->Initialized(sock,iC);
}

extern "C" CResult CreateProcessEnvironment(void** out_env)
{
	return SloongNetProcess::Instance->CreateProcessEnvironmentHandler(out_env);
}


CResult SloongNetProcess::Initialized(SOCKET sock,IControl* iC)
{
	m_pControl = iC;
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	m_pModuleConfig = IData::GetModuleConfig();
	if ( m_pModuleConfig == nullptr )
	{
		return CResult::Make_Error("No set module config. cannot go on.");
	}
	m_pLog = IData::GetLog();
	m_nManagerConnection = sock;
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetProcess::OnSocketClose, this, std::placeholders::_1));
	return CResult::Succeed();
}

CResult Sloong::SloongNetProcess::RequestPackageProcesser(CLuaProcessCenter* pProcess,CDataTransPackage* pack)
{
    string strRes("");
	char* pExData = nullptr;
	int nExSize;

	auto msg = pack->GetDataPackage();
	string uuid = msg->extend();
	auto infoItem = m_mapUserInfoList.find(uuid);
	if (infoItem == m_mapUserInfoList.end())
	{
		m_mapUserInfoList[uuid] = make_unique<CLuaPacket>();
		infoItem = m_mapUserInfoList.find(uuid);
	}
	if (!pProcess->MsgProcess(infoItem->second.get(), msg->content(), strRes, pExData, nExSize)) {
		strRes = "{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}";
	}
	pack->ResponsePackage( ResultType::Succeed, strRes);
	return CResult::Succeed();
}


CResult Sloong::SloongNetProcess::ResponsePackageProcesser(CLuaProcessCenter* pProcess,CDataTransPackage* pack)
{
	m_pLog->Info("ResponsePackageProcesser event");
	
	return CResult::Succeed();
}

void Sloong::SloongNetProcess::OnSocketClose(IEvent* event)
{
}



inline CResult Sloong::SloongNetProcess::CreateProcessEnvironmentHandler(void** out_env)
{
	auto item = make_shared<CLuaProcessCenter>();
	item->Initialize(m_pControl);
	m_listProcess.push_back(item);
	(*out_env) = item.get();
	return CResult::Succeed();
}




void Sloong::SloongNetProcess::EventPackageProcesser(CDataTransPackage* trans_pack)
{
	auto event = Events_MIN;
	auto data_pack = trans_pack->GetDataPackage();
	if(!Manager::Events_Parse(data_pack->content(),&event))
	{
		m_pLog->Error(Helper::Format("Receive event but parse error. content:[%s]",data_pack->content().c_str()));
		return;
	}

	switch (event)
	{
	case Manager::Events::RestartNode:{
		m_pControl->SendMessage(EVENT_TYPE::ProgramRestart);
		}break;
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


