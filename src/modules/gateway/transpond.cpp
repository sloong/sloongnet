/*
 * @Author: WCB
 * @Date: 2020-04-28 14:43:16
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 20:39:20
 * @Description: file content
 */
#include "transpond.h"
#include "IData.h"
#include "gateway.h"
#include "snowflake.h"

CResult Sloong::GatewayTranspond::Initialize(IControl* ic)
{
    m_pLog = IData::GetLog();
    m_iC = ic;
    return CResult::Succeed();
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
	auto target = SloongNetGateway::Instance->GetPorcessConnect(pack->function());
	if( target == INVALID_SOCKET )
	{
		m_pLog->Debug(Helper::Format("No find process service for function[%d]. package [%d][%llu]", pack->function(),  pack->sessionid(), pack->id() ));
		return PackageResult::Make_Error(Helper::Format("No find process service for function[%d]. package [%d][%llu]", pack->function(),  pack->sessionid(), pack->id() ));
	}

	auto response = Package::MakeResponse(pack);
	for( auto i : pack->clocks() )
	{
		response->add_clocks(i);
	}
	response->add_clocks(GetClock());

	auto trans_pack = Package::MakeResponse(pack);
	
	auto id = snowflake::Instance->nextid();
	trans_pack->set_id(id);
	trans_pack->set_sessionid(target);
	
	m_pLog->Debug(Helper::Format("Trans package [%lld][%llu] -> [%d][%llu]", pack->sessionid(), pack->id(), trans_pack->sessionid(), trans_pack->id()));

	SloongNetGateway::Instance->m_mapSerialToRequest[trans_pack->sessionid()] = move(response);
	return PackageResult::Make_OK(move(trans_pack));
}

PackageResult Sloong::GatewayTranspond::MessageToClient(UniquePackage info, DataPackage *pack)
{
	info->add_clocks(GetClock());
	info->set_result(pack->result());
	info->set_content(pack->content());
	info->set_extend(pack->extend());
	
	return PackageResult::Make_OK(move(info));
}