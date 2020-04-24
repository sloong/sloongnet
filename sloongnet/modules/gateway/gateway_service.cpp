/* File Name: server.c */
#include "gateway_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "IData.h"
#include "NormalEvent.hpp"
using namespace Sloong;
using namespace Sloong::Events;


unique_ptr<SloongNetGateway> Sloong::SloongNetGateway::Instance = nullptr;

extern "C" CResult MessagePackageProcesser(CDataTransPackage* pack)
{
	return SloongNetGateway::Instance->MessagePackageProcesser(pack);
}
	
extern "C" CResult NewConnectAcceptProcesser(CSockInfo* info)
{
	SloongNetGateway::Instance->AcceptConnectProcesser(info);
	return CResult::Succeed();
}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongNetGateway::Instance = make_unique<SloongNetGateway>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(IControl* iC){
	return SloongNetGateway::Instance->Initialized(iC);
}


CResult SloongNetGateway::Initialized(IControl* iC)
{
	m_pControl = iC;
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	Json::Reader reader;
	if ( m_pConfig->moduleconfig().length() > 0 && reader.parse(m_pConfig->moduleconfig(), m_oExConfig))
	{
		shared_ptr<CNormalEvent> event = make_shared<CNormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableTimeoutCheck);
		event->SetMessage(CUniversal::Format("{\"TimeoutTime\":\"%d\", \"CheckInterval\":%d}",m_oExConfig["TimeoutTime"].asInt(),m_oExConfig["TimeoutCheckInterval"].asInt()));
		m_pControl->SendMessage(event);

		event->SetEvent(EVENT_TYPE::EnableClientCheck);
		event->SetMessage(CUniversal::Format("{\"ClientCheckKey\":\"%s\", \"ClientCheckTime\":%d}",m_oExConfig["ClientCheckKey"].asString(),m_oExConfig["ClientCheckKey"].asInt()));
		m_pControl->SendMessage(event);
	}
	m_pLog = IData::GetLog();
	
	m_pControl->RegisterEventHandler(ProgramStart,std::bind(&SloongNetGateway::OnStart, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetGateway::OnSocketClose, this, std::placeholders::_1));
	return CResult::Succeed();
}


CResult Sloong::SloongNetGateway::MessagePackageProcesser(CDataTransPackage* pack)
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

bool SloongNetGateway::ConnectToProcess()
{
	// TODO: should be send a query requst to manager.
	/*auto list = CUniversal::split(m_oConfig.processaddress(),';');
	for( auto item = list.begin();item!= list.end(); item++ )
	{
		auto connect = make_shared<EasyConnect>();

		connect->Initialize(*item,nullptr); 
		connect->Connect();
		int sockID = connect->GetSocketID();
		m_mapProcessList[sockID] = connect;
		m_mapProcessLoadList[sockID] = 0;
		m_pNetwork->AddMonitorSocket(sockID );
	}*/
	return true;
}


void SloongNetGateway::OnStart(SmartEvent evt)
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
void Sloong::SloongNetGateway::AcceptConnectProcesser(CSockInfo* info){
	auto uuid = CUtility::GenUUID();
	// regist this uuid to control. if succeed, the control do nothing. but if it is registed, the control will send an error message. then retry regist again.
	m_mapUUIDList[info->m_pCon->GetSocketID()] = uuid;
}

void Sloong::SloongNetGateway::MessageToProcesser(CDataTransPackage* pack)
{
	// Step 1: 将已经收到的来自客户端的请求内容转换为protobuf格式
	auto sendMsg = make_shared<DataPackage>();
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
	transPack->Initialize(process_id->second, m_pLog);

	transPack->RequestPackage(sendMsg);

	// Step 5: 新建一个NetworkEx类型的事件，将上面准备完毕的数据发送出去。
	auto process_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	process_event->SetSocketID(process_id->second->GetSocketID());
	process_event->SetDataPackage(transPack);
	m_pControl->CallMessage(process_event);
	m_nSerialNumber++;
}

void Sloong::SloongNetGateway::MessageToClient(CDataTransPackage* pack)
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

void Sloong::SloongNetGateway::OnSocketClose(SmartEvent event)
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

