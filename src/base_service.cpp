/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-11-12 15:56:50
 * @LastEditTime: 2021-08-26 14:42:19
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/base_service.cpp
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

#include "base_service.h"
#include "utility.h"
#include "snowflake.h"

#include "events/SendPackage.hpp"
#include "events/SendPackageToManager.hpp"
#include "events/RegisterConnection.hpp"
using namespace Sloong::Events;

#include "modules/manager/protocol/manager.pb.h"
using namespace Manager;

IControl *Sloong::IData::m_iC = nullptr;
unique_ptr<CSloongBaseService> Sloong::CSloongBaseService::Instance = nullptr;

void CSloongBaseService::sloong_terminator()
{
    cout << "terminator function is called, system will shutdown. " << endl;
    cout << CUtility::GetCallStack();
    exit(0);
}

void CSloongBaseService::sloong_unexpected()
{
    cout << "unexpected function is called, system will shutdown. " << endl;
    cout << CUtility::GetCallStack();
    exit(0);
}

void CSloongBaseService::on_sigint(int signal)
{
    cout << "SIGSEGV signal happened, system will shutdown. signal:" << signal << endl;
    cout << CUtility::GetCallStack();
    exit(0);
}

void CSloongBaseService::on_SIGINT_Event(int signal)
{
    cout << "SIGINT signal happened. Exit." << endl;
    Instance->Stop();
}

CResult CSloongBaseService::InitlializeForWorker(RuntimeDataPackage *data, RunInfo *info, EasyConnect *con)
{
    cout << "Connect to control succeed. Start registe and get configuation." << endl;

    RegisterWorkerRequest sub_req;
    if (!info->AssignedTargetTemplateID.empty())
    {
        sub_req.set_runmode(RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_AssignTemplate);
        for (auto &item : Helper::split(info->AssignedTargetTemplateID, ','))
        {
            if (item.empty())
                continue;
            int id = 0;
            if (!ConvertStrToInt(item, &id))
                return CResult::Make_Error(format("Convert template id {} to int failed.", item));
            sub_req.add_assigntargettemplateid(id);
        }
    }
    else if (!info->ExcludeTargetType.empty())
    {
        sub_req.set_runmode(RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_ExcludeType);
        for (auto &item : Helper::split(info->ExcludeTargetType, ','))
        {
            if (item.empty())
                continue;
            MODULE_TYPE forceType = MODULE_TYPE::Manager;
            if (!MODULE_TYPE_Parse(item, &forceType))
                return CResult::Make_Error(format("Parse {} to module type error", item));

            sub_req.add_excludetargettype(forceType);
        }
    }
    else if (!info->IncludeTargetType.empty())
    {
        sub_req.set_runmode(RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_IncludeType);
        for (auto &item : Helper::split(info->IncludeTargetType, ','))
        {
            if (item.empty())
                continue;
            MODULE_TYPE forceType = MODULE_TYPE::Manager;
            if (!MODULE_TYPE_Parse(item, &forceType))
                return CResult::Make_Error(format("Parse {} to module type error", item));

            sub_req.add_includetargettype(forceType);
        }
    }

    uint64_t uuid = 0;
    auto result = CResult::Make_Error("Cancelled by User.");
    while (m_emStatus != RUN_STATUS::Exit)
    {
        auto req = Package::GetRequestPackage();
        req->set_function(Manager::Functions::RegisterWorker);
        req->set_sender(uuid);

        req->set_content(ConvertObjToStr(&sub_req));
        if (con->SendPackage(move(req)).IsFialed())
            return CResult::Make_Error("Send get config request error.");
        auto res = con->RecvPackage(true);
        if (res.IsFialed())
            return CResult::Make_Error("Receive get config result error.");
        auto response = res.MoveResultObject();
        if (!response)
            return CResult::Make_Error("Parse the get config response data error.");

        if (response->result() == ResultType::Retry)
        {
            if (uuid == 0)
            {
                uuid = Helper::BytesToInt64(response->content().c_str());
                cout << "Control assigen uuid ." << uuid << endl;
            }
            else
            {
                this_thread::sleep_for(std::chrono::seconds(1));
                //sleep(1);
                cout << "Control return retry package. wait 1s and retry." << endl;
            }
            continue;
        }
        else if (response->result() == ResultType::Succeed)
        {
            auto res_pack = ConvertStrToObj<RegisterWorkerResponse>(response->content());
            string serverConfig = res_pack->configuation();
            if (serverConfig.size() == 0)
                return CResult::Make_Error("Control no return config infomation.");
            if (!data->mutable_templateconfig()->ParseFromString(serverConfig))
                return CResult::Make_Error("Parse the config struct error.");

            data->set_templateid(res_pack->templateid());
            data->set_nodeuuid(uuid);
            result = CResult::Succeed;
            break;
        }
        else
        {
            return CResult::Make_Error(format("Control return an unexpected result {}. Message {}.", ResultType_Name(response->result()), response->content()));
        }
    };
    cout << "Get configuation done." << endl;
    return result;
}

