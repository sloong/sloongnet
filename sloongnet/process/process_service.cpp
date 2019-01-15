/* File Name: server.c */
#include "serverconfig.h"
#include "CmdProcess.h"
#include "process_service.h"
#include "ControlCenter.h"
#include "NormalEvent.h"
#include "IData.h"
using namespace Sloong::Events;

IControl* Sloong::IData::m_iC = nullptr;


CServerConfig* g_pConfig = nullptr;


void sloong_terminator() 
{
	cout << "Unkonw error happened, system will shutdown. " << endl;
	write_call_stack();
	exit(0);
}

void on_sigint(int signal)
{
	cout << "Unhandle signal happened, system will shutdown. signal:" << signal<< endl;
	write_call_stack();
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
	m_pProcess = make_unique<CLuaProcessCenter>();
}

SloongNetProcess::~SloongNetProcess()
{
	Exit();
	CThreadPool::Exit();
    m_pLog->End();
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
		CServerConfig config;
		// CmdProcess会根据参数来加载正确的配置信息。成功返回true。
		if (CCmdProcess::Parser(argc, args, &config))
		{
			g_pConfig = &config;
			LOGTYPE oType = LOGTYPE::ONEFILE;
			if (!config->m_oLogInfo.LogWriteToOneFile)
			{
				oType = LOGTYPE::DAY;
			}
			m_pLog->Initialize(config->m_oLogInfo.LogPath, config->m_oLogInfo.DebugMode, LOGLEVEL(config->m_oLogInfo.LogLevel), oType);
			if (config->m_oLogInfo.NetworkPort != 0)
				m_pLog->EnableNetworkLog(config->m_oLogInfo.NetworkPort);
				
			Add(Configuation, config);
			Add(Logger, m_pLog.get());
			
			CThreadPool::AddWorkThread(std::bind(&SloongNetProcess::MessageWorkLoop, this, std::placeholders::_1), nullptr, config->m_nMessageCenterThreadQuantity);
			
			RegisterEvent(ProgramExit);
			RegisterEvent(ProgramStart);
			RegisterEventHandler(MSG_TYPE::ProgramExit, std::bind(&SloongNetProcess::ExitEventHandler, this, std::placeholders::_1));
			m_pNetwork->Initialize(m_iC);
			m_pProcess->Initialize(m_iC);
			
			// 在所有的成员都初始化之后，在注册处理函数
			m_iC->RegisterEventHandler(ReveivePackage, std::bind(&SloongNetProcess::OnReceivePackage, this, std::placeholders::_1));
			m_iC->RegisterEventHandler(SocketClose, std::bind(&SloongNetProcess::OnSocketClose, this, std::placeholders::_1));

			try{
				IData::Initialize(this);
				m_pCC->Initialize(this);
			}
			catch(exception e){
				m_pLog->Error(string("Excepiton happened in initialize for ControlCenter. Message:")+  string(e.what()));
				return false;
			}
		}
	}
	catch (exception& e)
	{
		cout << "exception happened, system will shutdown. message:" << e.what() << endl;
		return false;
	}
	catch(normal_except& e)
    {
        cout << "exception happened, system will shutdown. message:" << e.what() << endl;
		return false;
    }
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. "<< endl;
		write_call_stack();
		return false;
	}
	
	return true;
}

void SloongNetProcess::Run()
{
	m_pLog->Info("Application begin running.");
	m_emStatus = RUN_STATUS::Running;
	CThreadPool::Run();
	SendMessage(MSG_TYPE::ProgramStart);
	while (m_emStatus != RUN_STATUS::Exit)
	{
		m_oSync.wait();
	}
}


void Sloong::CControlCenter::OnReceivePackage(SmartEvent evt)
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
	char* pExData = nullptr;
	int nExSize;
	string strMsg = pack->GetRecvMessage();
	if (m_pProcess->MsgProcess(info, strMsg , strRes, pExData, nExSize)){
		pack->ResponsePackage(strRes,pExData,nExSize);
	}else{
		m_pLog->Error("Error in process");
		pack->ResponsePackage("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
	}
	
	net_evt->SetDataPackage(pack);
	m_iC->SendMessage(net_evt);
}

