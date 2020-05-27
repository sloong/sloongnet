/*
 * @Author: WCB
 * @Date: 2020-04-24 20:39:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-19 18:54:46
 * @Description: file content
 */
/* File Name: server.c */
#include "main.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "IData.h"
using namespace Sloong;
using namespace Sloong::Events;


unique_ptr<LuaMiddleLayer> Sloong::LuaMiddleLayer::Instance = nullptr;


extern "C" CResult RequestPackageProcesser(void* pEnv, CDataTransPackage* pack)
{
	auto pProcess = TYPE_TRANS<CLuaProcessCenter*>(pEnv);
	if( !pProcess)
	{
		LuaMiddleLayer::Instance->m_pLog->Error("Environment convert error. cannot process message.");
		return CResult::Invalid();
	}
	return LuaMiddleLayer::Instance->RequestPackageProcesser(pProcess,pack);
}

extern "C" CResult ResponsePackageProcesser(void* pEnv, CDataTransPackage* pack)
{
	auto pProcess = TYPE_TRANS<CLuaProcessCenter*>(pEnv);
	if( !pProcess)
	{
		LuaMiddleLayer::Instance->m_pLog->Error("Environment convert error. cannot process message.");
		return CResult::Invalid();
	}
	return LuaMiddleLayer::Instance->ResponsePackageProcesser(pProcess,pack);
}

extern "C" CResult EventPackageProcesser(CDataTransPackage* pack)
{
	LuaMiddleLayer::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
}
	
extern "C" CResult NewConnectAcceptProcesser(CSockInfo* info)
{
	return CResult::Succeed();
}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	LuaMiddleLayer::Instance = make_unique<LuaMiddleLayer>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(SOCKET sock,IControl* iC){
	return LuaMiddleLayer::Instance->Initialized(sock,iC);
}

extern "C" CResult CreateProcessEnvironment(void** out_env)
{
	return LuaMiddleLayer::Instance->CreateProcessEnvironmentHandler(out_env);
}


CResult LuaMiddleLayer::Initialized(SOCKET sock,IControl* iC)
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
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&LuaMiddleLayer::OnSocketClose, this, std::placeholders::_1));
	return CResult::Succeed();
}

CResult Sloong::LuaMiddleLayer::RequestPackageProcesser(CLuaProcessCenter* pProcess,CDataTransPackage* pack)
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


CResult Sloong::LuaMiddleLayer::ResponsePackageProcesser(CLuaProcessCenter* pProcess,CDataTransPackage* pack)
{
	m_pLog->Info("ResponsePackageProcesser event");
	
	return CResult::Succeed();
}

void Sloong::LuaMiddleLayer::OnSocketClose(IEvent* event)
{
}



inline CResult Sloong::LuaMiddleLayer::CreateProcessEnvironmentHandler(void** out_env)
{
	auto item = make_shared<CLuaProcessCenter>();
	item->Initialize(m_pControl);
	m_listProcess.push_back(item);
	(*out_env) = item.get();
	return CResult::Succeed();
}




void Sloong::LuaMiddleLayer::EventPackageProcesser(CDataTransPackage* trans_pack)
{
	auto data_pack = trans_pack->GetDataPackage();
	auto event = (Manager::Events)data_pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->Error(Helper::Format("Receive event but parse error. content:[%s]",data_pack->content().c_str()));
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


