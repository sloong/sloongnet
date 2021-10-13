/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-11-12 15:56:50
 * @LastEditTime: 2021-09-27 17:30:03
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

#include "events/RegisterConnection.hpp"
#include "events/SendPackage.hpp"
#include "events/SendPackageToManager.hpp"
#include "utility.h"

using namespace Sloong::Events;

#include "modules/manager/protocol/manager.pb.h"
using namespace Manager;

#include "linux_cpuload.hpp"
#include "linux_memoryload.hpp"

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

U64Result CSloongBaseService::InitlializeForWorker(EasyConnect *con)
{
    m_pLog->info("Connect to control succeed. Start registe and get configuration.");

    RegisterWorkerRequest sub_req;
    if (!m_oNodeRuntimeInfo.AssignedTargetTemplateID.empty())
    {
        sub_req.set_runmode(RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_AssignTemplate);
        for (auto &item : Helper::split(m_oNodeRuntimeInfo.AssignedTargetTemplateID, ','))
        {
            if (item.empty())
                continue;
            int id = 0;
            if (!ConvertStrToInt(item, &id))
                return U64Result::Make_Error(format("Convert template id {} to int failed.", item));
            sub_req.add_assigntargettemplateid(id);
        }
    }
    else if (!m_oNodeRuntimeInfo.ExcludeTargetType.empty())
    {
        sub_req.set_runmode(RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_ExcludeType);
        for (auto &item : Helper::split(m_oNodeRuntimeInfo.ExcludeTargetType, ','))
        {
            if (item.empty())
                continue;
            MODULE_TYPE forceType = MODULE_TYPE::Manager;
            if (!MODULE_TYPE_Parse(item, &forceType))
                return U64Result::Make_Error(format("Parse {} to module type error", item));

            sub_req.add_excludetargettype(forceType);
        }
    }
    else if (!m_oNodeRuntimeInfo.IncludeTargetType.empty())
    {
        sub_req.set_runmode(RegisterWorkerRequest_RunType::RegisterWorkerRequest_RunType_IncludeType);
        for (auto &item : Helper::split(m_oNodeRuntimeInfo.IncludeTargetType, ','))
        {
            if (item.empty())
                continue;
            MODULE_TYPE forceType = MODULE_TYPE::Manager;
            if (!MODULE_TYPE_Parse(item, &forceType))
                return U64Result::Make_Error(format("Parse {} to module type error", item));

            sub_req.add_includetargettype(forceType);
        }
    }

    uint64_t uuid = 0;
    auto result = U64Result::Make_Error("Cancelled by User.");
    while (m_emStatus != RUN_STATUS::Exit)
    {
        auto req = Package::GetRequestPackage();
        req->set_function(Manager::Functions::RegisterWorker);
        req->set_sender(uuid);

        req->set_content(ConvertObjToStr(&sub_req));
        if (con->SendPackage(move(req)).IsFialed())
            return U64Result::Make_Error("Send get config request error.");
        auto res = con->RecvPackage(true);
        if (res.IsFialed())
            return U64Result::Make_Error("Receive get config result error.");
        auto response = res.MoveResultObject();
        if (!response)
            return U64Result::Make_Error("Parse the get config response data error.");

        if (response->result() == ResultType::Retry)
        {
            if (uuid == 0)
            {
                uuid = Helper::BytesToInt64(response->content().c_str());
                m_pLog->info(format("Control assigen uuid [{}].", uuid));
            }
            else
            {
                this_thread::sleep_for(std::chrono::seconds(1));
                // sleep(1);
                m_pLog->debug("Control return retry package. wait 1s and retry.");
            }
            continue;
        }
        else if (response->result() == ResultType::Succeed)
        {
            auto res_pack = ConvertStrToObj<RegisterWorkerResponse>(response->content());
            string serverConfig = res_pack->configuration();
            if (serverConfig.size() == 0)
                return U64Result::Make_Error("Control no return config infomation.");
            if (!m_oNodeRuntimeInfo.TemplateConfig.ParseFromString(serverConfig))
                return U64Result::Make_Error("Parse the config struct error.");

            m_oNodeRuntimeInfo.NodeUUID = uuid;
            m_oNodeRuntimeInfo.TemplateID = res_pack->templateid();
            result = U64Result::Make_OKResult(res_pack->registerid());
            break;
        }
        else
        {
            return U64Result::Make_Error(format("Control return an unexpected result {}. Message {}.",
                                                ResultType_Name(response->result()), response->content()));
        }
    };
    m_pLog->info("Get configuration done.");
    return result;
}

