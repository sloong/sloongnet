/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2019-11-05 08:59:19
 * @LastEditTime: 2020-07-31 14:37:54
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/NetworkHub.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#include "NetworkHub.h"
#include "EpollEx.h"
#include "ConnectSession.h"
#include "IData.h"

#include "events/SendPackage.hpp"
#include "events/ConnectionBreak.hpp"
#include "events/MonitorSendStatus.hpp"
#include "events/RegisteConnection.hpp"
#include "events/GetConnectionInfo.hpp"
using namespace Sloong::Events;

Sloong::CNetworkHub::CNetworkHub()
{
	m_pEpoll = make_unique<CEpollEx>();
	m_pWaitProcessList = new queue_ex<UniquePackage>[s_PriorityLevel]();
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
	m_iC->RegisterEventHandler(EVENT_TYPE::ConnectionBreaked, std::bind(&CNetworkHub::OnConnectionBreakedEventHandler, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::MonitorSendStatus, std::bind(&CNetworkHub::MonitorSendStatusEventHandler, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::RegisteConnection, std::bind(&CNetworkHub::RegisteConnectionEventHandler, this, std::placeholders::_1));
	m_iC->RegisterEventHandler(EVENT_TYPE::GetConnectionInfo, std::bind(&CNetworkHub::OnGetConnectionInfoEventHandler, this, std::placeholders::_1));

	if (m_pRequestFunc == nullptr)
	{
		m_pLog->Fatal("Process function is null.");
		m_iC->SendMessage(EVENT_TYPE::ProgramStop);
	}

	if (m_pConfig->processthreadquantity() < 1)
		m_pLog->Fatal("the config value for process work quantity is invalid, please check.");

	CThreadPool::AddWorkThread(std::bind(&CNetworkHub::MessageProcessWorkLoop, this), m_pConfig->processthreadquantity());

	m_emStatus = RUN_STATUS::Running;
	m_pEpoll->Run();
	if (m_nConnectTimeoutTime > 0 && m_nCheckTimeoutInterval > 0)
		CThreadPool::AddWorkThread(std::bind(&CNetworkHub::CheckTimeoutWorkLoop, this));
		
	return CResult::Succeed;
}

void Sloong::CNetworkHub::Run(SharedEvent event)
{
	
}

void Sloong::CNetworkHub::Exit(SharedEvent event)
{
	m_emStatus = RUN_STATUS::Exit;
	m_oCheckTimeoutThreadSync.notify_all();
	m_oProcessThreadSync.notify_all();
	m_pEpoll->Exit();
}

void Sloong::CNetworkHub::SendPackageEventHandler(SharedEvent event)
{
	auto send_evt = DYNAMIC_TRANS<SendPackageEvent *>(event.get());
	auto id = send_evt->GetConnectionHashCode();
	if (!m_mapConnectIDToSession.exist(id))
	{
		m_pLog->Error(Helper::Format("SendPackageEventHandler function called, but the session[%lld] is no regiestd in NetworkHub.", socket));
		return;
	}

	if (send_evt->HaveCallbackFunc())
		m_iC->AddTempSharedPtr(Helper::ntos(send_evt->GetDataPackage()->id()), event);

	AddMessageToSendList(send_evt->MoveDataPackage());
}

void Sloong::CNetworkHub::AddMessageToSendList(UniquePackage pack)
{
	auto sessionid = pack->reserved().sessionid();
	if (!m_mapConnectIDToSession.exist(sessionid))
	{
		m_pLog->Error(Helper::Format("AddMessageToSendList function called, but the session[%lld] is no regiestd in NetworkHub.", sessionid));
		return;
	}

	auto session = m_mapConnectIDToSession[sessionid].get();
	auto res = session->SendDataPackage(std::move(pack));
	if (res == ResultType::Retry)
	{
		m_pEpoll->MonitorSendStatus(sessionid);
	}
	if (res == ResultType::Error)
	{
		SendConnectionBreak(sessionid);
	}
}

void Sloong::CNetworkHub::OnConnectionBreakedEventHandler(SharedEvent e)
{
	auto event = DYNAMIC_TRANS<ConnectionBreakedEventn *>(e.get());
	auto id = event->GetSessionID();
	if (!m_mapConnectIDToSession.exist(id))
		return;

	auto info = m_mapConnectIDToSession[id].get();
	auto socket = info->m_pConnection->GetSocketID();
	m_pLog->Info(Helper::Format("close connect:[%d]%s:%d.", socket, info->m_pConnection->m_strAddress.c_str(), info->m_pConnection->m_nPort));

	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_mapConnectIDToSession.erase(id);
	sockLck.unlock();

	m_pEpoll->DeleteMonitorSocket(socket);
}

void Sloong::CNetworkHub::MonitorSendStatusEventHandler(SharedEvent e)
{
	auto event = DYNAMIC_TRANS<MonitorSendStatusEvent *>(e.get());
	auto id = event->GetSessionID();
	if (!m_mapConnectIDToSession.exist(id))
		return;

	auto info = m_mapConnectIDToSession[id].get();
	m_pLog->Info(Helper::Format("MonitorSendStatus:%s:%d.", info->m_pConnection->m_strAddress.c_str(), info->m_pConnection->m_nPort));
	m_pEpoll->MonitorSendStatus(info->m_pConnection->GetSocketID());
}

void Sloong::CNetworkHub::RegisteConnectionEventHandler(SharedEvent e)
{
	auto event = DYNAMIC_TRANS<RegisteConnectionEvent *>(e.get());
	if (event == nullptr)
	{
		m_pLog->Error("RegisteConnectionEventHandler is called, but param type error.");
		return;
	}

	auto connect = make_unique<EasyConnect>();
	connect->InitializeAsClient(event->GetAddress(), event->GetPort(), m_pCTX);
	connect->Connect();

	auto info = make_unique<ConnectSession>();
	info->Initialize(m_iC, std::move(connect));
	auto socket = info->m_pConnection->GetSocketID();
	auto sessionid = info->m_pConnection->GetHashCode();
	m_pLog->Info(Helper::Format("Registe connection:[%d][%lld][%s:%d].", socket, sessionid, info->m_pConnection->m_strAddress.c_str(), info->m_pConnection->m_nPort));

	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_mapSocketToSessionID[socket] = sessionid;
	m_mapConnectIDToSession[sessionid] = std::move(info);
	m_pEpoll->AddMonitorSocket(socket);
	sockLck.unlock();

	event->CallCallbackFunc(sessionid);
}

void Sloong::CNetworkHub::OnGetConnectionInfoEventHandler(SharedEvent e)
{
	auto event = DYNAMIC_TRANS<GetConnectionInfoEvent *>(e.get());
	if (event == nullptr)
		return;

	auto info = m_mapConnectIDToSession.try_get(event->GetSessionID());
	if (info == nullptr)
		return;

	auto pConn = (*info)->m_pConnection.get();
	event->CallCallbackFunc(ConnectionInfo{Address : pConn->m_strAddress, Port : pConn->m_nPort});
}

inline void Sloong::CNetworkHub::SendConnectionBreak(int64_t sessionid)
{
	if (!m_mapConnectIDToSession.exist(sessionid))
		return;

	m_iC->SendMessage(make_shared<ConnectionBreakedEventn>(sessionid));
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
	while (m_emStatus != RUN_STATUS::Exit)
	{
		m_pLog->Debug("Check connect timeout start.");
	RecheckTimeout:
		for (auto it = m_mapConnectIDToSession.begin(); it != m_mapConnectIDToSession.end(); ++it)
		{
			if (it->second != NULL && time(NULL) - it->second->m_ActiveTime > tout)
			{
				m_pLog->Info(Helper::Format("[Timeout]:[Close connect:%s]", it->second->m_pConnection->m_strAddress.c_str()));
				SendConnectionBreak(it->first);
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
	auto pid = this_thread::get_id();
	string spid = Helper::ntos(pid);

	m_pLog->Info("Network hub work thread is started. PID:" + spid);

	while (m_emStatus == RUN_STATUS::Created)
	{
		this_thread::sleep_for(std::chrono::microseconds(100));
	}

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

	m_pLog->Info("Network hub work thread is running. PID:" + spid);
	while (m_emStatus != RUN_STATUS::Exit)
	{
		try
		{
		MessagePorcessListRetry:
			for (int i = 0; i < s_PriorityLevel; i++)
			{
				if (m_pWaitProcessList[i].empty())
					continue;

				PackageResult result(ResultType::Ignore);
				UniquePackage package = m_pWaitProcessList[i].TryMovePop();
				while (package != nullptr)
				{
					// In here, the result no the result for this request.
					// it just for is need add the pack obj to send list.
					result.SetResult(ResultType::Ignore);

					

					package->mutable_reserved()->add_clocks(GetClock());
					switch (package->type())
					{
					case DataPackage_PackageType::DataPackage_PackageType_EventPackage:
					{
						m_pLog->Verbos( "Event package <<< " +  package->ShortDebugString() );
						switch (package->function())
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
							m_pEventFunc(package.get());
							break;
						}
					}
					break;
					case DataPackage_PackageType::DataPackage_PackageType_NormalPackage:
					{
						m_pLog->Verbos( "Process package <<< " +  package->ShortDebugString() );
						if (package->status() == DataPackage_StatusType::DataPackage_StatusType_Request)
							result = m_pRequestFunc(pEnv, package.get());
						else
							result = m_pResponseFunc(pEnv, package.get());

						if (result.HaveResultObject())
						{
							auto response = result.MoveResultObject();
							m_pLog->Verbos( "Response package >>> " +  response->ShortDebugString() );
							AddMessageToSendList(move(response));
						}
						else if (result.GetResult() != ResultType::Ignore)
						{
							auto response = Package::MakeResponse(package.get());
							response->set_result(result.GetResult());
							response->set_content(result.GetMessage());
							m_pLog->Verbos( "Response package >>> " +  response->ShortDebugString());
							AddMessageToSendList(move(response));
						}
						else
						{
							m_pLog->Error(Helper::Format("Result[%s]", ResultType_Name(result.GetResult()).c_str()));
						}
					}
					break;
					default:
						m_pLog->Warn("Data package check type error. cannot process.");
					}
					package = nullptr;
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
	m_pLog->Info("Network hub work thread is exit " + spid);
}

/// 有新链接到达。
/// 接收链接之后，需要客户端首先发送客户端校验信息。只有校验成功之后才会进行SSL处理
ResultType Sloong::CNetworkHub::OnNewAccept(SOCKET conn_sock)
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

	if (m_pAcceptFunc)
	{
		auto res = m_pAcceptFunc(conn_sock);
		if (res.IsFialed())
		{
			return ResultType::Error;
		}
	}

	auto conn = make_unique<EasyConnect>();
	conn->InitializeAsServer(conn_sock, m_pCTX);

	auto info = make_unique<ConnectSession>();
	info->Initialize(m_iC, std::move(conn));

	m_pLog->Info(Helper::Format("Accept client:[%d][%d][%s:%d].", info->m_pConnection->GetHashCode(), info->m_pConnection->GetSocketID(), info->m_pConnection->m_strAddress.c_str(), info->m_pConnection->m_nPort));
	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_mapSocketToSessionID[conn_sock] = info->m_pConnection->GetHashCode();
	m_mapConnectIDToSession[info->m_pConnection->GetHashCode()] = std::move(info);
	sockLck.unlock();
	return ResultType::Succeed;
}

ResultType Sloong::CNetworkHub::OnDataCanReceive(SOCKET socket)
{
	if (!m_mapSocketToSessionID.exist(socket))
	{
		m_pLog->Error("OnDataCanReceive called, but socket is no registed.");
		return ResultType::Error;
	}
	auto sessionid = m_mapSocketToSessionID[socket];
	if (!m_mapConnectIDToSession.exist(sessionid))
	{
		m_pLog->Error("OnDataCanReceive called, but session is no registed.");
		return ResultType::Error;
	}
	auto info = m_mapConnectIDToSession[sessionid].get();
	if (!info->TryReceiveLock())
		return ResultType::Invalid;

	m_pLog->Verbos("OnDataCanReceive called, start receiving package.");
	auto res = info->OnDataCanReceive();
	if (res.IsFialed())
	{
		m_pLog->Error(res.GetMessage());
		SendConnectionBreak(sessionid);
	}

	auto pReadList = res.MoveResultObject();
	m_pLog->Verbos(Helper::Format("OnDataCanReceive done. received [%d] packages.", pReadList.size()));
	while (!pReadList.empty())
	{
		auto pack = std::move(pReadList.front());
		pReadList.pop();
		// Check the package is not a response package.
		// If is normal package, and status is response, and it saved in control center. add a task to call the callback function.
		if (pack->type() == DataPackage_PackageType::DataPackage_PackageType_NormalPackage && pack->status() == DataPackage_StatusType::DataPackage_StatusType_Response)
		{
			auto e = static_pointer_cast<IEvent>(m_iC->GetTempSharedPtr(Helper::ntos(pack->id())));
			shared_ptr<SendPackageEvent> event = nullptr;
			if (e != nullptr)
				event = dynamic_pointer_cast<SendPackageEvent>(e);
			if (event != nullptr)
			{
				auto shared_pack = shared_ptr<DataPackage>(move(pack));
				CThreadPool::EnqueTask([event, shared_pack](SMARTER p) {
					event->CallCallbackFunc(shared_pack.get());
					return (SMARTER) nullptr;
				});
				continue;
			}
		}
		m_pWaitProcessList[pack->priority()].push_move(std::move(pack));
	}
	m_oProcessThreadSync.notify_all();
	return res.GetResult();
}

ResultType Sloong::CNetworkHub::OnCanWriteData(SOCKET socket)
{
	if (!m_mapSocketToSessionID.exist(socket))
	{
		m_pLog->Error("OnCanWriteData called, but socket is no registed.");
		return ResultType::Error;
	}
	auto sessionid = m_mapSocketToSessionID[socket];
	if (!m_mapConnectIDToSession.exist(sessionid))
	{
		m_pLog->Error("OnCanWriteData called, but session is no registed.");
		return ResultType::Error;
	}
	auto info = m_mapConnectIDToSession[sessionid].get();
	if (!info->TrySendLock())
		return ResultType::Invalid;

	auto res = info->OnDataCanSend();
	if (res == ResultType::Error)
		SendConnectionBreak(sessionid);
	return res;
}

ResultType Sloong::CNetworkHub::OnOtherEventHappened(SOCKET sessionid)
{
	SendConnectionBreak(sessionid);
	return ResultType::Succeed;
}
