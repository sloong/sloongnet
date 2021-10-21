/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2019-11-05 08:59:19
 * @LastEditTime: 2021-10-19 17:04:39
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
#include "ConnectSession.h"
#include "EpollEx.h"
#include "IData.h"
#include "snowflake.h"

#include "events/ConnectionBreak.hpp"
#include "events/EnableTimeoutCheck.hpp"
#include "events/GetConnectionInfo.hpp"
#include "events/MonitorSendStatus.hpp"
#include "events/RegisterConnection.hpp"
#include "events/SendPackage.hpp"

using namespace Sloong::Events;

Sloong::CNetworkHub::CNetworkHub()
{
    m_pEpoll = make_unique<CEpollEx>();
    m_oWaitProcessList = vector<queue_safety<UniquePackage>>(PRIORITY_LEVEL_ARRAYSIZE);
}

Sloong::CNetworkHub::~CNetworkHub()
{
    Exit(nullptr);
    if (m_pCTX)
        SSLHelper::G_FreeSSL(m_pCTX);
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
    m_iC->RegisterEventHandler(EVENT_TYPE::SendPackage,
                               std::bind(&CNetworkHub::SendPackageEventHandler, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(EVENT_TYPE::ConnectionBreaked,
                               std::bind(&CNetworkHub::OnConnectionBreakedEventHandler, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(EVENT_TYPE::MonitorSendStatus,
                               std::bind(&CNetworkHub::MonitorSendStatusEventHandler, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(EVENT_TYPE::RegisterConnection,
                               std::bind(&CNetworkHub::RegisterConnectionEventHandler, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(EVENT_TYPE::GetConnectionInfo,
                               std::bind(&CNetworkHub::OnGetConnectionInfoEventHandler, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(EVENT_TYPE::EnableTimeoutCheck,
                               std::bind(&CNetworkHub::OnEnableTimeoutCheckEventHandler, this, std::placeholders::_1));

    if (m_pRequestFunc == nullptr)
    {
        m_pLog->critical("Process function is null.");
        m_iC->SendMessage(EVENT_TYPE::ProgramStop);
    }

    if (m_pConfig->processthreadquantity() < 1)
        m_pLog->critical("the config value for process work quantity is invalid, please check.");

    ThreadPool::AddWorkThread(std::bind(&CNetworkHub::MessageProcessWorkLoop, this),
                              m_pConfig->processthreadquantity());

    m_pEpoll->Run();

    return CResult::Succeed;
}

void Sloong::CNetworkHub::Run(SharedEvent event)
{
    m_pLog->info("NetworkHub is running.");
    m_emStatus = RUN_STATUS::Running;
}

void Sloong::CNetworkHub::Exit(SharedEvent event)
{
    m_emStatus = RUN_STATUS::Exit;
    m_oCheckTimeoutThreadSync.notify_all();
    m_oProcessThreadSync.notify_all();
    m_mapConnectIDToSession.clear();
    for (auto &i : m_oWaitProcessList)
    {
        i.clear();
    }
    m_oWaitProcessList.clear();
    m_pEpoll->Exit();
}

void Sloong::CNetworkHub::SendPackageEventHandler(SharedEvent event)
{
    auto send_evt = DYNAMIC_TRANS<SendPackageEvent *>(event.get());
    auto id = send_evt->GetConnectionHashCode();
    if (!m_mapConnectIDToSession.exist(id))
    {
        m_pLog->error(
            format("SendPackageEventHandler function called, but the session[{}] is no regiestd in NetworkHub.", id));
        return;
    }

    static uint64_t nodeUUID = IData::GetNodeUUID();

    send_evt->SetBaseData(nodeUUID, snowflake::Instance->nextid());

    if (send_evt->HaveCallbackFunc())
        m_iC->AddTempSharedPtr(Helper::ntos(send_evt->GetDataPackage()->id()), event);

    AddMessageToSendList(send_evt->MoveDataPackage());
}

void Sloong::CNetworkHub::AddMessageToSendList(UniquePackage pack)
{
    uint64_t sessionid = pack->sessionid();
    if (!m_mapConnectIDToSession.exist(sessionid))
    {
        m_pLog->error(format("AddMessageToSendList function called, but the session[{}] is no regiestd in NetworkHub.",
                             sessionid));
        return;
    }

    auto session = m_mapConnectIDToSession[sessionid].get();
    auto res = session->SendDataPackage(std::move(pack));
    if (res == ResultType::Retry)
    {
        m_pEpoll->ModifySendMonitorStatus(sessionid, true);
    }
    if (res == ResultType::Error)
    {
        SendConnectionBreak(sessionid);
    }
}

void Sloong::CNetworkHub::OnConnectionBreakedEventHandler(SharedEvent e)
{
    auto event = DYNAMIC_TRANS<ConnectionBreakedEvent *>(e.get());
    auto id = event->GetSessionID();
    if (!m_mapConnectIDToSession.exist(id))
        return;

    auto info = m_mapConnectIDToSession[id].get();

    m_pLog->info(format("Connect is braked:[{}]{}:{}.", info->m_pConnection->GetSocketID(),
                        info->m_pConnection->m_strAddress, info->m_pConnection->m_nPort));

    if (event->GetJustClose())
    {
        m_pLog->info(format("close connect:[{}]{}:{}. will try reconnect when send data in next time.",
                            info->m_pConnection->GetSocketID(), info->m_pConnection->m_strAddress,
                            info->m_pConnection->m_nPort));
        m_mapConnectIDToSession[id]->m_pConnection->Close();
    }
    else
    {
        m_pLog->info(format("close connect:[{}]{}:{}. Unregister this session.", info->m_pConnection->GetSocketID(),
                            info->m_pConnection->m_strAddress, info->m_pConnection->m_nPort));
        unique_lock<shared_mutex> sockLck(m_oSockListMutex);
        m_mapConnectIDToSession.erase(id);
        sockLck.unlock();

        m_pEpoll->UnregisteConnection(id);
    }
}

void Sloong::CNetworkHub::MonitorSendStatusEventHandler(SharedEvent e)
{
    auto event = DYNAMIC_TRANS<MonitorSendStatusEvent *>(e.get());
    auto id = event->GetSessionID();
    if (!m_mapConnectIDToSession.exist(id))
        return;

    auto info = m_mapConnectIDToSession[id].get();
    m_pLog->info(format("MonitorSendStatus:{}:{}.", info->m_pConnection->m_strAddress, info->m_pConnection->m_nPort));
    m_pEpoll->ModifySendMonitorStatus(info->m_pConnection->GetHashCode(), true);
}

void Sloong::CNetworkHub::RegisterConnectionEventHandler(SharedEvent e)
{
    auto event = DYNAMIC_TRANS<RegisterConnectionEvent *>(e.get());
    if (event == nullptr)
    {
        m_pLog->error("RegisterConnectionEventHandler is called, but param type error.");
        return;
    }

    auto connect = make_unique<EasyConnect>();
    auto res = connect->InitializeAsClient(m_pLog, event->GetAddress(), event->GetPort(), m_pCTX);
    if (res.IsFialed())
    {
        m_pLog->warn(res.GetMessage());
    }

    if (event->HaveReconnectCallback())
    {
        connect->SetOnReconnectCallback(event->MoveReconnectCallbackFunc());
    }

    auto info = make_unique<ConnectSession>();
    info->Initialize(m_iC, std::move(connect));
    auto sessionid = info->m_pConnection->GetHashCode();
    m_pLog->info(format("Register connection:[{}][{}][{}:{}].", info->m_pConnection->GetSocketID(), sessionid,
                        info->m_pConnection->m_strAddress, info->m_pConnection->m_nPort));

    unique_lock<shared_mutex> sockLck(m_oSockListMutex);
    m_mapConnectIDToSession[sessionid] = std::move(info);
    sockLck.unlock();
    m_pEpoll->RegisterConnection(m_mapConnectIDToSession[sessionid]->m_pConnection.get());

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
    event->CallCallbackFunc(ConnectionInfo{.Address = pConn->m_strAddress, .Port = pConn->m_nPort});
}

void Sloong::CNetworkHub::OnEnableTimeoutCheckEventHandler(SharedEvent e)
{
    auto event = DYNAMIC_TRANS<EnableTimeoutCheckEvent *>(e.get());
    if (event == nullptr)
        return;

    EnableTimeoutCheck(event->GetTimeoutTime(), event->GetCheckInterval());
}

inline void Sloong::CNetworkHub::SendConnectionBreak(uint64_t sessionid)
{
    if (!m_mapConnectIDToSession.exist(sessionid))
        return;

    auto e = make_shared<ConnectionBreakedEvent>(sessionid);
    // Only close no support reconnect session.
    if (m_mapConnectIDToSession[sessionid]->m_pConnection->SupportReconnect())
        e->SetJustClose(true);

    m_iC->SendMessage(e);
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
    if (m_nConnectTimeoutTime > 0 && m_nCheckTimeoutInterval > 0)
        ThreadPool::AddWorkThread(std::bind(&CNetworkHub::CheckTimeoutWorkLoop, this));
}

void Sloong::CNetworkHub::EnableSSL(const string &certFile, const string &keyFile, const string &passwd)
{
    auto ret = SSLHelper::G_InitializeSSL(&m_pCTX, certFile, keyFile, passwd);
    if (ret != S_OK)
    {
        m_pLog->error("Initialize SSL environment error.");
        m_pLog->error(SSLHelper::G_FormatSSLErrorMsg(ret));
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
    int tout = m_nConnectTimeoutTime;
    int tinterval = m_nCheckTimeoutInterval * 1000;

    map_ex<uint64_t, bool> closedList;

    m_pLog->debug("Check connect timeout thread is running.");
    while (m_emStatus != RUN_STATUS::Exit)
    {
        m_pLog->trace("Check connect timeout start.");
        closedList.clear();
    RecheckTimeout:
        shared_lock<shared_mutex> rlock(m_oSockListMutex);
        for (auto it = m_mapConnectIDToSession.begin(); it != m_mapConnectIDToSession.end(); ++it)
        {
            if (closedList.exist(it->first))
                continue;
            if (it->second != NULL && time(NULL) - it->second->m_ActiveTime > tout)
            {
                m_pLog->info(format("[Timeout]:[Close connect:{}]", it->second->m_pConnection->m_strAddress));
                closedList[it->first] = true;
                SendConnectionBreak(it->first);
                goto RecheckTimeout;
            }
        }
        rlock.unlock();
        m_pLog->trace(format("Check connect timeout done. wait [{}] ms.", tinterval));
        m_oCheckTimeoutThreadSync.wait_for(tinterval);
    }
    m_pLog->info("check timeout connect thread is exit ");
}

/// 消息处理工作线程函数
// 按照优先级，逐个处理待处理队列。每个队列处理完毕时，重新根据优先级处理，尽量保证高优先级的处理
// 为了避免影响接收时的效率，将队列操作放松到最低。以每次一定数量的处理来逐级加锁。
void Sloong::CNetworkHub::MessageProcessWorkLoop()
{
    auto pid = this_thread::get_id();
    string spid = Helper::ntos(pid);

    m_pLog->info("Network hub work thread is started. PID:" + spid);

    while (m_emStatus == RUN_STATUS::Created)
    {
        this_thread::sleep_for(std::chrono::microseconds(10));
    }

    // Before run loop, it's need create the environment, so check the run status before that.
    if (m_emStatus == RUN_STATUS::Exit)
    {
        m_pLog->critical(format("Network hub work thread[{}] is exit before into work loop. ", spid));
        return;
    }

    void *pEnv;
    m_pLog->debug("Call module create process environment function.");
    auto res = m_pCreateEnvFunc(&pEnv);
    if (res.IsFialed())
    {
        m_pLog->critical(res.GetMessage());
        m_iC->SendMessage(EVENT_TYPE::ProgramStop);
        return;
    }
    if (pEnv == nullptr)
    {
        m_pLog->warn("Create called succeed, but the evnironment value is null.");
    }
    else
    {
        m_pLog->debug("Create called succeed.");
    }

    while (m_emStatus != RUN_STATUS::Exit)
    {
        try
        {
        MessagePorcessListRetry:
            for (int i = 0; i < PRIORITY_LEVEL_ARRAYSIZE; i++)
            {
                if (m_oWaitProcessList[i].empty())
                    continue;

                PackageResult result(ResultType::Ignore);
                UniquePackage package = m_oWaitProcessList[i].pop(nullptr);

                while (package != nullptr)
                {
                    // In here, the result no the result for this request.
                    // it just for is need add the pack obj to send list.
                    result.SetResult(ResultType::Ignore);

                    package->record_point("StartPorcess");
                    switch (package->type())
                    {
                    case DataPackage_PackageType::DataPackage_PackageType_ControlEvent: {
                        PrintPackage(m_pLog, package.get(), "Event package <<< ");
                        switch (package->function())
                        {
                        case ControlEvent::Restart: {
                            m_pLog->info("Received restart control event. application will restart.");
                            m_iC->SendMessage(EVENT_TYPE::ProgramRestart);
                        }
                        break;
                        case ControlEvent::Stop: {
                            m_pLog->info("Received stop control event. application will stop.");
                            m_iC->SendMessage(EVENT_TYPE::ProgramStop);
                        }
                        break;
                        case ControlEvent::SetLogLevel: {
                            m_pLog->info("Received set log level control event.");
                            auto l = stoi(package->content());
                            if (!LogLevel_IsValid(l))
                            {
                                m_pLog->error(format("The content[{}] is not valid for the specified log level.", l));
                            }
                            auto level = spdlog::level::level_enum(l);
                            m_pLog->set_level(level);
                            m_pLog->info(format("Set log level to [{}:{}]", l, LogLevel_Name(l)));
                        }
                        break;
                        default:
                            m_pEventFunc(package.get());
                            break;
                        }
                    }
                    break;
                    case DataPackage_PackageType::DataPackage_PackageType_Request:
                    case DataPackage_PackageType::DataPackage_PackageType_Response: {
                        PrintPackage(m_pLog, package.get(), "Process package <<< ");
                        if (package->type() == DataPackage_PackageType::DataPackage_PackageType_Request)
                            result = m_pRequestFunc(pEnv, package.get());
                        else
                            result = m_pResponseFunc(pEnv, package.get());

                        if (result.HaveResultObject())
                        {
                            auto response = result.MoveResultObject();
                            PrintPackage(m_pLog, package.get(), "Response package <<< ");
                            AddMessageToSendList(move(response));
                        }
                        else if (result.GetResult() != ResultType::Ignore)
                        {
                            auto response = Package::MakeResponse(package.get());
                            response->set_result(result.GetResult());
                            response->set_content(result.GetMessage());
                            PrintPackage(m_pLog, package.get(), "Response package <<< ");
                            AddMessageToSendList(move(response));
                        }
                        else
                        {
                            // Ignore the package response
                        }
                    }
                    break;
                    default:
                        m_pLog->warn("Data package check type error. cannot process.");
                    }
                    package = nullptr;
                }
                goto MessagePorcessListRetry;
            }
            m_oProcessThreadSync.wait_for(100);
        }
        catch (const std::exception &e)
        {
            m_pLog->error(format("MessageProcessWorkLoop catch exception: {}", e.what()));
        }
        catch (...)
        {
            m_pLog->error("Unknown exception happened in MessageProcessWorkLoop");
        }
    }
    m_pLog->info(format("Network hub work thread[{}] is exit ", spid));
}

/// 有新链接到达。
/// 接收链接之后，需要客户端首先发送客户端校验信息。只有校验成功之后才会进行SSL处理
ResultType Sloong::CNetworkHub::OnNewAccept(uint64_t sock)
{
    SOCKET conn_sock = (SOCKET)sock;
    m_pLog->debug("Accept function is called.");

    // start client check when acdept
    if (m_nClientCheckKeyLength > 0)
    {
        char *pCheckBuf = new char[m_nClientCheckKeyLength + 1];
        memset(pCheckBuf, 0, m_nClientCheckKeyLength + 1);
        // In Check function, client need send the check key in 3 second.
        // 这里仍然使用Universal提供的ReceEx。这里不需要进行SSL接收
        int nLen = Helper::RecvEx(conn_sock, pCheckBuf, m_nClientCheckKeyLength, m_nClientCheckTime);
        if (nLen != m_nClientCheckKeyLength || 0 != strcmp(pCheckBuf, m_strClientCheckKey.c_str()))
        {
            m_pLog->warn(format("Check Key Error.Length[{}]:[{}].Server[{}]:[{}]Client", m_nClientCheckKeyLength, nLen,
                                m_strClientCheckKey, pCheckBuf));
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
    conn->InitializeAsServer(m_pLog, conn_sock, m_pCTX);

    auto info = make_unique<ConnectSession>();
    info->Initialize(m_iC, std::move(conn));
    auto id = info->m_pConnection->GetHashCode();
    m_pLog->info(format("Accept client:[{}][{}][{}:{}].", id, info->m_pConnection->GetSocketID(),
                        info->m_pConnection->m_strAddress, info->m_pConnection->m_nPort));
    unique_lock<shared_mutex> sockLck(m_oSockListMutex);
    m_mapConnectIDToSession[id] = std::move(info);
    sockLck.unlock();
    m_pEpoll->RegisterConnection(m_mapConnectIDToSession[id]->m_pConnection.get());
    return ResultType::Succeed;
}

ResultType Sloong::CNetworkHub::OnDataCanReceive(uint64_t sessionid)
{
    if (!m_mapConnectIDToSession.exist(sessionid))
    {
        m_pLog->error("OnDataCanReceive called, but session is no registed.");
        return ResultType::Error;
    }
    auto info = m_mapConnectIDToSession[sessionid].get();
    if (!info->TryReceiveLock())
        return ResultType::Invalid;

    m_pLog->trace("OnDataCanReceive called, start receiving package.");
    auto res = info->OnDataCanReceive();
    if (res.IsFialed())
    {
        m_pLog->error(res.GetMessage());
        SendConnectionBreak(sessionid);
    }

    auto pReadList = res.MoveResultObject();
    m_pLog->trace(format("OnDataCanReceive done. received [{}] packages.", pReadList.size()));
    while (!pReadList.empty())
    {
        auto pack = std::move(pReadList.front());
        pReadList.pop();
        // Check the package is not a response package.
        // If is normal package, and status is response, and it saved in control center. add a task to call the callback
        // function.
        if (pack->type() == DataPackage_PackageType::DataPackage_PackageType_Response)
        {
            auto e = static_pointer_cast<IEvent>(m_iC->GetTempSharedPtr(Helper::ntos(pack->id())));
            shared_ptr<SendPackageEvent> event = nullptr;
            if (e != nullptr)
                event = dynamic_pointer_cast<SendPackageEvent>(e);
            if (event != nullptr)
            {
                auto shared_pack = shared_ptr<Package>(move(pack));
                TaskPool::Run([event, shared_pack]() {
                    event->CallCallbackFunc(shared_pack.get());
                    return nullptr;
                });
                continue;
            }
        }
        if (m_oWaitProcessList.size() <= pack->priority())
        {
            m_pLog->warn(format("Package priority is invalid, ignoring the priority value."));
            m_oWaitProcessList[0].push(std::move(pack));
        }
        else
        {
            m_oWaitProcessList[pack->priority()].push(std::move(pack));
        }
    }
    m_oProcessThreadSync.notify_all();
    return res.GetResult();
}

ResultType Sloong::CNetworkHub::OnCanWriteData(uint64_t sessionid)
{
    if (!m_mapConnectIDToSession.exist(sessionid))
    {
        m_pLog->error("OnCanWriteData called, but session is no registed.");
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

ResultType Sloong::CNetworkHub::OnOtherEventHappened(uint64_t id)
{
    SendConnectionBreak(id);
    return ResultType::Succeed;
}