CResult CSloongBaseService::InitlializeForManager()
{
    m_oNodeRuntimeInfo.NodeUUID = 0;
    auto config = &m_oNodeRuntimeInfo.TemplateConfig;
    config->set_listenport(m_oNodeRuntimeInfo.Port);
    config->set_modulepath("./");
    config->set_modulename("libmanager.so");
    return CResult::Succeed;
}

void CSloongBaseService::InitSystem()
{
    set_terminate(sloong_terminator);
    set_unexpected(sloong_unexpected);
    // SIG_IGN:忽略信号的处理程序
    // SIGPIPE:在reader终止之后写pipe的时候发生
    signal(SIGPIPE, SIG_IGN); // this signal should call the socket check
                              // function. and remove the timeout socket.
    // SIGCHLD:
    // 进程Terminate或Stop的时候,SIGPIPE会发送给进程的父进程,缺省情况下该Signal会被忽略
    signal(SIGCHLD, SIG_IGN);
    // SIGINT:由Interrupt
    // Key产生,通常是Ctrl+c或者Delete,发送给所有的ForeGroundGroup进程.
    signal(SIGINT, &on_SIGINT_Event);
    // SIGSEGV:当一个进程执行了一个无效的内存引用，或发生段错误时发送给它的信号
    signal(SIGSEGV, &on_sigint);
}

CResult CSloongBaseService::Initialize(NodeInfo info, spdlog::logger *log)
{
    m_pLog = log;
    m_emStatus = RUN_STATUS::Created;

    m_oNodeRuntimeInfo = move(info);
    m_oExitResult = CResult::Succeed;

    InitSystem();

    CResult res = CResult::Succeed;
    UniqueConnection pManagerConnect = nullptr;
    uint64_t registerid = 0;
    if (info.ManagerMode)
    {
        res = InitlializeForManager();
    }
    else
    {
        pManagerConnect = make_unique<EasyConnect>();
        res =
            pManagerConnect->InitializeAsClient(nullptr, m_oNodeRuntimeInfo.Address, m_oNodeRuntimeInfo.Port, nullptr);
        if (res.IsFialed())
        {
            return CResult::Make_Error("Connect to control fialed." + res.GetMessage());
        }

        auto regres = InitlializeForWorker(pManagerConnect.get());
        if (regres.IsFialed())
            return CResult(regres.GetResult(), regres.GetMessage());
        else
            registerid = regres.GetResultObject();
    }
    if (res.IsFialed())
        return res;
    auto pConfig = &m_oNodeRuntimeInfo.TemplateConfig;
    // if (pConfig->logoperation() == 0)
    // pConfig->set_logoperation(LOGOPT::WriteToSTDOut);

#ifdef DEBUG
    pConfig->set_loglevel(Core::LogLevel::Verbos);
    // pConfig->set_logoperation(pConfig->logoperation() | LOGOPT::WriteToSTDOut);
#endif
    m_pLog->set_level(spdlog::level::level_enum(pConfig->loglevel()));
    // m_pLog->Initialize(pConfig->logpath(), "", LOGOPT(pConfig->logoperation()),
    // LOGLEVEL(pConfig->loglevel()), LOGTYPE::DAY);

    res = InitModule();
    if (res.IsFialed())
        return res;

    if (m_pPrepareInitializeFunc)
    {
        res = m_pPrepareInitializeFunc(pConfig);
        if (res.IsFialed())
        {
            m_pLog->critical(res.GetMessage());
            return res;
        }
    }

    res = m_iC->Initialize(pConfig->mqthreadquantity(), m_pLog);
    if (res.IsFialed())
    {
        m_pLog->critical(res.GetMessage());
        return res;
    }
    m_pLog->debug("Control center initialization succeed.");

    m_iC->Add(DATA_ITEM::ServerConfiguration, &m_oNodeRuntimeInfo.TemplateConfig);
    m_iC->Add(DATA_ITEM::Logger, m_pLog);
    m_iC->Add(DATA_ITEM::NodeUUID, &m_oNodeRuntimeInfo.NodeUUID);
    if (pConfig->moduleconfig().length() > 0)
    {
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        JSONCPP_STRING err;
        if (!reader->parse(pConfig->moduleconfig().c_str(),
                           pConfig->moduleconfig().c_str() + pConfig->moduleconfig().length(), &m_oModuleConfig, &err))
        {
            m_pLog->critical("Error parsing module configuration");
            cerr << err << endl;
            return CResult::Make_Error("Error parsing module configuration");
        }
        m_iC->Add(DATA_ITEM::ModuleConfiguration, &m_oModuleConfig);
    }
    else
    {
        m_iC->Add(DATA_ITEM::ModuleConfiguration, nullptr);
    }

    res = m_pModuleInitializationFunc(m_iC.get());
    if (res.IsFialed())
    {
        m_pLog->critical(res.GetMessage());
        return res;
    }
    m_pLog->debug("Module initialization succeed.");

    m_iC->RegisterEventHandler(EVENT_TYPE::ProgramRestart, std::bind(&CSloongBaseService::OnProgramRestartEventHandler,
                                                                     this, std::placeholders::_1));
    m_iC->RegisterEventHandler(EVENT_TYPE::ProgramStop,
                               std::bind(&CSloongBaseService::OnProgramStopEventHandler, this, std::placeholders::_1));
    m_iC->RegisterEventHandler(
        EVENT_TYPE::SendPackageToManager,
        std::bind(&CSloongBaseService::OnSendPackageToManagerEventHandler, this, std::placeholders::_1));

    IData::Initialize(m_iC.get());
    m_pNetwork->RegisterEnvCreateProcesser(m_pModuleCreateProcessEvnFunc);
    m_pNetwork->RegisterProcesser(m_pModuleRequestHandler, m_pModuleResponseHandler, m_pModuleEventHandler);
    m_pNetwork->RegisterAccpetConnectProcesser(m_pModuleAcceptHandler);

    res = m_pNetwork->Initialize(m_iC.get());
    if (res.IsFialed())
    {
        m_pLog->critical(res.GetMessage());
        return res;
    }
    m_pLog->debug("Network initialization succeed.");

    if (pManagerConnect)
    {
        auto event =
            make_shared<Events::RegisterConnectionEvent>(pManagerConnect->m_strAddress, pManagerConnect->m_nPort);
        event->SetCallbackFunc([s = &m_ManagerSession](IEvent *e, uint64_t sessionid) { *s = sessionid; });
        event->EnableReconnectCallback([&](uint64_t, int, int) {
            ReconnectRegisterRequest req_pack;
            req_pack.set_templateid(m_oNodeRuntimeInfo.TemplateID);
            req_pack.set_nodeuuid(m_oNodeRuntimeInfo.NodeUUID);

            auto event = make_shared<SendPackageToManagerEvent>(Manager::Functions::ReconnectRegister,
                                                                ConvertObjToStr(&req_pack));
            return event->SyncCall(m_iC.get(), 5000);
        });
        m_iC->CallMessage(event);
    }

    res = m_pModuleInitializedFunc();
    if (res.IsFialed())
    {
        m_pLog->critical(res.GetMessage());
        return res;
    }
    m_pLog->debug("Module initialized succeed.");

    if (!info.ManagerMode)
    {
        auto res = RegisterNode(registerid);
        if (res.IsFialed())
        {
            m_pLog->critical(res.GetMessage());
            return res;
        }
    }

    return CResult::Succeed;
}

