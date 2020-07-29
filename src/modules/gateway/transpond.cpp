/*** 
 * @Author: Chuanbin Wang
 * @Date: 2020-04-28 14:43:16
 * @LastEditTime: 2020-07-29 17:23:24
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/gateway/transpond.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#include "transpond.h"
#include "IData.h"
#include "gateway.h"
#include "snowflake.h"

CResult Sloong::GatewayTranspond::Initialize(IControl* ic)
{
    m_pLog = IData::GetLog();
    m_iC = ic;
    return CResult::Succeed;
}

PackageResult GatewayTranspond::RequestPackageProcesser(DataPackage *trans_pack)
{
	return MessageToProcesser(trans_pack);
}

PackageResult GatewayTranspond::ResponsePackageProcesser( UniquePackage info, DataPackage *trans_pack)
{
	return MessageToClient(move(info),trans_pack);
}


PackageResult Sloong::GatewayTranspond::MessageToProcesser(DataPackage *pack)
{
	m_pLog->Debug("Receive new request package.");
	auto target = SloongNetGateway::Instance->GetPorcessConnection(pack->function());
	if( target == 0 )
	{
		m_pLog->Debug(Helper::Format("No find process service for function[%d]. package [%d][%lld]", pack->function(),  pack->reserved().sessionid(), pack->id() ));
		return PackageResult::Make_Error(Helper::Format("No find process service for function[%d]. package [%d][%lld]", pack->function(),  pack->reserved().sessionid(), pack->id() ));
	}

	auto response = Package::MakeResponse(pack);
	for( auto i : pack->reserved().clocks() )
	{
		response->mutable_reserved()->add_clocks(i);
	}
	response->mutable_reserved()->add_clocks(GetClock());

	auto trans_pack = Package::MakeResponse(pack);
	trans_pack->set_status(DataPackage_StatusType::DataPackage_StatusType_Request);
	
	auto id = snowflake::Instance->nextid();
	trans_pack->set_id(id);
	trans_pack->mutable_reserved()->set_sessionid(target);
	
	m_pLog->Debug(Helper::Format("Trans package [%lld][%lld] -> [%d][%lld]", pack->reserved().sessionid(), pack->id(), trans_pack->reserved().sessionid(), trans_pack->id()));

	SloongNetGateway::Instance->m_mapSerialToRequest[trans_pack->id()] = move(response);
	return PackageResult::Make_OK(move(trans_pack));
}

PackageResult Sloong::GatewayTranspond::MessageToClient(UniquePackage info, DataPackage *pack)
{
	info->mutable_reserved()->add_clocks(GetClock());
	info->set_result(pack->result());
	info->set_content(pack->content());
	info->set_extend(pack->extend());
	
	return PackageResult::Make_OK(move(info));
}