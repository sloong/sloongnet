/* File Name: server.c */
#include "control_service.h"
#include "NetworkHub.h"
#include "ControlHub.h"
#include "IData.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "DataTransPackage.h"
#include "configuation.h"
#include "SQLiteEx.h"
#include "config.pb.h"
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

		//m_pLog->Initialize(m_pConfig->m_oControlConfig.logpath(), "", m_pConfig->m_oControlConfig.debugmode(), LOGLEVEL(m_pConfig->m_oControlConfig.loglevel()), LOGTYPE::DAY);
		m_pLog->Initialize(m_pConfig->m_oControlConfig.logpath(), "", true, LOGLEVEL::All, LOGTYPE::DAY);

		m_pControl->Initialize(m_pConfig->m_oControlConfig.mqthreadquantity());
		m_pControl->Add(DATA_ITEM::GlobalConfiguation, &m_pConfig->m_oControlConfig);
		m_pControl->Add(Logger, m_pLog.get());

		m_pControl->RegisterEvent(ProgramExit);
		m_pControl->RegisterEvent(ProgramStart);
		try
		{
			IData::Initialize(m_pControl.get());
			m_pNetwork->Initialize(m_pControl.get());
			m_pNetwork->SetProperty(DataTransPackageProperty::DisableAll);
			m_pNetwork->RegisterMessageProcesser(std::bind(&SloongNetService::MessagePackageProcesser, this, std::placeholders::_1));
		}
		catch (exception e)
		{
			m_pLog->Error(string("Excepiton happened in initialize for ControlCenter. Message:") + string(e.what()));
			return false;
		}

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

void Sloong::SloongNetService::MessagePackageProcesser(SmartPackage pack)
{
	ProtobufMessage::MessagePackage msgPack;
	msgPack.ParseFromString(pack->GetRecvMessage());
	string config;
	if( msgPack.function() == MessageFunction::GetConfig)
	{
		switch(msgPack.sender())
		{
			case ModuleType::Proxy:
				m_pConfig->m_oProxyConfig.SerializeToString(&config);
				break;
			case ModuleType::Process:
				m_pConfig->m_oProcessConfig.SerializeToString(&config);
				break;
			case ModuleType::Firewall:
				m_pConfig->m_oFirewallConfig.SerializeToString(&config);
				break;
			case ModuleType::DataCenter:
				m_pConfig->m_oDataConfig.SerializeToString(&config);
				break;
			case ModuleType::DBCenter:
				m_pConfig->m_oDBConfig.SerializeToString(&config);
				break;
		}
	}

	pack->ResponsePackage(config);

	auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(pack->GetSocketID());
	response_event->SetDataPackage(pack);
	m_pControl->CallMessage(response_event);
}

void Sloong::SloongNetService::Exit()
{
	m_pLog->Info("Application will exit.");
	m_pControl->SendMessage(EVENT_TYPE::ProgramExit);
	m_pControl->Exit();
	m_oSync.notify_one();
}