CResult CSloongBaseService::InitModule()
{
    // Load the module library
    string libFullPath =
        m_oNodeRuntimeInfo.TemplateConfig.modulepath() + m_oNodeRuntimeInfo.TemplateConfig.modulename();

    m_pLog->debug(format("Start init module {} and load module functions", libFullPath));
    m_pModule = dlopen(libFullPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (m_pModule == nullptr)
    {
        string errMsg = format("Load library {} error {}.", libFullPath, dlerror());
        m_pLog->error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    char *errmsg;
    m_pModuleCreateProcessEvnFunc = (CreateProcessEnvironmentFunction)dlsym(m_pModule, "CreateProcessEnvironment");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function CreateProcessEnvironment error {}.", errmsg);
        m_pLog->error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleRequestHandler = (RequestPackageProcessFunction)dlsym(m_pModule, "RequestPackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function RequestPackageProcesser error {}.", errmsg);
        m_pLog->error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleResponseHandler = (ResponsePackageProcessFunction)dlsym(m_pModule, "ResponsePackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function ResponsePackageProcesser error {}.", errmsg);
        m_pLog->error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleEventHandler = (EventPackageProcessFunction)dlsym(m_pModule, "EventPackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function EventPackageProcessFunction error {}.", errmsg);
        m_pLog->error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleAcceptHandler = (NewConnectAcceptProcessFunction)dlsym(m_pModule, "NewConnectAcceptProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function NewConnectAcceptProcesser error {}. "
                               "Use default function.",
                               errmsg);
        m_pLog->warn(errMsg);
    }
    m_pPrepareInitializeFunc = (PrepareInitializeFunction)dlsym(m_pModule, "PrepareInitialize");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function PrepareInitialize error {}. maybe module no need.", errmsg);
        m_pLog->warn(errMsg);
    }
    m_pModuleInitializationFunc = (ModuleInitializationFunction)dlsym(m_pModule, "ModuleInitialization");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function ModuleInitialize error {}. maybe "
                               "module no need initiliaze.",
                               errmsg);
        m_pLog->warn(errMsg);
    }
    m_pModuleInitializedFunc = (ModuleInitializedFunction)dlsym(m_pModule, "ModuleInitialized");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = format("Load function ModuleInitialize error {}. maybe "
                               "module no need initiliaze.",
                               errmsg);
        m_pLog->warn(errMsg);
    }
    m_pLog->debug("load module functions done.");
    return CResult::Succeed;
}

