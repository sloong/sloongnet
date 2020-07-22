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

CResult GatewayTranspond::RequestPackageProcesser(CDataTransPackage *trans_pack)
{
	return MessageToProcesser(trans_pack);
}

CResult GatewayTranspond::ResponsePackageProcesser( RequestInfo* info, CDataTransPackage *trans_pack)
{
	return MessageToClient(info,trans_pack);
}


CResult Sloong::GatewayTranspond::MessageToProcesser(CDataTransPackage *pack)
{
	m_pLog->Debug("Receive new request package.");
	auto data_pack = pack->GetDataPackage();
	auto target = SloongNetGateway::Instance->GetPorcessConnect(data_pack->function());
	if( target == INVALID_SOCKET )
	{
		pack->ResponsePackage(ResultType::Error,"No process service online .");
		m_pLog->Debug(Helper::Format("No find process service for function[%d]. package [%d][%llu]", data_pack->function(),  pack->GetSocketID(), pack->GetSerialNumber() ));
		return CResult::Succeed();
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
	res_pack->ResponsePackage(res_data);

	auto t = res_pack->GetRecord();
	t->push_front(req_info->tProcess);
	t->push_front(req_info->tStart);

	return CResult::Succeed();
}