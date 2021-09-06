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

#include "events/ModuleOnOff.hpp"
using namespace Sloong::Events;

unique_ptr<LuaMiddleLayer> Sloong::LuaMiddleLayer::Instance = nullptr;

extern "C" PackageResult RequestPackageProcesser(void *pEnv, Package *pack)
{
	auto pProcess = STATIC_TRANS<CLuaProcessCenter *>(pEnv);
	if (!pProcess)
	{
		LuaMiddleLayer::Instance->GetLog()->Error("Environment convert error. cannot process message.");
		return PackageResult::Invalid();
	}
	return LuaMiddleLayer::Instance->RequestPackageProcesser(pProcess, pack);
}

extern "C" PackageResult ResponsePackageProcesser(void *pEnv, Package *pack)
{
	auto pProcess = STATIC_TRANS<CLuaProcessCenter *>(pEnv);
	if (!pProcess)
	{
		LuaMiddleLayer::Instance->GetLog()->Error("Environment convert error. cannot process message.");
		return PackageResult::Invalid();
	}
	return LuaMiddleLayer::Instance->ResponsePackageProcesser(pProcess, pack);
}

extern "C" CResult EventPackageProcesser(Package *pack)
{
	LuaMiddleLayer::Instance->EventPackageProcesser(pack);
	return CResult::Succeed;
}

extern "C" CResult NewConnectAcceptProcesser(SOCKET sock)
{
	return CResult::Succeed;
}

extern "C" CResult ModuleInitialization(IControl* ic)
{
	LuaMiddleLayer::Instance = make_unique<LuaMiddleLayer>();
	return LuaMiddleLayer::Instance->Initialization(ic);
}

extern "C" CResult ModuleInitialized()
{
	return LuaMiddleLayer::Instance->Initialized();
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
	return LuaMiddleLayer::Instance->CreateProcessEnvironmentHandler(out_env);
}


CResult LuaMiddleLayer::Initialization(IControl *ic)
{
	IObject::Initialize(ic);
	IData::Initialize(ic);
	return CResult::Succeed;
}

CResult LuaMiddleLayer::Initialized()
{
	m_pConfig = IData::GetGlobalConfig();
	m_pModuleConfig = IData::GetModuleConfig();
	if (m_pModuleConfig == nullptr)
	{
		return CResult::Make_Error("No set module config. cannot go on.");
	}
	m_iC->RegisterEventHandler(EVENT_TYPE::ConnectionBreaked, std::bind(&LuaMiddleLayer::OnConnectionBreaked, this, std::placeholders::_1));
	auto res = CGlobalFunction::Instance->Initialize(m_iC);
	if( res.IsFialed() )
		return res;
	auto item = make_shared<CLuaProcessCenter>();
	res = item->Initialize(m_iC);
	if (res.IsFialed())
		return res;
	return res;
}

PackageResult Sloong::LuaMiddleLayer::RequestPackageProcesser(CLuaProcessCenter *pProcess, Package *pack)
{
	CLuaPacket *info = nullptr;
	if (!m_mapUserInfoList.exist(pack->sender()))
		m_mapUserInfoList[pack->sender()] = make_unique<CLuaPacket>();

	info = m_mapUserInfoList[pack->sender()].get();
	auto function = pack->function();
	auto& content =pack->content();
	auto& extend = pack->extend();
	m_pLog->Info(format("Request [{}]:[{}][{}]", function, content.length(), extend.length()));
	auto res = pProcess->MsgProcess(function, info, content, extend);
	if( res.IsFialed() )
	{
		m_pLog->Warn(format("Response [{}]:[{}][{}].", function, ResultType_Name(res.GetResult()), res.GetMessage()));
		return PackageResult::Make_OKResult(Package::MakeResponse(pack,res));
	}
	else
	{
		auto& content = res.GetMessage();
		auto& extendUUID = res.GetResultObject();
		if (!extendUUID.empty())
		{
			if (m_iC->ExistTempBytes(extendUUID))
			{
				int size = 0;
				auto ptr = m_iC->GetTempBytes(extendUUID, &size);
				m_pLog->Info(format("Response [{}]:[{}][{}][{}].", function, ResultType_Name(res.GetResult()), content.length(), size));
				return PackageResult::Make_OKResult(Package::MakeResponse(pack,res.GetResult(), content, ptr.get(), size));
			}
			else if (m_iC->ExistTempString(extendUUID))
			{
				auto extend = m_iC->GetTempString(extendUUID);
				m_pLog->Info(format("Response [{}]:[{}][{}][{}].", function, ResultType_Name(res.GetResult()), content.length(), extend.length()));
				return PackageResult::Make_OKResult(Package::MakeResponse(pack,res.GetResult(), content, extend));
			}
			else
			{
				m_pLog->Error(format("Response [{}]:[{}][{}][Message is required an extend UUID[{}]. but not find in IControl. Ignore.]", function, ResultType_Name(res.GetResult()), content.length(), extendUUID));
				return PackageResult::Make_OKResult(Package::MakeResponse(pack,res));
			}
		}
		else
		{
			m_pLog->Info(format("Response [{}]:[{}][{}].", function, ResultType_Name(res.GetResult()), content.length()));
			return PackageResult::Make_OKResult(Package::MakeResponse(pack,res));
		}
	}
}

PackageResult Sloong::LuaMiddleLayer::ResponsePackageProcesser(CLuaProcessCenter *pProcess, Package *pack)
{
	m_pLog->Info("ResponsePackageProcesser event");

	return PackageResult::Ignore();
}

void Sloong::LuaMiddleLayer::OnConnectionBreaked(SharedEvent event)
{
}

inline CResult Sloong::LuaMiddleLayer::CreateProcessEnvironmentHandler(void **out_env)
{
	auto item = make_shared<CLuaProcessCenter>();
	auto res = item->Initialize(m_iC);
	if (res.IsFialed())
		return res;

	(*out_env) = item.get();
	m_listProcess.emplace_back(move(item));
	return CResult::Succeed;
}


void Sloong::LuaMiddleLayer::OnReferenceModuleOnlineEvent(const string &str_req, Package *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOnline event");
	auto event = make_shared<ModuleOnlineEvent>(LUA_EVENT_TYPE::OnReferenceModuleOnline);
	auto info = event->GetInfos();
	info->ParseFromString(str_req);
	m_iC->SendMessage(event);
}

void Sloong::LuaMiddleLayer::OnReferenceModuleOfflineEvent(const string &str_req, Package *trans_pack)
{
	m_pLog->Info("Receive ReferenceModuleOffline event");
	auto event = make_shared<ModuleOfflineEvent>(LUA_EVENT_TYPE::OnReferenceModuleOffline);
	auto info = event->GetInfos();
	info->ParseFromString(str_req);
	m_iC->SendMessage(event);
}

void Sloong::LuaMiddleLayer::EventPackageProcesser(Package *pack)
{
	auto event = (Manager::Events)pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->Error(format("Receive event but parse error. content:[{}]", pack->content()));
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
		m_pLog->Error(format("Event is no processed. [{}][{}].", Manager::Events_Name(event), event));
	}
	break;
	}
}


void Sloong::LuaMiddleLayer::SetReloadScriptFlag()
{
	for( auto& item : m_listProcess)
	{
		item->ReloadContext();
	}
}