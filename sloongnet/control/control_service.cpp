/* File Name: server.c */
#include "control_service.h"
#include "NetworkHub.h"
#include "ControlHub.h"
#include "IData.h"
#include "utility.h"
#include "NetworkEvent.h"
#include "DataTransPackage.h"
#include "configuation.h"
#include "SQLiteEx.h"

#include "version.h"
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
SloongNetService g_AppService;

void on_SIGINT_Event(int signal)
{
	g_AppService.Exit();
}

int main(int argc, char **args)
{
	if (g_AppService.Initialize(argc, args))
		g_AppService.Run();

	return 0;
}

SloongNetService::SloongNetService()
{
	m_pLog = make_unique<CLog>();
	m_pNetwork = make_unique<CNetworkHub>();
	m_pControl = make_unique<CControlHub>();
	m_pConfig = make_unique<CConfiguation>();
}

SloongNetService::~SloongNetService()
{
	Exit();
	CThreadPool::Exit();
	m_pLog->End();
}

void PrintVersion()
{
	cout << PRODUCT_TEXT << endl;
	cout << VERSION_TEXT << endl;
	cout << COPYRIGHT_TEXT << endl;
}

void PrientHelp()
{
	cout << "param: listen port" << endl;
}

bool SloongNetService::Initialize(int argc, char **args)
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
		int port = atoi(args[1]);
		if (port == 0)
		{
			cout << "Convert [" << args[1] << "] to int port fialed." << endl;
			return false;
		}

		m_pConfig->Initialize("system");
		m_pConfig->LoadAll();

		m_pLog->Initialize(m_pConfig->m_oControlConfig.logpath(), "", m_pConfig->m_oControlConfig.debugmode(), LOGLEVEL(m_pConfig->m_oControlConfig.loglevel()), LOGTYPE::DAY);
		// if (config.m_oLogInfo.NetworkPort != 0)
		// 	m_pLog->EnableNetworkLog(config.m_oLogInfo.NetworkPort);

		m_pControl->Initialize(m_pConfig->m_oControlConfig.mqthreadquantity());
		m_pControl->Add(Configuation, &m_pConfig->m_oControlConfig);
		m_pControl->Add(Logger, m_pLog.get());

		m_pControl->RegisterEvent(ProgramExit);
		m_pControl->RegisterEvent(ProgramStart);
		try
		{
			IData::Initialize(m_pControl.get());
			m_pNetwork->Initialize(m_pControl.get());
		}
		catch (exception e)
		{
			m_pLog->Error(string("Excepiton happened in initialize for ControlCenter. Message:") + string(e.what()));
			return false;
		}
		m_pControl->RegisterEventHandler(ReveivePackage, std::bind(&SloongNetService::OnReceivePackage, this, std::placeholders::_1));
		m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetService::OnSocketClose, this, std::placeholders::_1));

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

void SloongNetService::Run()
{
	m_pLog->Info("Application begin running.");
	m_pControl->SendMessage(EVENT_TYPE::ProgramStart);
	m_oSync.wait();
}

void Sloong::SloongNetService::OnReceivePackage(SmartEvent evt)
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
	// char* pExData = nullptr;
	// int nExSize;
	string strMsg = pack->GetRecvMessage();
	// if (m_pProcess->MsgProcess(info, strMsg , strRes, pExData, nExSize)){
	// 	pack->ResponsePackage(strRes,pExData,nExSize);
	// }else{
	// 	m_pLog->Error("Error in process");
	pack->ResponsePackage("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
	// }

	net_evt->SetDataPackage(pack);
	m_pControl->SendMessage(net_evt);
}

void Sloong::SloongNetService::OnSocketClose(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	// call close function.
	net_evt->CallCallbackFunc(net_evt);
}

void Sloong::SloongNetService::Exit()
{
	m_pLog->Info("Application will exit.");
	m_pControl->SendMessage(EVENT_TYPE::ProgramExit);
	m_pControl->Exit();
	m_oSync.notify_one();
}