void Sloong::CControlCenter::OnSocketClose(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	// call close function.
	m_pProcess->CloseSocket(info);
	net_evt->CallCallbackFunc(net_evt);
}

void Sloong::SloongNetProcess::Exit()
{
	m_pLog->Info("Application will exit.");
	SendMessage(MSG_TYPE::ProgramExit);
}

void Sloong::SloongNetProcess::ExitEventHandler(SmartEvent ev)
{
	m_emStatus = RUN_STATUS::Exit;
	m_oSync.notify_all();
}

bool Sloong::SloongNetProcess::Add(DATA_ITEM item, void * object)
{
	auto it = m_oDataList.find(item);
	if (it != m_oDataList.end())
	{
		return false;
	}
	m_oDataList.insert(make_pair(item, object));
	return false;
}

void * Sloong::SloongNetProcess::Get(DATA_ITEM item)
{
	auto data = m_oDataList.find(item);
	if (data == m_oDataList.end())
	{
		return nullptr;
	}
	return (*data).second;
}

bool Sloong::SloongNetProcess::Remove(DATA_ITEM item)
{
	m_oDataList.erase(item);
}

bool Sloong::SloongNetProcess::AddTemp(string name, void * object)
{
	m_oTempDataList[name] = object;
	return true;
}

void * Sloong::SloongNetProcess::GetTemp(string name)
{
	auto item = m_oTempDataList.find(name);
	if (item == m_oTempDataList.end())
	{
		return nullptr;
	}
	m_oTempDataList.erase(name);
	return (*item).second;
}



void Sloong::SloongNetProcess::SendMessage(MSG_TYPE msgType)
{
	auto evt = make_shared<CNormalEvent>();
	evt->SetEvent(msgType);
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oSync.notify_one();
}

void Sloong::SloongNetProcess::SendMessage(SmartEvent evt)
{
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oSync.notify_one();
}


void Sloong::SloongNetProcess::RegisterEvent(MSG_TYPE t)
{
	m_oMsgHandlerList[t] = vector<MsgHandlerFunc>();
}

/**
 * @Remarks: One message only have one handler. so cannot register handled message again.
 * @Params: 
 * @Return: 
 */
void Sloong::SloongNetProcess::RegisterEventHandler(MSG_TYPE t, MsgHandlerFunc func)
{
	if (m_oMsgHandlerList.find(t) == m_oMsgHandlerList.end())
	{
		throw normal_except("Target event is not regist.");
	}
	else{
		m_oMsgHandlerList[t].push_back(func);
	}
}


void Sloong::SloongNetProcess::MessageWorkLoop(SMARTER param)
{
	while (m_emStatus != RUN_STATUS::Exit)
	{
		try
		{
			if (m_emStatus == RUN_STATUS::Created)
			{
				SLEEP(100);
				continue;
			}
			if (m_oMsgList.empty())
			{
				m_oSync.wait_for(1);
				continue;
			}
			if (!m_oMsgList.empty())
			{
				unique_lock<mutex> lck(m_oMsgListMutex);
				if (m_oMsgList.empty())
				{
					lck.unlock();
					continue;
				}

				auto p = m_oMsgList.front();
				m_oMsgList.pop();
				lck.unlock();

				// Get the message handler list.
				auto evt_type = p->GetEvent();
				auto handler_list = m_oMsgHandlerList[evt_type];
				int handler_num = handler_list.size();
				if ( handler_num == 0 )
					continue;

				for (int i = 0; i < handler_num; i++)
				{
					auto func = handler_list[i];
					func(p);
				}
			}
		}
		catch (...)
		{
			cerr << "Unhandle exception in MessageCenter work loop." << endl;
		}
		
	}
}


