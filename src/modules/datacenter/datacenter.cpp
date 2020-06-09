/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 18:25:58
 * @Description: file content
 */
#include "datacenter.h"
#include "IData.h"
#include "NetworkEvent.hpp"
using namespace Sloong;
using namespace Sloong::Events;

#include "protocol/manager.pb.h"

unique_ptr<CDataCenter> Sloong::CDataCenter::Instance = nullptr;

extern "C" CResult RequestPackageProcesser(void *env, CDataTransPackage *pack)
{
	auto pDB = TYPE_TRANS<DBHub *>(env);
	if (pDB)
		return pDB->RequestPackageProcesser(pack);
	else
		return CResult::Make_Error("RequestPackageProcesser error, Environment convert failed.");
}

extern "C" CResult ResponsePackageProcesser(void *env, CDataTransPackage *pack)
{
	/*auto pDB = TYPE_TRANS<DBHub *>(env);
	if (pDB)
		return pDB->ResponsePackageProcesser(pack);
	else
		return CResult::Make_Error("ResponsePackageProcesser error, Environment convert failed.");*/
	return CResult::Make_Error("NO SUPPORT!");
}

extern "C" CResult EventPackageProcesser(CDataTransPackage *pack)
{
	CDataCenter::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
}

extern "C" CResult NewConnectAcceptProcesser(CSockInfo *info)
{
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialization(GLOBAL_CONFIG *confiog)
{
	CDataCenter::Instance = make_unique<CDataCenter>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(SOCKET sock, IControl *iC)
{
	return CDataCenter::Instance->Initialized(sock, iC);
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
	return CDataCenter::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult CDataCenter::Initialized(SOCKET sock, IControl *ic)
{
	IObject::Initialize(ic);
	IData::Initialize(ic);
	m_pConfig = IData::GetGlobalConfig();
	m_pModuleConfig = IData::GetModuleConfig();
	m_pRuntimeData = IData::GetRuntimeData();
	if (m_pModuleConfig == nullptr)
	{
		return CResult::Make_Error("Initialize error. no config data.");
	}
	m_nManagerConnection = sock;

	return CResult::Succeed();
}

CResult CDataCenter::CreateProcessEnvironmentHandler(void **out_env)
{
	auto item = make_unique<DBHub>();
	auto res = item->Initialize(m_iC);
	if (res.IsFialed())
		return res;
	(*out_env) = item.get();
	m_listDBHub.push_back(std::move(item));
	return CResult::Succeed();
}

void Sloong::CDataCenter::EventPackageProcesser(CDataTransPackage *trans_pack)
{
	auto data_pack = trans_pack->GetDataPackage();
	auto event = (Manager::Events)data_pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->Error(Helper::Format("EventPackageProcesser is called.but the fucntion[%d] check error.", event));
		return;
	}

	switch (event)
	{
	default:
	{
		m_pLog->Error(Helper::Format("Event is no processed. [%s][%d].", Manager::Events_Name(event).c_str(), event));
	}
	break;
	}
}