CResult CSloongBaseService::RegisterNode(uint64_t id)
{
    RegisterNodeRequest req_pack;
    req_pack.set_registerid(id);

    auto event = make_shared<SendPackageToManagerEvent>(Manager::Functions::RegisterNode, ConvertObjToStr(&req_pack));
    return event->SyncCall(m_iC.get(), 5000);
}

CResult CSloongBaseService::Run()
{
    if (m_emStatus != RUN_STATUS::Created)
    {
        // May create evnironment error, and the application is received programstop
        // event.
        return CResult::Make_Error("Application run function is called, but the status not created.");
    }
    m_pLog->info("Application begin running.");
    m_iC->SendMessage(EVENT_TYPE::ProgramStart);
    m_emStatus = RUN_STATUS::Running;

    auto CpuLoadInstance = cpuLoad::createInstance();
    unique_ptr<memoryLoad> MemoryLoadInstance = make_unique<memoryLoad>();

    // Report server load status each one minutes.
    while (!m_oExitSync.wait_for(REPORT_LOAD_STATUS_INTERVAL) && m_emStatus != RUN_STATUS::Exit)
    {
        if (!m_oNodeRuntimeInfo.ManagerMode) // Manager module
        {
            Manager::ReportLoadStatusRequest req;
            auto load = CpuLoadInstance->getCurrentCpuUsage();
            auto mem_load = MemoryLoadInstance->getCurrentMemUsageInPercent();
            req.set_cpuload(load);
            req.set_memroyused(mem_load);

            auto event = make_shared<Events::SendPackageEvent>(m_ManagerSession);
            event->SetRequest((int)Functions::ReportLoadStatus, ConvertObjToStr(&req), PRIORITY_LEVEL::Inessential);
            m_iC->SendMessage(event);
        }
    }

    m_pLog->info("Application main work loop end with result " + ResultType_Name(m_oExitResult.GetResult()));

    return m_oExitResult;
}

void CSloongBaseService::Stop()
{
    m_pLog->info("Application will exit.");
    m_iC->SendMessage(EVENT_TYPE::ProgramStop);
    if (m_emStatus == RUN_STATUS::Created)
        m_emStatus = RUN_STATUS::Exit;
}

void CSloongBaseService::OnProgramRestartEventHandler(SharedEvent event)
{
    // Restart service. use the Exit Sync object, notify the wait thread and
    // return the ExitResult. in main function, check the result, if is Retry, do
    // the init loop.
    m_oExitResult = CResult(ResultType::Retry);
    m_emStatus = RUN_STATUS::Exit;
    m_oExitSync.notify_all();
}

void CSloongBaseService::OnProgramStopEventHandler(SharedEvent event)
{
    if (m_emStatus == RUN_STATUS::Exit)
        return;
    m_emStatus = RUN_STATUS::Exit;
    m_pLog->info("Application receive ProgramStopEvent.");
    if (m_pModule)
        dlclose(m_pModule);
    m_oExitSync.notify_all();
    m_iC->Exit();
}

void CSloongBaseService::OnSendPackageToManagerEventHandler(SharedEvent e)
{
    auto event = dynamic_pointer_cast<SendPackageToManagerEvent>(e);

    auto req = make_shared<SendPackageEvent>(m_ManagerSession);
    req->SetCallbackFunc([event](IEvent *e, Package *p) { event->CallCallbackFunc(p); });
    req->SetRequest(event->GetFunctionID(), event->GetContent(), PRIORITY_LEVEL::Immediate);
    m_iC->SendMessage(req);
}

void CSloongBaseService::OnGetManagerSocketEventHandler(SharedEvent e)
{
}
