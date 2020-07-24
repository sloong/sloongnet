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

extern "C" PackageResult RequestPackageProcesser(void *pEnv, DataPackage *pack)
{
	auto pProcess = STATIC_TRANS<CLuaProcessCenter *>(pEnv);
	if (!pProcess)
	{
		LuaMiddleLayer::Instance->GetLog()->Error("Environment convert error. cannot process message.");
		return PackageResult::Invalid();
	}
	return LuaMiddleLayer::Instance->RequestPackageProcesser(pProcess, pack);
}

extern "C" PackageResult ResponsePackageProcesser(void *pEnv, DataPackage *pack)
{
	auto pProcess = STATIC_TRANS<CLuaProcessCenter *>(pEnv);
	if (!pProcess)
	{
		LuaMiddleLayer::Instance->GetLog()->Error("Environment convert error. cannot process message.");
		return PackageResult::Invalid();
	}
	return LuaMiddleLayer::Instance->ResponsePackageProcesser(pProcess, pack);
}

extern "C" CResult EventPackageProcesser(DataPackage *pack)
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
	m_iC->RegisterEventHandler(EVENT_TYPE::ConnectionBreak, std::bind(&LuaMiddleLayer::OnConnectionBreak, this, std::placeholders::_1));
	return CGlobalFunction::Instance->Initialize(m_iC);
}

PackageResult Sloong::LuaMiddleLayer::RequestPackageProcesser(CLuaProcessCenter *pProcess, DataPackage *pack)
{
	CLuaPacket *info = nullptr;
	if (!m_mapUserInfoList.exist(pack->sender()))
		m_mapUserInfoList[pack->sender()] = make_unique<CLuaPacket>();

	info = m_mapUserInfoList[pack->sender()].get();
	auto function = pack->function();
	auto& content =pack->content();
	auto& extend = pack->extend();
	m_pLog->Verbos(Helper::Format("Request [%d]:[%d][%d]", function, content.length(), extend.length()));
	auto res = pProcess->MsgProcess(function, info, content, extend);
	if( res.IsFialed() )
	{
		m_pLog->Verbos(Helper::Format("Response [%d]:[%s][%s].", function, ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str()));
		return PackageResult::Make_OK(Package::MakeResponse(pack,res));
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
				return PackageResult::Make_OK(Package::MakeResponse(pack,res.GetResult(), content, ptr.get(), size));
			}
			else if (m_iC->ExistTempString(extendUUID))
			{
				auto extend = m_iC->GetTempString(extendUUID);
				m_pLog->Verbos(Helper::Format("Response [%d]:[%s][%d][%d].", function, ResultType_Name(res.GetResult()).c_str(), content.length(), extend.length()));
				return PackageResult::Make_OK(Package::MakeResponse(pack,res.GetResult(), content, extend));
			}
			else
			{
				m_pLog->Error(Helper::Format("Response [%d]:[%s][%d][Message is required an extend UUID. but not find in IControl. Ignore.]", function, ResultType_Name(res.GetResult()).c_str(), content.length()));
				return PackageResult::Make_OK(Package::MakeResponse(pack,res));
			}
		}
		else
		{
			m_pLog->Verbos(Helper::Format("Response [%d]:[%s][%d].", function, ResultType_Name(res.GetResult()).c_str(), content.length()));
			return PackageResult::Make_OK(Package::MakeResponse(pack,res));
		}
	}
}

PackageResult Sloong::LuaMiddleLayer::ResponsePackageProcesser(CLuaProcessCenter *pProcess, DataPackage *pack)
{
	m_pLog->Info("ResponsePackageProcesser event");

	return PackageResult::Succeed();
}

void Sloong::LuaMiddleLayer::OnConnectionBreak(SharedEvent event)
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


void Sloong::LuaMiddleLayer::OnReferenceModuleOnlineEvent(const string &str_req, DataPackage *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOnline event");
	/*auto req = ConvertStrToObj<Manager::EventReferenceModuleOnline>(str_req);
	auto item = req->item();
	m_mapUUIDToNode[item.uuid()] = item;
	m_mapTempteIDToUUIDs[item.templateid()].push_back(item.uuid());
	m_pLog->Debug(Helper::Format("New module is online:[%llu][%s:%d]", item.uuid(), item.address().c_str(), item.port()));

	AddConnection(item.uuid(), item.address(), item.port());*/
}

void Sloong::LuaMiddleLayer::OnReferenceModuleOfflineEvent(const string &str_req, DataPackage *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOffline event");
}

void Sloong::LuaMiddleLayer::EventPackageProcesser(DataPackage *pack)
{
	auto event = (Manager::Events)pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->Error(Helper::Format("Receive event but parse error. content:[%s]", pack->content().c_str()));
		return;
	}

	switch (event)
	{
	case Manager::Events::ReferenceModuleOnline:
	{
		OnReferenceModuleOnlineEvent(pack->content(), pack);
	}
	break;
	case Manager::Events::ReferenceModuleOffline:
	{
		OnReferenceModuleOfflineEvent(pack->content(), pack);
	}
	break;
	default:
	{
		m_pLog->Error(Helper::Format("Event is no processed. [%s][%d].", Manager::Events_Name(event).c_str(), event));
	}
	break;
	}
}