CResult CSloongBaseService::InitlializeForManager(RuntimeDataPackage *data)
{
    data->set_nodeuuid(0);
    data->set_templateid(1);
    auto config = data->mutable_templateconfig();
    config->set_listenport(data->managerport());
    config->set_modulepath("./");
    config->set_modulename("libmanager.so");
    return CResult::Succeed;
}

void CSloongBaseService::InitSystem()
{
    set_terminate(sloong_terminator);
    set_unexpected(sloong_unexpected);
    //SIG_IGN:忽略信号的处理程序
    //SIGPIPE:在reader终止之后写pipe的时候发生
    signal(SIGPIPE, SIG_IGN); // this signal should call the socket check function. and remove the timeout socket.
    //SIGCHLD: 进程Terminate或Stop的时候,SIGPIPE会发送给进程的父进程,缺省情况下该Signal会被忽略
    signal(SIGCHLD, SIG_IGN);
    //SIGINT:由Interrupt Key产生,通常是Ctrl+c或者Delete,发送给所有的ForeGroundGroup进程.
    signal(SIGINT, &on_SIGINT_Event);
    // SIGSEGV:当一个进程执行了一个无效的内存引用，或发生段错误时发送给它的信号
    signal(SIGSEGV, &on_sigint);
}

CResult CSloongBaseService::Initialize(RunInfo info)
{
    m_emStatus = RUN_STATUS::Created;

    m_oServerConfig.set_manageraddress(info.Address);
    m_oServerConfig.set_managerport(info.Port);
    m_oExitResult = CResult::Succeed;

    InitSystem();

    CResult res = CResult::Succeed;
    UniqueConnection pManagerConnect = nullptr;

    if (info.ManagerMode)
    {
        res = InitlializeForManager(&m_oServerConfig);
    }
    else
    {
        pManagerConnect = make_unique<EasyConnect>();
        res = pManagerConnect->InitializeAsClient(nullptr, m_oServerConfig.manageraddress(), m_oServerConfig.managerport(), nullptr);
        if (res.IsFialed())
        {
            return CResult::Make_Error("Connect to control fialed." + res.GetMessage());
        }

        res = InitlializeForWorker(&m_oServerConfig, &info, pManagerConnect.get());
    }
    if (res.IsFialed())
        return res;
    auto pConfig = m_oServerConfig.mutable_templateconfig();
    if (pConfig->logoperation() == 0)
        pConfig->set_logoperation(LOGOPT::WriteToSTDOut);

#ifdef DEBUG
    pConfig->set_loglevel(Core::LogLevel::All);
    pConfig->set_logoperation(pConfig->logoperation() | LOGOPT::WriteToSTDOut);
#endif
    m_pLog->Initialize(pConfig->logpath(), "", LOGOPT(pConfig->logoperation()), LOGLEVEL(pConfig->loglevel()), LOGTYPE::DAY);

    res = InitModule();
    if (res.IsFialed())
        return res;

    if (m_pPrepareInitializeFunc)
    {
        res = m_pPrepareInitializeFunc(pConfig);
        if (res.IsFialed())
        {
            m_pLog->Fatal(res.GetMessage());
            return res;
        }
    }

    res = m_iC->Initialize(pConfig->mqthreadquantity(), m_pLog.get());
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.GetMessage());
        return res;
    }
    m_pLog->Debug("Control center initialization succeed.");

    m_iC->Add(DATA_ITEM::ServerConfiguation, m_oServerConfig.mutable_templateconfig());
    m_iC->Add(DATA_ITEM::Logger, m_pLog.get());
    m_iC->Add(DATA_ITEM::RuntimeData, &m_oServerConfig);
    if (pConfig->moduleconfig().length() > 0)
    {
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        JSONCPP_STRING err;
        if (!reader->parse(pConfig->moduleconfig().c_str(), pConfig->moduleconfig().c_str() + pConfig->moduleconfig().length(), &m_oModuleConfig, &err))
        {
            m_pLog->Fatal("Error parsing module configuration");
            cerr << err << endl;
            return CResult::Make_Error("Error parsing module configuration");
        }
        m_iC->Add(DATA_ITEM::ModuleConfiguation, &m_oModuleConfig);
    }
    else
    {
        m_iC->Add(DATA_ITEM::ModuleConfiguation, nullptr);
    }

    res = m_pModuleInitializationFunc(m_iC.get());
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.GetMessage());
        return res;
    }
    m_pLog->Debug("Module initialization succeed.");

    m_iC->RegisterEventHandler(EVENT_TYPE::ProgramRestart, std::bind(&CSloongBaseService::OnProgramRestartEventHandler, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStop, std::bind(&CSloongBaseService::OnProgramStopEventHandler, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(EVENT_TYPE::SendPackageToManager, std::bind(&CSloongBaseService::OnSendPackageToManagerEventHandler, this, std::placeholders::_1));

    IData::Initialize(m_iC.get());
    m_pNetwork->RegisterEnvCreateProcesser(m_pModuleCreateProcessEvnFunc);
    m_pNetwork->RegisterProcesser(m_pModuleRequestHandler, m_pModuleResponseHandler, m_pModuleEventHandler);
    m_pNetwork->RegisterAccpetConnectProcesser(m_pModuleAcceptHandler);

    res = m_pNetwork->Initialize(m_iC.get());
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.GetMessage());
        return res;
    }
    m_pLog->Debug("Network initialization succeed.");

    if (pManagerConnect)
    {
        auto event = make_shared<Events::RegisterConnectionEvent>(pManagerConnect->m_strAddress, pManagerConnect->m_nPort);
        event->SetCallbackFunc([s = &m_ManagerSession](IEvent *e, uint64_t sessionid)
                               { *s = sessionid; });
        event->EnableReconnectCallback([&](uint64_t, int, int)
                                       {
                                           ReconnectRegisterRequest req_pack;
                                           req_pack.set_templateid(m_oServerConfig.templateid());
                                           req_pack.set_nodeuuid(m_oServerConfig.nodeuuid());

                                           auto event = make_shared<SendPackageToManagerEvent>(Manager::Functions::ReconnectRegister, ConvertObjToStr(&req_pack));
                                           return event->SyncCall(m_iC.get(), 5000);
                                       });
        m_iC->CallMessage(event);
    }

    res = m_pModuleInitializedFunc();
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.GetMessage());
        return res;
    }
    m_pLog->Debug("Module initialized succeed.");

    if (!info.ManagerMode)
    {
        auto res = RegisterNode();
        if (res.IsFialed())
        {
            m_pLog->Fatal(res.GetMessage());
            return res;
        }
    }

    return CResult::Succeed;
}

