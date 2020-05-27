/*
 * @Author: WCB
 * @Date: 2020-04-28 14:43:16
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 20:39:20
 * @Description: file content
 */
#include "NetworkEvent.hpp"
#include "transpond.h"
#include "IData.h"
#include "gateway_service.h"

#include "snowflake.h"

using namespace Sloong::Events;

CResult Sloong::GatewayTranspond::Initialize(IControl* ic)
{
    m_pLog = IData::GetLog();
    m_pControl = ic;
    return CResult::Succeed();
}

CResult GatewayTranspond::RequestPackageProcesser(CDataTransPackage *trans_pack)
{
	m_pLog->Debug("Receive new request package.");
	auto res = MessageToProcesser(trans_pack);
	m_pLog->Debug(Helper::Format("Response [%s][%s].", ResultType_Name(res.Result()).c_str(), res.Message().c_str()));
	return res;
}

CResult GatewayTranspond::ResponsePackageProcesser( RequestInfo* info, CDataTransPackage *trans_pack)
{
	return MessageToClient(info,trans_pack);
}


CResult Sloong::GatewayTranspond::MessageToProcesser(CDataTransPackage *pack)
{
	auto data_pack = pack->GetDataPackage();
	auto target = SloongNetGateway::Instance->GetPorcessConnect(data_pack->function());
	if( target == INVALID_SOCKET )
	{
		return CResult::Make_Error("No process service online .");
	}

	RequestInfo info;
	info.RequestConnect = pack->GetConnection();
	info.SerialNumber = data_pack->id();
	info.tStart = pack->GetRecord()->front();	
	info.tProcess = pack->GetTimeval();
		
	auto serialNumber = snowflake::Instance->nextid();
	data_pack->set_id(serialNumber);
	pack->ClearConnection();
	pack->SetSocket(target);
	pack->RequestPackage();

	m_pLog->Debug(Helper::Format("Trans package [%d][%llu] -> [%d][%llu]", info.RequestConnect->GetSocketID(), info.SerialNumber, target, serialNumber));

	SloongNetGateway::Instance->m_mapSerialToRequest[serialNumber] = info;
	return CResult::Succeed();
}

CResult Sloong::GatewayTranspond::MessageToClient(RequestInfo *req_info, CDataTransPackage *res_pack)
{
	auto res_data = res_pack->GetDataPackage();
	res_data->set_id(req_info->SerialNumber);
	res_pack->SetConnection(req_info->RequestConnect);
	res_pack->ResponsePackage(ResultType::Succeed);

	auto t = res_pack->GetRecord();
	t->push_front(req_info->tProcess);
	t->push_front(req_info->tStart);

	return CResult::Succeed();
}