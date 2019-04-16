/* File Name: server.c */
#include "proxy_service.h"
#include "NetworkHub.h"
#include "ControlHub.h"
#include "IData.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "DataTransPackage.h"
using namespace Sloong;
using namespace Sloong::Events;

IControl *Sloong::IData::m_iC = nullptr;

void sloong_terminator()
{
	cout << "Unkonw error happened, system will shutdown. " << endl;
	CUtility::write_call_stack();
	exit(0);
}

void on_sigint(int signal)
{
	cout << "Unhandle signal happened, system will shutdown. signal:" << signal << endl;
	CUtility::write_call_stack();
	exit(0);
}

// 成功加载后即创建UServer对象，并开始运行。
SloongNetProxy g_AppService;

void on_SIGINT_Event(int signal)
{
	g_AppService.Exit();
}

int main(int argc, char **args)
{
	if (g_AppService.Initialize(argc, args))
		g_AppService.Run();
}

SloongNetProxy::SloongNetProxy()
{
	m_pLog = make_unique<CLog>();
	m_pNetwork = make_unique<CNetworkHub>();
	m_pControl = make_unique<CControlHub>();
}

SloongNetProxy::~SloongNetProxy()
{
	Exit();
	CThreadPool::Exit();
	m_pLog->End();
}

void PrientHelp()
{
	cout << "param: address:port" << endl;
}

bool SloongNetProxy::Initialize(int argc, char **args)
{
	set_terminate(sloong_terminator);
	set_unexpected(sloong_terminator);

	//SIG_IGN:忽略信号的处理程序
	//SIGPIPE:在reader终止之后写pipe的时候发生
	signal(SIGPIPE, SIG_IGN); // this signal should call the socket check function. and remove the timeout socket.
	//SIGCHLD: 进程Terminate或Stop的时候,SIGPIPE会发送给进程的父进程,缺省情况下该Signal会被忽略
	signal(SIGCHLD, SIG_IGN);
	//SIGINT:由Interrupt Key产生,通常是Ctrl+c或者Delete,发送给所有的ForeGroundGroup进程.
	signal(SIGINT, &on_SIGINT_Event);
	// SIGSEGV:当一个进程执行了一个无效的内存引用，或发生段错误时发送给它的信号
	signal(SIGSEGV, &on_sigint);

	try
	{
		if (argc != 2)
		{
			PrientHelp();
			return false;
		}

		if( !ConnectToControl(args[1]))
		{
			cout << "Connect to control fialed." <<endl;
			return false;
		}

		ProtobufMessage::MessagePackage pack;
		pack.set_function(MessageFunction::GetConfig);
		pack.set_sender(ModuleType::Proxy);
		pack.set_receiver(ModuleType::ControlCenter);
		
		string strMsg;
		pack.SerializeToString(&strMsg);

		CDataTransPackage dataPackage;
		dataPackage.Initialize(m_pSocket);
		dataPackage.SetProperty(DataTransPackageProperty::DisableAll);
		dataPackage.RequestPackage(strMsg);
		NetworkResult result = dataPackage.SendPackage();
		if(result != NetworkResult::Succeed)
		{
			cerr << "Send get config request error."<< endl;
			return false;
		}
		result = dataPackage.RecvPackage(0);
		if(result != NetworkResult::Succeed)
		{
			cerr << "Receive get config result error."<< endl;
			return false;
		}

		m_oConfig.ParseFromString(dataPackage.GetRecvMessage());
		
		auto serv_config = m_oConfig.serverconfig();

		//m_pLog->Initialize(serv_config.logpath(), "", serv_config.debugmode(), LOGLEVEL(serv_config.loglevel()), LOGTYPE::DAY);
		m_pLog->Initialize(serv_config.logpath(), "", true, LOGLEVEL::All, LOGTYPE::DAY);

		m_pControl->Initialize(serv_config.mqthreadquantity());
		m_pControl->Add(DATA_ITEM::GlobalConfiguation, m_oConfig.mutable_serverconfig());
		m_pControl->Add(DATA_ITEM::ModuleConfiguation, &m_oConfig);
		m_pControl->Add(Logger, m_pLog.get());

		m_pControl->RegisterEvent(ProgramExit);
		m_pControl->RegisterEvent(ProgramStart);
		
		try
		{
			IData::Initialize(m_pControl.get());
			m_pNetwork->Initialize(m_pControl.get());
			m_pNetwork->EnableClientCheck(m_oConfig.clientcheckkey(),m_oConfig.clientchecktime());
			m_pNetwork->EnableTimeoutCheck(m_oConfig.timeouttime(), m_oConfig.timeoutcheckinterval());
			m_pNetwork->RegisterMessageProcesser(std::bind(&SloongNetProxy::MessagePackageProcesser, this, std::placeholders::_1));
		}
		catch (exception e)
		{
			m_pLog->Error(string("Excepiton happened in initialize for ControlCenter. Message:") + string(e.what()));
			return false;
		}

		m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetProxy::OnSocketClose, this, std::placeholders::_1));

		return true;
	}
	catch (exception &e)
	{
		cout << "exception happened, system will shutdown. message:" << e.what() << endl;
	}
	catch (normal_except &e)
	{
		cout << "exception happened, system will shutdown. message:" << e.what() << endl;
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. " << endl;
		CUtility::write_call_stack();
	}

	return false;
}