CResult CSloongBaseService::InitModule()
{
    // Load the module library
    string libFullPath = m_oServerConfig.templateconfig().modulepath() + m_oServerConfig.templateconfig().modulename();

    m_pLog->Debug(format("Start init module {} and load module functions", libFullPath));
    m_pModule = dlopen(libFullPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (m_pModule == nullptr)
    {
        string errMsg = format("Load library {} error {}.", libFullPath, dlerror());
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    char *errmsg;
    m_pModuleCreateProcessEvnFunc = (CreateProcessEnvironmentFunction)dlsym(m_pModule, "CreateProcessEnvironment");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function CreateProcessEnvironment error {}.", errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleRequestHandler = (RequestPackageProcessFunction)dlsym(m_pModule, "RequestPackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function RequestPackageProcesser error {}.", errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleResponseHandler = (ResponsePackageProcessFunction)dlsym(m_pModule, "ResponsePackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function ResponsePackageProcesser error {}.", errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleEventHandler = (EventPackageProcessFunction)dlsym(m_pModule, "EventPackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function EventPackageProcessFunction error {}.", errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleAcceptHandler = (NewConnectAcceptProcessFunction)dlsym(m_pModule, "NewConnectAcceptProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function NewConnectAcceptProcesser error {}. Use default function.", errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pPrepareInitializeFunc = (PrepareInitializeFunction)dlsym(m_pModule, "PrepareInitialize");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function PrepareInitialize error {}. maybe module no need.", errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pModuleInitializationFunc = (ModuleInitializationFunction)dlsym(m_pModule, "ModuleInitialization");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function ModuleInitialize error {}. maybe module no need initiliaze.", errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pModuleInitializedFunc = (ModuleInitializedFunction)dlsym(m_pModule, "ModuleInitialized");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function ModuleInitialize error {}. maybe module no need initiliaze.", errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pLog->Debug("load module functions done.");
    return CResult::Succeed;
}

CResult CSloongBaseService::RegisterNode()
{
    RegisterNodeRequest req_pack;
    req_pack.set_templateid(m_oServerConfig.templateid());

    auto event = make_shared<SendPackageToManagerEvent>(Manager::Functions::RegisterNode, ConvertObjToStr(&req_pack));
    return event->SyncCall(m_iC.get(), 5000);
}

CResult CSloongBaseService::Run()
{
    if (m_emStatus != RUN_STATUS::Created)
    {
        // May create evnironment error, and the application is received programstop event.
        return CResult::Make_Error("Application run function is called, but the status not created.");
    }
    m_pLog->Info("Application begin running.");
    m_iC->SendMessage(EVENT_TYPE::ProgramStart);
    m_emStatus = RUN_STATUS::Running;

    auto prev_status = make_shared<CPU_OCCUPY>();
    CUtility::RecordCPUStatus(prev_status.get());
    int mem_total, mem_free;
    // Report server load status each one minutes.
    while (!m_oExitSync.wait_for(REPORT_LOAD_STATUS_INTERVAL) && m_emStatus != RUN_STATUS::Exit)
    {
        Manager::ReportLoadStatusRequest req;
        auto load = CUtility::CalculateCPULoad(prev_status.get());
        CUtility::GetMemory(mem_total, mem_free);
        req.set_cpuload(load);
        req.set_memroyused(mem_total / mem_free);

        if (m_oServerConfig.templateid() != 1) // Manager module
        {
            auto event = make_shared<Events::SendPackageEvent>(m_ManagerSession);
            event->SetRequest(m_oServerConfig.nodeuuid(), snowflake::Instance->nextid(), Base::PRIORITY_LEVEL::LOW_LEVEL, (int)Functions::ReportLoadStatus, ConvertObjToStr(&req));
            m_iC->SendMessage(event);
        }

        CUtility::RecordCPUStatus(prev_status.get());
    }

    m_pLog->Info("Application main work loop end with result " + ResultType_Name(m_oExitResult.GetResult()));

    return m_oExitResult;
}

void CSloongBaseService::Stop()
{
    m_pLog->Info("Application will exit.");
    m_iC->SendMessage(EVENT_TYPE::ProgramStop);
    if (m_emStatus == RUN_STATUS::Created)
        m_emStatus = RUN_STATUS::Exit;
}

void CSloongBaseService::OnProgramRestartEventHandler(SharedEvent event)
{
    // Restart service. use the Exit Sync object, notify the wait thread and return the ExitResult.
    // in main function, check the result, if is Retry, do the init loop.
    m_oExitResult = CResult(ResultType::Retry);
    m_emStatus = RUN_STATUS::Exit;
    m_oExitSync.notify_all();
}

void CSloongBaseService::OnProgramStopEventHandler(SharedEvent event)
{
    if (m_emStatus == RUN_STATUS::Exit)
        return;
    m_emStatus = RUN_STATUS::Exit;
    m_pLog->Info("Application receive ProgramStopEvent.");
    m_oExitSync.notify_all();
    m_iC->Exit();
    m_pLog->End();
    if (m_pModule)
        dlclose(m_pModule);
}

void CSloongBaseService::OnSendPackageToManagerEventHandler(SharedEvent e)
{
    auto event = dynamic_pointer_cast<SendPackageToManagerEvent>(e);

    auto req = make_shared<SendPackageEvent>(m_ManagerSession);
    req->SetCallbackFunc([event](IEvent *e, Package *p)
                         { event->CallCallbackFunc(p); });
    req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, event->GetFunctionID(), event->GetContent());
    m_iC->SendMessage(req);
}

void CSloongBaseService::OnGetManagerSocketEventHandler(SharedEvent e)
{
}
