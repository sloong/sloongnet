/* File Name: server.c */
#include "serverconfig.h"
#include "CmdProcess.h"
#include "proxy_service.h"
#include "NetworkHub.h"
#include "ControlHub.h"
#include "IData.h"
#include "utility.h"
#include "NetworkEvent.h"
#include "DataTransPackage.h"
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
SloongNetProxy g_AppService;

void on_SIGINT_Event(int signal)
{
	g_AppService.Exit();
}


int main( int argc, char** args )
{
	if(g_AppService.Initialize(argc,args))
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


bool SloongNetProxy::Initialize(int argc, char** args)
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
		
		// CmdProcess会根据参数来加载正确的配置信息。成功返回true。
		if (CCmdProcess::Parser(argc, args, &config))
		{
			LOGTYPE oType = LOGTYPE::ONEFILE;
			if (!config.m_oLogInfo.LogWriteToOneFile)
			{
				oType = LOGTYPE::DAY;
			}
			m_pLog->Initialize(config.m_oLogInfo.LogPath, "",config.m_oLogInfo.DebugMode, LOGLEVEL(config.m_oLogInfo.LogLevel), oType);
			if (config.m_oLogInfo.NetworkPort != 0)
				m_pLog->EnableNetworkLog(config.m_oLogInfo.NetworkPort);
				
			m_pControl->Initialize(config.m_nMessageCenterThreadQuantity);
			m_pControl->Add(Configuation, &config);
			m_pControl->Add(Logger, m_pLog.get());
			
			m_pControl->RegisterEvent(ProgramExit);
			m_pControl->RegisterEvent(ProgramStart);
			m_pControl->RegisterEventHandler(ReveivePackage, std::bind(&SloongNetProxy::OnReceivePackage, this, std::placeholders::_1));
			m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetProxy::OnSocketClose, this, std::placeholders::_1));

			try{
				IData::Initialize(m_pControl.get());
				m_pNetwork->Initialize(m_pControl.get());
			}
			catch(exception e){
				m_pLog->Error(string("Excepiton happened in initialize for ControlCenter. Message:")+  string(e.what()));
				return false;
			}
			return true;
		}
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

void SloongNetProxy::Run()
{
	m_pLog->Info("Application begin running.");
	m_pControl->SendMessage(MSG_TYPE::ProgramStart);
}


void Sloong::SloongNetProxy::OnReceivePackage(SmartEvent evt)
{	
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(evt);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	SmartPackage pack = net_evt->GetDataPackage();

	net_evt->SetEvent(MSG_TYPE::SendMessage);
	
	string strRes("");
	// char* pExData = nullptr;
	// int nExSize;
	// string strMsg = pack->GetRecvMessage();
	// if (m_pProcess->MsgProcess(info, strMsg , strRes, pExData, nExSize)){
	// 	pack->ResponsePackage(strRes,pExData,nExSize);
	// }else{
	// 	m_pLog->Error("Error in process");
		pack->ResponsePackage("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
	// }
	
	net_evt->SetDataPackage(pack);
	m_pControl->SendMessage(net_evt);
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
	net_evt->CallCallbackFunc(net_evt);
}

void Sloong::SloongNetProxy::Exit()
{
	m_pLog->Info("Application will exit.");
	m_pControl->SendMessage(MSG_TYPE::ProgramExit);
	m_pControl->Exit();
}