bool SloongNetProxy::ConnectToProcess()
{
	auto list = CUniversal::split(m_oConfig.processaddress(),";");
	for( auto item = list.begin();item!= list.end(); item++ )
	{
		auto connect = make_shared<EasyConnect>();

		connect->Initialize(*item,nullptr); 
		connect->Connect();
		int sockID = connect->GetSocketID();
		m_mapProcessList[sockID] = connect;
		m_mapProcessLoadList[sockID] = 0;
		m_pNetwork->AddMonitorSocket(sockID, DataTransPackageProperty::DisableAll );
	}
}

bool SloongNetProxy::ConnectToControl(string controlAddress)
{
	
	m_pSocket = make_shared<EasyConnect>();
	m_pSocket->Initialize(controlAddress,nullptr);
	m_pSocket->Connect();
	/*string clientCheckKey = "c2xvb25nYzJ4dmIyNW5PRFJtT0dWa01ERTBNalZsTkRBd01XUmlZV1UxT0RZM05tRmlaamd3TmpsbmJtOXZiSE1nbm9vbHM";
	m_pSocket->Send(clientCheckKey);*/
}

void SloongNetProxy::Run()
{
	m_pLog->Info("Application begin running.");
	m_pControl->SendMessage(EVENT_TYPE::ProgramStart);
	ConnectToProcess();
	m_oSync.wait();
}


void Sloong::SloongNetProxy::MessagePackageProcesser(SmartPackage pack)
{
	auto event_happend_socket = pack->GetSocketID();
	if( m_mapProcessLoadList.find(event_happend_socket) == m_mapProcessLoadList.end() )
	{
		// Step 1: 将已经收到的来自客户端的请求内容转换为protobuf格式
		ProtobufMessage::MessagePackage msg;
		msg.set_sender(ModuleType::Proxy);
		msg.set_receiver(ModuleType::Process);
		msg.set_function(MessageFunction::SendRequest);
		msg.set_context(pack->GetRecvMessage());
		msg.set_prioritylevel(pack->GetPriority());
		msg.set_serialnumber(m_nSerialNumber);
		string sendMsg;
		msg.SerializeToString(&sendMsg);

		// Step 2: 根据负载情况找到发送的目标Process服务
		auto process_id = m_mapProcessList.begin();

		// Step 3: 根据流水号将来自客户端的Event对象保存起来
		m_mapPackageList[m_nSerialNumber] = pack;

		// Step 4: 创建发送到指定process服务的DataTrans包
		auto transPack = make_shared<CDataTransPackage>();
		transPack->Initialize(process_id->second,m_pLog.get());
		transPack->SetProperty(DataTransPackageProperty::DisableAll);
		
		transPack->RequestPackage(sendMsg);

		// Step 5: 新建一个NetworkEx类型的事件，将上面准备完毕的数据发送出去。
		auto process_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
		process_event->SetSocketID(process_id->second->GetSocketID());
		process_event->SetDataPackage(transPack);
		m_pControl->CallMessage(process_event);
		m_nSerialNumber++;
	}
	else
	{
		// Step 1: 将Process服务处理完毕的消息转换为正常格式
		ProtobufMessage::MessagePackage msg;
		msg.ParseFromString(pack->GetRecvMessage());

		// Step 2: 根据收到的SerailNumber找到对应保存的来自客户端的Event对象
		auto event_obj = m_mapPackageList.find(msg.serialnumber());
		if( event_obj == m_mapPackageList.end() )
		{
			m_pLog->Error(CUniversal::Format("Event list no have target event data. SerailNumber:[%d]",pack->GetSerialNumber()));
			return;
		}
		auto client_request_package = event_obj->second;
		client_request_package->ResponsePackage(msg.context());

		// Step 3: 发送相应数据
		auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
		response_event->SetSocketID(client_request_package->GetSocketID());
		response_event->SetDataPackage(client_request_package);
		m_pControl->CallMessage(response_event);
	}
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

void Sloong::SloongNetProxy::Exit()
{
	m_pLog->Info("Application will exit.");
	m_pControl->SendMessage(EVENT_TYPE::ProgramExit);
	m_pControl->Exit();
	m_oSync.notify_one();
}
