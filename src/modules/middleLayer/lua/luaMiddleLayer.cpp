/*
 * @Author: WCB
 * @Date: 2020-04-24 20:39:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-19 18:54:46
 * @Description: file content
 */
/* File Name: server.c */
#include "luaMiddleLayer.h"
#include "utility.h"
#include "globalfunction.h"
#include "IData.h"
using namespace Sloong;
using namespace Sloong::Events;

unique_ptr<LuaMiddleLayer> Sloong::LuaMiddleLayer::Instance = nullptr;

extern "C" CResult RequestPackageProcesser(void *pEnv, CDataTransPackage *pack)
{
	auto pProcess = STATIC_TRANS<CLuaProcessCenter *>(pEnv);
	if (!pProcess)
	{
		LuaMiddleLayer::Instance->GetLog()->Error("Environment convert error. cannot process message.");
		return CResult::Invalid();
	}
	return LuaMiddleLayer::Instance->RequestPackageProcesser(pProcess, pack);
}

extern "C" CResult ResponsePackageProcesser(void *pEnv, CDataTransPackage *pack)
{
	auto pProcess = STATIC_TRANS<CLuaProcessCenter *>(pEnv);
	if (!pProcess)
	{
		LuaMiddleLayer::Instance->GetLog()->Error("Environment convert error. cannot process message.");
		return CResult::Invalid();
	}
	return LuaMiddleLayer::Instance->ResponsePackageProcesser(pProcess, pack);
}

extern "C" CResult EventPackageProcesser(CDataTransPackage *pack)
{
	LuaMiddleLayer::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
}

extern "C" CResult NewConnectAcceptProcesser(SOCKET sock)
{
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialization(GLOBAL_CONFIG *confiog)
{
	LuaMiddleLayer::Instance = make_unique<LuaMiddleLayer>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(IControl *iC)
{
	return LuaMiddleLayer::Instance->Initialized( iC);
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
	return LuaMiddleLayer::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult LuaMiddleLayer::Initialized(IControl *iC)
{
	IObject::Initialize(iC);
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	m_pModuleConfig = IData::GetModuleConfig();
	if (m_pModuleConfig == nullptr)
	{
		return CResult::Make_Error("No set module config. cannot go on.");
	}
	m_iC->RegisterEventHandler(SocketClose, std::bind(&LuaMiddleLayer::OnSocketClose, this, std::placeholders::_1));
	return CGlobalFunction::Instance->Initialize(m_iC);
}

CResult Sloong::LuaMiddleLayer::RequestPackageProcesser(CLuaProcessCenter *pProcess, CDataTransPackage *pack)
{
	auto msg = pack->GetDataPackage();
	CLuaPacket *info = nullptr;
	if (!m_mapUserInfoList.exist(msg->sender()))
		m_mapUserInfoList[msg->sender()] = make_unique<CLuaPacket>();

	info = m_mapUserInfoList[msg->sender()].get();
	auto function = msg->function();
	auto& content =msg->content();
	auto& extend = msg->extend();
	m_pLog->Verbos(Helper::Format("Request [%d]:[%d][%d]", function, content.length(), extend.length()));
	auto res = pProcess->MsgProcess(function, info, content, extend);
	if( res.IsFialed() )
	{
		m_pLog->Verbos(Helper::Format("Response [%d]:[%s][%s].", function, ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str()));
		pack->ResponsePackage(res);
	}
	else
	{
		auto& content = res.GetMessage();
		auto& extendUUID = res.GetResultObject();
		if (extendUUID.length() > 0)
		{
			if (m_iC->ExistTempBytes(extendUUID))
			{
				int size = 0;
				auto ptr = m_iC->GetTempBytes(extendUUID, &size);
				m_pLog->Verbos(Helper::Format("Response [%d]:[%s][%d][%d].", function, ResultType_Name(res.GetResult()).c_str(), content.length(), size));
				pack->ResponsePackage(res.GetResult(), content,ptr.get(), size);
			}
			else if (m_iC->ExistTempString(extendUUID))
			{
				auto extend = m_iC->GetTempString(extendUUID);
				m_pLog->Verbos(Helper::Format("Response [%d]:[%s][%d][%d].", function, ResultType_Name(res.GetResult()).c_str(), content.length(), extend.length()));
				pack->ResponsePackage(res.GetResult(), content, extend);
			}
			else
			{
				m_pLog->Error(Helper::Format("Response [%d]:[%s][%d][Message is required an extend UUID. but not find in IControl. Ignore.]", function, ResultType_Name(res.GetResult()).c_str(), content.length()));
				pack->ResponsePackage(res.GetResult(),content);
			}
		}
		else
		{
			m_pLog->Verbos(Helper::Format("Response [%d]:[%s][%d].", function, ResultType_Name(res.GetResult()).c_str(), content.length()));
			pack->ResponsePackage(res);
		}
	}
	return CResult::Succeed();
}

CResult Sloong::LuaMiddleLayer::ResponsePackageProcesser(CLuaProcessCenter *pProcess, CDataTransPackage *pack)
{
	m_pLog->Info("ResponsePackageProcesser event");

	return CResult::Succeed();
}

void Sloong::LuaMiddleLayer::OnSocketClose(SharedEvent event)
{
}

inline CResult Sloong::LuaMiddleLayer::CreateProcessEnvironmentHandler(void **out_env)
{
	auto item = make_shared<CLuaProcessCenter>();
	auto res = item->Initialize(m_iC);
	if (res.IsFialed())
		return res;
	m_listProcess.push_back(item);
	(*out_env) = item.get();
	return CResult::Succeed();
}


void Sloong::LuaMiddleLayer::OnReferenceModuleOnlineEvent(const string &str_req, CDataTransPackage *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOnline event");
	/*auto req = ConvertStrToObj<Manager::EventReferenceModuleOnline>(str_req);
	auto item = req->item();
	m_mapUUIDToNode[item.uuid()] = item;
	m_mapTempteIDToUUIDs[item.templateid()].push_back(item.uuid());
	m_pLog->Debug(Helper::Format("New module is online:[%llu][%s:%d]", item.uuid(), item.address().c_str(), item.port()));

	AddConnection(item.uuid(), item.address(), item.port());*/
}

void Sloong::LuaMiddleLayer::OnReferenceModuleOfflineEvent(const string &str_req, CDataTransPackage *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOffline event");
}

void Sloong::LuaMiddleLayer::EventPackageProcesser(CDataTransPackage *trans_pack)
{
	auto data_pack = trans_pack->GetDataPackage();
	auto event = (Manager::Events)data_pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->Error(Helper::Format("Receive event but parse error. content:[%s]", data_pack->content().c_str()));
		return;
	}

	switch (event)
	{
	case Manager::Events::ReferenceModuleOnline:
	{
		OnReferenceModuleOnlineEvent(data_pack->content(), trans_pack);
	}
	break;
	case Manager::Events::ReferenceModuleOffline:
	{
		OnReferenceModuleOfflineEvent(data_pack->content(), trans_pack);
	}
	break;
	default:
	{
		m_pLog->Error(Helper::Format("Event is no processed. [%s][%d].", Manager::Events_Name(event).c_str(), event));
	}
	break;
	}
}
