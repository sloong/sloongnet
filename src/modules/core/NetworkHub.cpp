#include "NetworkHub.h"
#include "epollex.h"
#include "sockinfo.h"
#include "NetworkEvent.hpp"
#include "SendPackageEvent.hpp"
#include "IData.h"

using namespace Sloong::Events;

Sloong::CNetworkHub::CNetworkHub()
{
	m_pEpoll = make_unique<CEpollEx>();
	m_pWaitProcessList = new queue_ex<UniqueTransPackage>[s_PriorityLevel]();
}

Sloong::CNetworkHub::~CNetworkHub()
{
	if (m_pCTX)
		EasyConnect::G_FreeSSL(m_pCTX);
	for (int i = 0; i < s_PriorityLevel; i++)
	{
		while (!m_pWaitProcessList[i].empty())
		{
			m_pWaitProcessList[i].front().reset();
			m_pWaitProcessList[i].pop_move();
		}
	}
	SAFE_DELETE_ARR(m_pWaitProcessList);
}

CResult Sloong::CNetworkHub::Initialize(IControl *iMsg)
{
	IObject::Initialize(iMsg);

	m_pConfig = IData::GetGlobalConfig();
	auto res = m_pEpoll->Initialize(m_iC);
	if (res.IsFialed())
		return res;

	if (m_pConfig->enablessl())
	{
		EnableSSL(m_pConfig->certfilepath(), m_pConfig->keyfilepath(), m_pConfig->certpasswd());
	}

	m_pEpoll->SetEventHandler(std::bind(&CNetworkHub::OnNewAccept, this, std::placeholders::_1),
							  std::bind(&CNetworkHub::OnDataCanReceive, this, std::placeholders::_1),
							  std::bind(&CNetworkHub::OnCanWriteData, this, std::placeholders::_1),
							  std::bind(&CNetworkHub::OnOtherEventHappened, this, std::placeholders::_1));

	m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStart, std::bind(&CNetworkHub::Run, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStop, std::bind(&CNetworkHub::Exit, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::SendPackage, std::bind(&CNetworkHub::SendPackageEventHandler, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::SocketClose, std::bind(&CNetworkHub::CloseConnectEventHandler, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::MonitorSendStatus, std::bind(&CNetworkHub::MonitorSendStatusEventHandler, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::RegisteConnection, std::bind(&CNetworkHub::RegisteConnectionEventHandler, this, std::placeholders::_1));
	return CResult::Succeed();
}

void Sloong::CNetworkHub::Run(SharedEvent event)
{
	m_bIsRunning = true;
	m_pEpoll->Run();
	if (m_nConnectTimeoutTime > 0 && m_nCheckTimeoutInterval > 0)
		CThreadPool::AddWorkThread(std::bind(&CNetworkHub::CheckTimeoutWorkLoop, this));

	if (m_pRequestFunc == nullptr)
	{
		m_pLog->Fatal("Process function is null.");
		m_iC->SendMessage(EVENT_TYPE::ProgramStop);
	}

	if (m_pConfig->processthreadquantity() < 1)
		m_pLog->Fatal("the config value for process work quantity is invalid, please check.");

	CThreadPool::AddWorkThread(std::bind(&CNetworkHub::MessageProcessWorkLoop, this), m_pConfig->processthreadquantity());
}

void Sloong::CNetworkHub::Exit(SharedEvent event)
{
	m_bIsRunning = false;
	m_oCheckTimeoutThreadSync.notify_all();
	m_oProcessThreadSync.notify_all();
	m_pEpoll->Exit();
}

void Sloong::CNetworkHub::SendPackageEventHandler(SharedEvent event)
{
	auto send_evt = DYNAMIC_TRANS<CSendPackageEvent *>(event.get());
	auto socket = send_evt->GetSocketID();
	if (!m_SockList.exist(socket))
	{
		m_pLog->Error(Helper::Format("SendPackageEventHandler function called, but the socket[%d] is no regiestd in NetworkHub.", socket));
		return;
	}
	auto info = m_SockList[socket].get();
	if (send_evt->HaveCallbackFunc())
		m_iC->AddTempSharedPtr(Helper::ntos(send_evt->GetDataPackage()->id()), event);

	auto transPack = make_unique<CDataTransPackage>(info->m_pCon.get());
	transPack->RequestPackage(*send_evt->GetDataPackage());

	AddMessageToSendList(transPack);
}

void Sloong::CNetworkHub::AddMessageToSendList(UniqueTransPackage &pack)
{
	int socket = pack->GetSocketID();
	if (!m_SockList.exist(socket))
	{
		m_pLog->Error(Helper::Format("AddMessageToSendList function called, but the socket[%d] is no regiestd in NetworkHub.", socket));
		return;
	}

	auto info = m_SockList[socket].get();
	auto res = info->SendDataPackage(std::move(pack));
	if (res == ResultType::Retry)
	{
		m_pEpoll->MonitorSendStatus(socket);
	}
	if (res == ResultType::Error)
	{
		SendCloseConnectEvent(socket);
	}
}

void Sloong::CNetworkHub::CloseConnectEventHandler(SharedEvent event)
{
	auto net_evt = DYNAMIC_TRANS<CNetworkEvent *>(event.get());
	auto id = net_evt->GetSocketID();
	if (!m_SockList.exist(id))
		return;

	auto info = m_SockList[id].get();
	m_pLog->Info(Helper::Format("close connect:%s:%d.", info->m_pCon->m_strAddress.c_str(), info->m_pCon->m_nPort));
	m_pEpoll->DeleteMonitorSocket(id);
	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_SockList.erase(id);
	sockLck.unlock();
}

void Sloong::CNetworkHub::MonitorSendStatusEventHandler(SharedEvent event)
{
	auto net_evt = DYNAMIC_TRANS<CNetworkEvent *>(event.get());
	m_pEpoll->MonitorSendStatus(net_evt->GetSocketID());
}

void Sloong::CNetworkHub::RegisteConnectionEventHandler(SharedEvent event)
{
	auto net_evt = DYNAMIC_TRANS<CNetworkEvent *>(event.get());
	auto nSocket = net_evt->GetSocketID();

	auto info = make_unique<CSockInfo>();
	info->Initialize(m_iC, nSocket, m_pCTX);
	m_pLog->Info(Helper::Format("Registe connection:[%d][%s:%d].", nSocket, info->m_pCon->m_strAddress.c_str(), info->m_pCon->m_nPort));

	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_SockList[nSocket] = std::move(info);
	sockLck.unlock();
	m_pEpoll->AddMonitorSocket(nSocket);
}

void Sloong::CNetworkHub::SendCloseConnectEvent(int socket)
{
	if (!m_SockList.exist(socket))
		return;

	auto event = make_shared<CNetworkEvent>(EVENT_TYPE::SocketClose);
	event->SetSocketID(socket);

	m_iC->SendMessage(event);
}

void Sloong::CNetworkHub::EnableClientCheck(const string &clientCheckKey, int clientCheckTime)
{
	m_strClientCheckKey = clientCheckKey;
	m_nClientCheckTime = clientCheckTime;
	m_nClientCheckKeyLength = (int)m_strClientCheckKey.length();
}

void Sloong::CNetworkHub::EnableTimeoutCheck(int timeoutTime, int checkInterval)
{
	m_nConnectTimeoutTime = timeoutTime;
	m_nCheckTimeoutInterval = checkInterval;
}

void Sloong::CNetworkHub::EnableSSL(const string &certFile, const string &keyFile, const string &passwd)
{
	auto ret = EasyConnect::G_InitializeSSL(&m_pCTX, certFile, keyFile, passwd);
	if (ret != S_OK)
	{
		m_pLog->Error("Initialize SSL environment error.");
		m_pLog->Error(EasyConnect::G_FormatSSLErrorMsg(ret));
	}
}

/*************************************************
* Function: * check_connect_timeout
* Description: * 检测长时间没反应的网络连接，并关闭删除
* Input: *
* Output: *
* Others: *
*************************************************/
void Sloong::CNetworkHub::CheckTimeoutWorkLoop()
{
	int tout = m_nConnectTimeoutTime * 60;
	int tinterval = m_nCheckTimeoutInterval * 60 * 1000;

	m_pLog->Debug("Check connect timeout thread is running.");
	while (m_bIsRunning)
	{
		m_pLog->Debug("Check connect timeout start.");
	RecheckTimeout:
		for (auto it = m_SockList.begin(); it != m_SockList.end(); ++it)
		{
			if (it->second != NULL && time(NULL) - it->second->m_ActiveTime > tout)
			{
				m_pLog->Info(Helper::Format("[Timeout]:[Close connect:%s]", it->second->m_pCon->m_strAddress.c_str()));
				SendCloseConnectEvent(it->first);
				goto RecheckTimeout;
			}
		}
		m_pLog->Debug(Helper::Format("Check connect timeout done. wait [%d] ms.", tinterval));
		m_oCheckTimeoutThreadSync.wait_for(tinterval);
	}
	m_pLog->Info("check timeout connect thread is exit ");
}

/// 消息处理工作线程函数
// 按照优先级，逐个处理待处理队列。每个队列处理完毕时，重新根据优先级处理，尽量保证高优先级的处理
// 为了避免影响接收时的效率，将队列操作放松到最低。以每次一定数量的处理来逐级加锁。
void Sloong::CNetworkHub::MessageProcessWorkLoop()
{
	m_pLog->Debug("MessageProcessWorkLoop thread is running.");
	void *pEnv;
	m_pLog->Verbos("Call module create process environment function.");
	auto res = m_pCreateEnvFunc(&pEnv);
	if (res.IsFialed())
	{
		m_pLog->Fatal(res.GetMessage());
		m_iC->SendMessage(EVENT_TYPE::ProgramStop);
		return;
	}
	if (pEnv == nullptr)
	{
		m_pLog->Warn("Create called succeed, but the evnironment value is null.");
	}
	else
	{
		m_pLog->Verbos("Create called succeed.");
	}

	UniqueTransPackage pack;
	DataPackage *data_pack = nullptr;
	while (m_bIsRunning)
	{
		try
		{
		MessagePorcessListRetry:
			for (int i = 0; i < s_PriorityLevel; i++)
			{
				if (m_pWaitProcessList[i].empty())
					continue;

				while (m_pWaitProcessList[i].TryMovePop(pack))
				{
					// In here, the result no the result for this request.
					// it just for is need add the pack obj to send list.
					res.SetResult(ResultType::Ignore);

					pack->Record();
					data_pack = pack->GetDataPackage();
					switch (data_pack->type())
					{
					case DataPackage_PackageType::DataPackage_PackageType_EventPackage:
					{
						switch (data_pack->function())
						{
						case ControlEvent::Restart:
							m_pLog->Info("Received restart control event. application will restart.");
							m_iC->SendMessage(EVENT_TYPE::ProgramRestart);
							break;
						case ControlEvent::Stop:
							m_pLog->Info("Received stop control event. application will stop.");
							m_iC->SendMessage(EVENT_TYPE::ProgramStop);
							break;
						default:
							m_pEventFunc(pack.get());
							break;
						}
					}
					break;
					case DataPackage_PackageType::DataPackage_PackageType_RequestPackage:
					{
						if (data_pack->status() == DataPackage_StatusType::DataPackage_StatusType_Request)
							res = m_pRequestFunc(pEnv, pack.get());
						else
							res = m_pResponseFunc(pEnv, pack.get());
					}
					break;
					default:
						m_pLog->Warn("Data package check type error. cannot process.");
					}
					pack->Record();
					data_pack = nullptr;
					if (res.IsSucceed())
						AddMessageToSendList(pack);
					else
						pack = nullptr;
				}
				goto MessagePorcessListRetry;
			}
			m_oProcessThreadSync.wait_for(1000);
		}
		catch (...)
		{
			m_pLog->Error("Unknown exception happened in MessageProcessWorkLoop");
		}
	}
	m_pLog->Info("MessageProcessWorkLoop thread is exit ");
}

/// 有新链接到达。
/// 接收链接之后，需要客户端首先发送客户端校验信息。只有校验成功之后才会进行SSL处理
ResultType Sloong::CNetworkHub::OnNewAccept(int conn_sock)
{
	m_pLog->Debug("Accept function is called.");

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
			m_pLog->Warn(Helper::Format("Check Key Error.Length[%d]:[%d].Server[%s]:[%s]Client", m_nClientCheckKeyLength, nLen, m_strClientCheckKey.c_str(), pCheckBuf));
			return ResultType::Error;
		}
	}

	auto info = make_unique<CSockInfo>();
	info->Initialize(m_iC, conn_sock, m_pCTX);

	if (m_pAcceptFunc)
	{
		m_pAcceptFunc(info.get());
	}

	m_pLog->Info(Helper::Format("Accept client:[%s:%d].", info->m_pCon->m_strAddress.c_str(), info->m_pCon->m_nPort));
	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_SockList[conn_sock] = std::move(info);
	sockLck.unlock();
	return ResultType::Succeed;
}

ResultType Sloong::CNetworkHub::OnDataCanReceive(int nSocket)
{
	if (!m_SockList.exist(nSocket))
	{
		m_pLog->Error("OnDataCanReceive called, but SockInfo is null.");
		return ResultType::Error;
	}
	auto info = m_SockList[nSocket].get();
	if (!info->TryReceiveLock())
		return ResultType::Invalid;

	m_pLog->Verbos("OnDataCanReceive called, start receiving package.");
	queue<unique_ptr<CDataTransPackage>> readList;
	auto res = info->OnDataCanReceive(readList);
	if (res == ResultType::Error)
		SendCloseConnectEvent(nSocket);

	m_pLog->Verbos(Helper::Format("OnDataCanReceive done. received [%d] packages.", readList.size()));
	while (!readList.empty())
	{
		auto t = std::move(readList.front());
		readList.pop();
		auto pack = t->GetDataPackage();
		if (pack->type() == DataPackage_PackageType::DataPackage_PackageType_RequestPackage && pack->status() == DataPackage_StatusType::DataPackage_StatusType_Response)
		{
			auto event = static_pointer_cast<IEvent>(m_iC->GetTempSharedPtr(Helper::ntos(t->GetSerialNumber())));
			shared_ptr<CSendPackageEvent> send_evt = nullptr;
			if (event != nullptr)
				send_evt = dynamic_pointer_cast<CSendPackageEvent>(event);
			if (send_evt != nullptr)
			{
				auto shared_pack = shared_ptr<CDataTransPackage>(move(t));
				CThreadPool::EnqueTask([send_evt, shared_pack](SMARTER p) {
					send_evt->CallCallbackFunc(shared_pack.get());
					return (SMARTER)nullptr;
				});
				continue;
			}
		}
		m_pWaitProcessList[readList.front()->GetPriority()].push_move(std::move(t));
	}
	m_oProcessThreadSync.notify_all();
	return res;
}

ResultType Sloong::CNetworkHub::OnCanWriteData(int nSocket)
{
	if (!m_SockList.exist(nSocket))
	{
		m_pLog->Error("OnCanWriteData called, but SockInfo is null.");
		return ResultType::Error;
	}
	auto info = m_SockList[nSocket].get();

	if (!info->TrySendLock())
		return ResultType::Invalid;

	auto res = info->OnDataCanSend();
	if (res == ResultType::Error)
		SendCloseConnectEvent(nSocket);
	return res;
}

ResultType Sloong::CNetworkHub::OnOtherEventHappened(int nSocket)
{
	SendCloseConnectEvent(nSocket);
	return ResultType::Succeed;
}
