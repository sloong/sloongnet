/* File Name: server.c */
#include "proxy_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"

using namespace Sloong;
using namespace Sloong::Events;

void SloongNetProxy::AfterInit()
{
	m_oConfig.ParseFromString(m_pServerConfig->exconfig());
	m_pControl->Add(DATA_ITEM::ServerConfiguation, &m_oConfig);
	m_pNetwork->EnableClientCheck(m_oConfig.clientcheckkey(),m_oConfig.clientchecktime());
	m_pNetwork->EnableTimeoutCheck(m_oConfig.timeouttime(), m_oConfig.timeoutcheckinterval());
	
	RegistFunctionHandler(Functions::ProcessMessage,std::bind(&SloongNetProxy::ProcessMessageHanlder, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_pNetwork->RegisterAccpetConnectProcesser(std::bind(&SloongNetProxy::AcceptConnectProcesser, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(ProgramStart,std::bind(&SloongNetProxy::OnStart, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetProxy::OnSocketClose, this, std::placeholders::_1));
}

bool SloongNetProxy::ConnectToProcess()
{
	auto list = CUniversal::split(m_oConfig.processaddress(),';');
	for( auto item = list.begin();item!= list.end(); item++ )
	{
		auto connect = make_shared<EasyConnect>();

		connect->Initialize(*item,nullptr); 
		connect->Connect();
		int sockID = connect->GetSocketID();
		m_mapProcessList[sockID] = connect;
		m_mapProcessLoadList[sockID] = 0;
		m_pNetwork->AddMonitorSocket(sockID );
	}
	return true;
}


void SloongNetProxy::OnStart(SmartEvent evt)
{
	ConnectToProcess();
}



// Case 1:
// 		In this function, build the uuid by the socket id. and regist the uuid to control. 
// 		then send all message to process server must have this uuid.
// 		the process server processed by this value.
// Case 2:
//		In this function, just build the uuid, and use it in next request, but no regist to control, because ne login no should have userinfo.
//		so process server always create new empty UserInfo to run process function. if in this request, the user is logined, so the empty UserInfo isChanged. so we regist it in this time. 
void Sloong::SloongNetProxy::AcceptConnectProcesser(shared_ptr<CSockInfo> info){
	auto uuid = CUtility::GenUUID();
	// regist this uuid to control. if succeed, the control do nothing. but if it is registed, the control will send an error message. then retry regist again.
	m_mapUUIDList[info->m_pCon->GetSocketID()] = uuid;
}



bool Sloong::SloongNetProxy::ProcessMessageHanlder(Functions func, string sender, SmartPackage pack)
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
	return true;
}

void Sloong::SloongNetProxy::MessageToProcesser(SmartPackage pack)
{
	// Step 1: 将已经收到的来自客户端的请求内容转换为protobuf格式
	auto sendMsg = make_shared<DataPackage>();
	sendMsg->set_receiver(ModuleType::Process);
	sendMsg->set_function(Functions::ProcessMessage);
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
	transPack->Initialize(process_id->second, m_pLog.get());

	transPack->RequestPackage(sendMsg);

	// Step 5: 新建一个NetworkEx类型的事件，将上面准备完毕的数据发送出去。
	auto process_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	process_event->SetSocketID(process_id->second->GetSocketID());
	process_event->SetDataPackage(transPack);
	m_pControl->CallMessage(process_event);
	m_nSerialNumber++;
}

void Sloong::SloongNetProxy::MessageToClient(SmartPackage pack)
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
	client_request_package->ResponsePackage(recvMsg->content(), recvMsg->extend());

	// Step 3: 发送相应数据
	auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(client_request_package->GetSocketID());
	response_event->SetDataPackage(client_request_package);
	m_pControl->CallMessage(response_event);
	
}

void Sloong::SloongNetProxy::OnSocketClose(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	// call close function.
	//m_pProcess->CloseSocket(info);
	//net_evt->CallCallbackFunc(net_evt);
}

