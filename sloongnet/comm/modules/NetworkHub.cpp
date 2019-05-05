#include "NetworkHub.h"
#include "epollex.h"
#include "sockinfo.h"
#include "NetworkEvent.hpp"
#include "IData.h"

using namespace Sloong::Events;

Sloong::CNetworkHub::CNetworkHub()
{
	m_pEpoll = make_unique<CEpollEx>();
	m_pWaitProcessList = new queue_ex<SmartPackage>[s_PriorityLevel]();
}

Sloong::CNetworkHub::~CNetworkHub()
{
	if (m_pCTX)
	{
		SSL_CTX_free(m_pCTX);
	}
	for (int i = 0; i < s_PriorityLevel;i++)
	{
		while (!m_pWaitProcessList[i].empty())
		{
			m_pWaitProcessList[i].pop();
        }
	}
	SAFE_DELETE_ARR(m_pWaitProcessList);
}

void Sloong::CNetworkHub::Initialize(IControl *iMsg)
{
	IObject::Initialize(iMsg);

	m_pConfig = IData::GetGlobalConfig();
	m_pEpoll->Initialize(m_iC);

	if (m_pConfig->enablessl())
	{
		EnableSSL(m_pConfig->certfilepath(), m_pConfig->keyfilepath(), m_pConfig->certpasswd());
	}

	m_pEpoll->SetEventHandler(std::bind(&CNetworkHub::OnNewAccept, this, std::placeholders::_1),
							  std::bind(&CNetworkHub::OnDataCanReceive, this, std::placeholders::_1),
							  std::bind(&CNetworkHub::OnCanWriteData, this, std::placeholders::_1),
							  std::bind(&CNetworkHub::OnOtherEventHappened, this, std::placeholders::_1));

	m_iC->RegisterEvent(EVENT_TYPE::SocketClose);
	m_iC->RegisterEvent(EVENT_TYPE::SendMessage);
	m_iC->RegisterEvent(EVENT_TYPE::MonitorSendStatus);
	m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStart, std::bind(&CNetworkHub::Run, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::ProgramExit, std::bind(&CNetworkHub::Exit, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::SendMessage, std::bind(&CNetworkHub::SendMessageEventHandler, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::SocketClose, std::bind(&CNetworkHub::CloseConnectEventHandler, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::MonitorSendStatus, std::bind(&CNetworkHub::MonitorSendStatusEventHandler, this, std::placeholders::_1));
}

void Sloong::CNetworkHub::Run(SmartEvent event)
{
	m_bIsRunning = true;
	m_pEpoll->Run(m_pConfig->listenport(), m_pConfig->epollthreadquantity());
	if( m_nConnectTimeoutTime > 0 && m_nCheckTimeoutInterval > 0 )
		CThreadPool::AddWorkThread(std::bind(&CNetworkHub::CheckTimeoutWorkLoop, this, std::placeholders::_1), nullptr);

	if( m_pProcessFunc == nullptr ){
		m_pLog->Fatal("Process function is null.");
	}

	if( m_pConfig->processthreadquantity() < 1 )
		m_pLog->Fatal("the config value for process work quantity is invalid, please check.");	
		
	CThreadPool::AddWorkThread(std::bind(&CNetworkHub::MessageProcessWorkLoop, this, std::placeholders::_1), nullptr, m_pConfig->processthreadquantity() );
}

void Sloong::CNetworkHub::Exit(SmartEvent event)
{
	m_bIsRunning = false;
	m_oCheckTimeoutThreadSync.notify_all();
	m_oProcessThreadSync.notify_all();
	m_pEpoll->Exit();
}

void Sloong::CNetworkHub::SendMessageEventHandler(SmartEvent event)
{
	auto send_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	SmartPackage pack = send_evt->GetDataPackage();
	int socket = pack->GetSocketID();
	shared_ptr<CSockInfo> info = m_SockList[socket];

	auto res = info->ResponseDataPackage(pack);
	if (res == NetworkResult::Retry)
	{
		m_pEpoll->MonitorSendStatus(socket);
	}
	if (res == NetworkResult::Error)
	{
		SendCloseConnectEvent(socket);
	}
}

void Sloong::CNetworkHub::CloseConnectEventHandler(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto id = net_evt->GetSocketID();
	auto item = m_SockList.find(id);
	if (item == m_SockList.end())
		return;

	shared_ptr<CSockInfo> info = m_SockList[id];
	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_SockList.erase(item);
	sockLck.unlock();
	if (!info)
		return;

	// in here no need delete the send list and read list
	// when delete the SocketInfo object , it will delete the list .
	info->m_pCon->Close();
	m_pLog->Info(CUniversal::Format("close connect:%s:%d.", info->m_pCon->m_strAddress, info->m_pCon->m_nPort));
}

void Sloong::CNetworkHub::MonitorSendStatusEventHandler(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	m_pEpoll->MonitorSendStatus(net_evt->GetSocketID());
}

void Sloong::CNetworkHub::SendCloseConnectEvent(int socket)
{
	shared_ptr<CSockInfo> info = m_SockList[socket];
	if (info == nullptr)
		return;

	auto event = make_shared<CNetworkEvent>(EVENT_TYPE::SocketClose);
	event->SetSocketID(socket);

	//TODO：
	//event->SetUserInfo(info->m_pUserInfo.get());
	m_iC->SendMessage(event);
}

void Sloong::CNetworkHub::EnableClientCheck(const string &clientCheckKey, int clientCheckTime)
{
	m_strClientCheckKey = clientCheckKey;
	m_nClientCheckTime = clientCheckTime;
	m_nClientCheckKeyLength = m_strClientCheckKey.length();
}

void Sloong::CNetworkHub::EnableTimeoutCheck(int timeoutTime, int checkInterval)
{
	m_nConnectTimeoutTime = timeoutTime;
	m_nCheckTimeoutInterval = checkInterval;
}

void Sloong::CNetworkHub::EnableSSL(string certFile, string keyFile, string passwd)
{
	int ret = EasyConnect::G_InitializeSSL(m_pCTX, certFile, keyFile, passwd);
	if (ret != S_OK)
	{
		m_pLog->Error("Initialize SSL environment error.");
		m_pLog->Error(EasyConnect::G_FormatSSLErrorMsg(ret));
	}
}

void Sloong::CNetworkHub::AddMonitorSocket( int nSocket )
{
	auto info = make_shared<CSockInfo>();
	info->Initialize(m_iC, nSocket, m_pCTX);
	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_SockList[nSocket] = info;
	sockLck.unlock();
	m_pEpoll->AddMonitorSocket(nSocket);
}

/*************************************************
* Function: * check_connect_timeout
* Description: * 检测长时间没反应的网络连接，并关闭删除
* Input: *
* Output: *
* Others: *
*************************************************/
void Sloong::CNetworkHub::CheckTimeoutWorkLoop(SMARTER param)
{
	int tout = m_nConnectTimeoutTime * 60;
	int tinterval = m_nCheckTimeoutInterval * 60;

	m_pLog->Verbos("Check connect timeout thread is running.");
	while (m_bIsRunning)
	{
		m_pLog->Verbos("Check connect timeout start.");
	RecheckTimeout:
		unique_lock<mutex> sockLck(m_oSockListMutex);
		for (map<int, shared_ptr<CSockInfo>>::iterator it = m_SockList.begin(); it != m_SockList.end(); ++it)
		{
			if (it->second != NULL && time(NULL) - it->second->m_ActiveTime > tout)
			{
				sockLck.unlock();
				m_pLog->Info(CUniversal::Format("[Timeout]:[Close connect:%s]", it->second->m_pCon->m_strAddress));
				SendCloseConnectEvent(it->first);
				goto RecheckTimeout;
			}
		}
		sockLck.unlock();
		m_pLog->Verbos(CUniversal::Format("Check connect timeout done. wait [%d] seconds.", tinterval));
		m_oCheckTimeoutThreadSync.wait_for(tinterval);
	}
	m_pLog->Info("check timeout connect thread is exit ");
}



/// 消息处理工作线程函数
// 按照优先级，逐个处理待处理队列。每个队列处理完毕时，重新根据优先级处理，尽量保证高优先级的处理
// 为了避免影响接收时的效率，将队列操作放松到最低。以每次一定数量的处理来逐级加锁。
void Sloong::CNetworkHub::MessageProcessWorkLoop(SMARTER param)
{
	m_pLog->Verbos("MessageProcessWorkLoop thread is running.");
	while (m_bIsRunning)
	{
		for( int i = 0; i < s_PriorityLevel; i++ )
		{
			if( m_pWaitProcessList[i].empty())
				continue;
			while( !m_pWaitProcessList[i].empty() )
			{
				auto param = m_pWaitProcessList[i].pop();
				m_pProcessFunc(param);
			}
			break;
		}
		m_oProcessThreadSync.wait();
	}
	m_pLog->Info("MessageProcessWorkLoop thread is exit ");
}


/// 有新链接到达。
/// 接收链接之后，需要客户端首先发送客户端校验信息。只有校验成功之后才会进行SSL处理
NetworkResult Sloong::CNetworkHub::OnNewAccept(int conn_sock)
{
	m_pLog->Verbos("Accept function is called.");

	// start client check when acdept
	if (m_nClientCheckKeyLength > 0)
	{
		char *pCheckBuf = new char[m_nClientCheckKeyLength + 1];
		memset(pCheckBuf, 0, m_nClientCheckKeyLength + 1);
		// In Check function, client need send the check key in 3 second.
		// 这里仍然使用Universal提供的ReceEx。这里不需要进行SSL接收
		int nLen = CUniversal::RecvEx(conn_sock, pCheckBuf, m_nClientCheckKeyLength, m_nClientCheckTime);
		if (nLen != m_nClientCheckKeyLength || 0 != strcmp(pCheckBuf, m_strClientCheckKey.c_str()))
		{
			m_pLog->Warn(CUniversal::Format("Check Key Error.Length[%d]:[%d].Server[%s]:[%s]Client", m_nClientCheckKeyLength, nLen, m_strClientCheckKey.c_str(), pCheckBuf));
			return NetworkResult::Error;
		}
	}

	auto info = make_shared<CSockInfo>();
	info->Initialize(m_iC, conn_sock, m_pCTX);
	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_SockList[conn_sock] = info;
	sockLck.unlock();

	if( m_pAcceptFunc ){
		m_pAcceptFunc(info);
	}

	m_pLog->Info(CUniversal::Format("Accept client:[%s:%d].", info->m_pCon->m_strAddress, info->m_pCon->m_nPort));

	return NetworkResult::Succeed;
}



NetworkResult Sloong::CNetworkHub::OnDataCanReceive(int nSocket)
{
	shared_ptr<CSockInfo> info = m_SockList[nSocket];

	if (info == NULL)
	{
		m_pLog->Error("OnDataCanReceive called, but SockInfo is null.");
		return NetworkResult::Error;
	}

	queue<SmartPackage> readList;
	auto res = info->OnDataCanReceive(readList);
	if (res == NetworkResult::Error)
		SendCloseConnectEvent(nSocket);

	if( !readList.empty())
	{
		auto item = readList.front();
		readList.pop();
		m_pWaitProcessList[item->GetPriority()].push(item);
	}
	m_oProcessThreadSync.notify_all();
	return res;
}

NetworkResult Sloong::CNetworkHub::OnCanWriteData(int nSocket)
{
	// 可以写入事件
	shared_ptr<CSockInfo> info = m_SockList[nSocket];

	if (info == NULL)
	{
		m_pLog->Error("OnCanWriteData called, but SockInfo is null.");
		return NetworkResult::Error;
	}

	auto res = info->OnDataCanSend();
	if (res == NetworkResult::Error)
		SendCloseConnectEvent(nSocket);
	return res;
}

NetworkResult Sloong::CNetworkHub::OnOtherEventHappened(int nSocket)
{
	SendCloseConnectEvent(nSocket);
	return NetworkResult::Succeed;
}
