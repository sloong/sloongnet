/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-04-28 14:43:16
 * @LastEditTime: 2021-01-11 10:48:00
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/gateway/transpond.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#include "transpond.h"
#include "package.hpp"
#include "IData.h"
#include "gateway.h"
#include "snowflake.h"

CResult Sloong::GatewayTranspond::Initialize(IControl* ic)
{
    m_pLog = IData::GetLog();
    m_iC = ic;
    return CResult::Succeed;
}

PackageResult GatewayTranspond::RequestPackageProcesser(Package *trans_pack)
{
	return MessageToProcesser(trans_pack);
}

PackageResult GatewayTranspond::ResponsePackageProcesser( UniquePackage info, Package *trans_pack)
{
	return MessageToClient(move(info),trans_pack);
}


PackageResult Sloong::GatewayTranspond::MessageToProcesser(Package *pack)
{
	m_pLog->Debug("Receive new request package.");
	auto target = SloongNetGateway::Instance->GetPorcessConnection(pack->function());
	if( target == 0 )
	{
		auto msg = Helper::Format("No find process service for function[%d]. package [%d][%lld]", pack->function(), pack->sessionid(), pack->id() );
		m_pLog->Debug(msg);
		return PackageResult::Make_Error(msg);
	}

	auto response = PackageHelper::MakeResponse(pack);
	for( auto i : pack->clocks() )
	{
		response->add_clocks(i);
	}
	response->add_clocks(GetClock());

	auto trans_pack = PackageHelper::MakeResponse(pack);
	trans_pack->set_status(DataPackage_StatusType::DataPackage_StatusType_Request);
	PackageHelper::SetContent(trans_pack.get(), pack->content() );
	PackageHelper::SetExtend( trans_pack.get(), pack->extend() );

	auto id = snowflake::Instance->nextid();
	trans_pack->set_id(id);
	trans_pack->set_sessionid(target);
	
	m_pLog->Debug(Helper::Format("Trans package [%lld][%lld] -> [%d][%lld]", pack->sessionid(), pack->id(), trans_pack->sessionid(), trans_pack->id()));

	SloongNetGateway::Instance->m_mapSerialToRequest[trans_pack->id()] = move(response);
	return PackageResult::Make_OKResult(move(trans_pack));
}

PackageResult Sloong::GatewayTranspond::MessageToClient(UniquePackage info, Package *pack)
{
	info->add_clocks(GetClock());
	info->set_result(pack->result());
	PackageHelper::SetContent(info.get(), pack->content() );
	PackageHelper::SetExtend( info.get(), pack->extend() );
	
	return PackageResult::Make_OKResult(move(info));
}