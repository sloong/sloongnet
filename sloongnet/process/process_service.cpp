/* File Name: server.c */
#include "process_service.h"
#include "NetworkHub.h"
#include "ControlHub.h"
#include "LuaProcessCenter.h"
#include "IData.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "DataTransPackage.h"
using namespace Sloong;
using namespace Sloong::Events;

IControl* Sloong::IData::m_iC = nullptr;

void sloong_terminator() 
{
	cout << "Unkonw error happened, system will shutdown. " << endl;
	CUtility::write_call_stack();
	exit(0);
}

void on_sigint(int signal)
{
	cout << "Unhandle signal happened, system will shutdown. signal:" << signal<< endl;
	CUtility::write_call_stack();
	exit(0);
}

// 成功加载后即创建UServer对象，并开始运行。
SloongNetProcess g_AppService;

void on_SIGINT_Event(int signal)
{
	g_AppService.Exit();
}


int main( int argc, char** args )
{
	if(g_AppService.Initialize(argc,args))
		g_AppService.Run();
}


SloongNetProcess::SloongNetProcess()
{
	m_pLog = make_unique<CLog>();
	m_pNetwork = make_unique<CNetworkHub>();
	m_pControl = make_unique<CControlHub>();
	m_pProcess = make_unique<CLuaProcessCenter>();
}

SloongNetProcess::~SloongNetProcess()
{
	Exit();
	CThreadPool::Exit();
    m_pLog->End();
}

void PrientHelp()
{
	cout << "param: address:port" << endl;
}

bool SloongNetProcess::Initialize(int argc, char** args)
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
		pack.set_sender(ModuleType::Process);
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
			m_pProcess->Initialize(m_pControl.get());
		}
		catch (exception e)
		{
			m_pLog->Error(string("Excepiton happened in initialize for ControlCenter. Message:") + string(e.what()));
			return false;
		}

		m_pControl->RegisterEventHandler(ReveivePackage, std::bind(&SloongNetProcess::OnReceivePackage, this, std::placeholders::_1));
		m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetProcess::OnSocketClose, this, std::placeholders::_1));

		return true;
	}
	catch (exception& e)
	{
		cout << "exception happened, system will shutdown. message:" << e.what() << endl;
	}
	catch(normal_except& e)
    {
        cout << "exception happened, system will shutdown. message:" << e.what() << endl;
    }
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. "<< endl;
		CUtility::write_call_stack();
	}
	
	return false;
}

bool SloongNetProcess::ConnectToControl(string controlAddress)
{
	
	m_pSocket = make_shared<lConnect>();
	m_pSocket->Initialize(controlAddress,nullptr);
	m_pSocket->Connect();
	
	/*string clientCheckKey = "c2xvb25nYzJ4dmIyNW5PRFJtT0dWa01ERTBNalZsTkRBd01XUmlZV1UxT0RZM05tRmlaamd3TmpsbmJtOXZiSE1nbm9vbHM";
	m_pSocket->Send(clientCheckKey);*/
}

void SloongNetProcess::Run()
{
	m_pLog->Info("Application begin running.");
	m_pControl->SendMessage(EVENT_TYPE::ProgramStart);
	m_oSync.wait();
}


void Sloong::SloongNetProcess::OnReceivePackage(SmartEvent evt)
{	
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(evt);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	SmartPackage pack = net_evt->GetDataPackage();

	net_evt->SetEvent(EVENT_TYPE::SendMessage);
	
	string strRes("");
	char* pExData = nullptr;
	int nExSize;

	ProtobufMessage::MessagePackage msg;
	msg.ParseFromString(pack->GetRecvMessage());
	msg.set_sender(ModuleType::Process);
	msg.set_receiver(ModuleType::Proxy);
	msg.set_function(MessageFunction::ResponseRequest);
	if (m_pProcess->MsgProcess(info, msg.context() , strRes, pExData, nExSize)){
		msg.set_context(strRes);
	}else{
		m_pLog->Error("Error in process");
		msg.set_context("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
	}
	string res;
	msg.SerializeToString(&res);
	pack->ResponsePackage(res,pExData,nExSize);
	net_evt->SetDataPackage(pack);
	m_pControl->SendMessage(net_evt);
}

void Sloong::SloongNetProcess::OnSocketClose(SmartEvent event)
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

void Sloong::SloongNetProcess::Exit()
{
	m_pLog->Info("Application will exit.");
	m_pControl->SendMessage(EVENT_TYPE::ProgramExit);
	m_pControl->Exit();
	m_oSync.notify_one();
}
