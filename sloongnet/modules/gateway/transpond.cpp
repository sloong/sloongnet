/*
 * @Author: WCB
 * @Date: 2020-04-28 14:43:16
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-13 16:29:41
 * @Description: file content
 */
#include "NetworkEvent.hpp"
#include "transpond.h"
#include "IData.h"

#include "protocol/processer.pb.h"

using namespace Sloong::Events;

CResult Sloong::GatewayTranspond::Initialize(IControl* ic)
{
    m_pLog = IData::GetLog();
    m_pControl = ic;
    return CResult::Succeed();
}

CResult Sloong::GatewayTranspond::PackageProcesser(CDataTransPackage* pack)
{
	auto event_happend_socket = pack->GetSocketID();
	if( m_mapProcessLoadList.find(event_happend_socket) == m_mapProcessLoadList.end() )
	{
		MessageToProcesser(pack);
	}
	else
	{
		MessageToClient(pack);
	}
	return CResult::Succeed();
}

// Case 1:
// 		In this function, build the uuid by the socket id. and regist the uuid to control. 
// 		then send all message to process server must have this uuid.
// 		the process server processed by this value.
// Case 2:
//		In this function, just build the uuid, and use it in next request, but no regist to control, because ne login no should have userinfo.
//		so process server always create new empty UserInfo to run process function. if in this request, the user is logined, so the empty UserInfo isChanged. so we regist it in this time. 
void Sloong::GatewayTranspond::AcceptConnectProcesser(CSockInfo* info){
	auto uuid = CUtility::GenUUID();
	// regist this uuid to control. if succeed, the control do nothing. but if it is registed, the control will send an error message. then retry regist again.
	m_mapUUIDList[info->m_pCon->GetSocketID()] = uuid;
}


void Sloong::GatewayTranspond::MessageToProcesser(CDataTransPackage* pack)
{
	// Step 1: 将已经收到的来自客户端的请求内容转换为protobuf格式
	auto sendMsg = make_shared<DataPackage>();
	sendMsg->set_function(Processer::Functions::ProcessMessage);
	sendMsg->set_content(pack->GetRecvMessage());
	sendMsg->set_prioritylevel(pack->GetPriority());
	sendMsg->set_serialnumber(m_nSerialNumber);
	sendMsg->set_extend(m_mapUUIDList[pack->GetSocketID()].data());

	// Step 2: 根据负载情况找到发送的目标Process服务
	auto process_id = m_mapProcessList.begin();

	// Step 3: 根据流水号将来自客户端的Event对象保存起来
	m_mapPackageList[m_nSerialNumber] = pack;

	// Step 4: 创建发送到指定process服务的DataTrans包
	auto transPack = make_shared<CDataTransPackage>();
	transPack->Initialize(process_id->second, m_pLog);

	transPack->RequestPackage(sendMsg);

	// Step 5: 新建一个NetworkEx类型的事件，将上面准备完毕的数据发送出去。
	auto process_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	process_event->SetSocketID(process_id->second->GetSocketID());
	process_event->SetDataPackage(transPack);
	m_pControl->CallMessage(process_event);
	m_nSerialNumber++;
}

void Sloong::GatewayTranspond::MessageToClient(CDataTransPackage* pack)
{
	// Step 1: 将Process服务处理完毕的消息转换为正常格式
	auto recvMsg = pack->GetRecvPackage();

	// Step 2: 根据收到的SerailNumber找到对应保存的来自客户端的Event对象
	auto event_obj = m_mapPackageList.find(recvMsg->serialnumber());
	if (event_obj == m_mapPackageList.end()) {
		m_pLog->Error(CUniversal::Format("Event list no have target event data. SerailNumber:[%d]", pack->GetSerialNumber()));
		return;
	}
	auto client_request_package = event_obj->second;
	client_request_package->ResponsePackage( ResultType::Succeed, recvMsg->content(), &recvMsg->extend());

	// Step 3: 发送相应数据
	/*auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(client_request_package->GetSocketID());
	response_event->SetDataPackage(client_request_package);
	m_pControl->CallMessage(response_event);*/
	
}