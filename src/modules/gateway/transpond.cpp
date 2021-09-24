/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-04-28 14:43:16
 * @LastEditTime: 2021-09-23 17:08:53
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

CResult Sloong::GatewayTranspond::Initialize(IControl *ic)
{
	IObject::Initialize(ic);
	return CResult::Succeed;
}

PackageResult GatewayTranspond::RequestPackageProcesser(Package *trans_pack)
{
	return MessageToProcesser(trans_pack);
}

PackageResult GatewayTranspond::ResponsePackageProcesser(UniquePackage info, Package *trans_pack)
{
	return MessageToClient(move(info), trans_pack);
}

PackageResult Sloong::GatewayTranspond::MessageToProcesser(Package *pack)
{
	m_pLog->debug("Receive new request package.");
	auto target = SloongNetGateway::Instance->GetForwardInfo(pack->function());
	if (target.IsSucceed())
	{
		auto info = target.GetResultObject();
		auto response = Package::MakeResponse(pack);
		response->record_point("ForwardToProcesser");

		// Use the make request to copy need send data. and reset type to request.
		auto trans_pack = Package::MakeResponse(pack);
		trans_pack->set_type(DataPackage_PackageType::DataPackage_PackageType_Request);
		trans_pack->set_content(pack->content());
		trans_pack->set_extend(pack->extend());
		trans_pack->set_function(info.function_id);

		auto id = snowflake::Instance->nextid();
		trans_pack->set_id(id);
		trans_pack->set_sessionid(info.connection_id);

		m_pLog->debug(format("Trans package [{}][{}] -> [{}][{}]", pack->sessionid(), pack->id(), trans_pack->sessionid(), trans_pack->id()));

		SloongNetGateway::Instance->m_mapSerialToRequest[trans_pack->id()] = move(response);
		return PackageResult::Make_OKResult(move(trans_pack));
	}
	else
	{
		m_pLog->error(target.GetMessage());
		return PackageResult::Make_Error(target.GetMessage());
	}
}

PackageResult Sloong::GatewayTranspond::MessageToClient(UniquePackage info, Package *pack)
{
	info->record_point("ResponseToClient");
	info->set_result(pack->result());
	info->set_content(pack->content());
	info->set_extend(pack->extend());

	return PackageResult::Make_OKResult(move(info));
}