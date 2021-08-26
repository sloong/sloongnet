/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-04-28 14:43:16
 * @LastEditTime: 2021-08-20 14:14:33
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
		auto msg = format("No find process service for function[{}]. package [{}][{}]", pack->function(), pack->sessionid(), pack->id() );
		m_pLog->Debug(msg);
		return PackageResult::Make_Error(msg);
	}

	auto response = Package::MakeResponse(pack);
	for( auto i : pack->clocks() )
	{
		response->add_clocks(i);
	}
	response->add_clocks(GetClock());

	auto trans_pack = Package::MakeResponse(pack);
	trans_pack->set_status(DataPackage_StatusType::DataPackage_StatusType_Request);
	trans_pack->set_content( pack->content() );
	trans_pack->set_extend( pack->extend() );

	auto id = snowflake::Instance->nextid();
	trans_pack->set_id(id);
	trans_pack->set_sessionid(target);
	
	m_pLog->Debug(format("Trans package [{}][{}] -> [{}][{}]", pack->sessionid(), pack->id(), trans_pack->sessionid(), trans_pack->id()));

	SloongNetGateway::Instance->m_mapSerialToRequest[trans_pack->id()] = move(response);
	return PackageResult::Make_OKResult(move(trans_pack));
}

PackageResult Sloong::GatewayTranspond::MessageToClient(UniquePackage info, Package *pack)
{
	info->add_clocks(GetClock());
	info->set_result(pack->result());
	info->set_content( pack->content() );
	info->set_extend( pack->extend() );
	 
	return PackageResult::Make_OKResult(move(info));
}