/*
 * @Author: WCB
 * @Date: 2019-10-15 10:41:43
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:17:47
 * @Description: Main instance for sloongnet application.
 */

#include "base_service.h"
#include "utility.h"
#include "snowflake.h"

#include "events/SendPackage.hpp"
#include "events/SendPackageToManager.hpp"
#include "events/RegisteConnection.hpp"
using namespace Sloong::Events;

#include "protocol/manager.pb.h"
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

CResult CSloongBaseService::InitlializeForWorker(RuntimeDataPackage *data, int forceTempID, EasyConnect *con)
{
    cout << "Connect to control succeed. Start registe and get configuation." << endl;

    int64_t uuid = 0;
    auto result = CResult::Make_Error("Cancelled by User.");
    while ( m_emStatus != RUN_STATUS::Exit )
    {
        auto req = Package::GetRequestPackage();
        req->set_function(Manager::Functions::RegisteWorker);
        req->set_sender(uuid);
        if (forceTempID > 0)
        {
            RegisteWorkerRequest sub_req;
            sub_req.set_forcetargettemplateid(forceTempID);
            req->set_content(ConvertObjToStr(&sub_req));
        }
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
            auto res_pack = ConvertStrToObj<RegisteWorkerResponse>(response->content());
            string serverConfig = res_pack->configuation();
            if (serverConfig.size() == 0)
                return CResult::Make_Error("Control no return config infomation.");
            if (!data->mutable_templateconfig()->ParseFromString(serverConfig))
                return CResult::Make_Error("Parse the config struct error.");

            data->set_templateid(res_pack->templateid());
            data->set_nodeuuid(uuid);
            result = CResult::Succeed();
            break;
        }
        else
        {
            return CResult::Make_Error(Helper::Format("Control return an unexpected result [%s]. Message [%s].", ResultType_Name(response->result()).c_str(), response->content().c_str()));
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
    config->set_modulepath("./modules/");
    config->set_modulename("libmanager.so");
    return CResult::Succeed();
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

CResult CSloongBaseService::Initialize(bool ManagerMode, string address, int port, int forceTempID)
{
    m_oServerConfig.set_manageraddress(address);
    m_oServerConfig.set_managerport(port);
    m_oExitResult = CResult::Succeed();

    InitSystem();

    CResult res = CResult::Succeed();
    UniqueConnection pManagerConnect = nullptr;

    if (ManagerMode)
    {
        res = InitlializeForManager(&m_oServerConfig);
    }
    else
    {
        pManagerConnect = make_unique<EasyConnect>();
        pManagerConnect->InitializeAsClient(m_oServerConfig.manageraddress(), m_oServerConfig.managerport(), nullptr);
        if (!pManagerConnect->Connect())
        {
            return CResult::Make_Error("Connect to control fialed.");
        }
        res = InitlializeForWorker(&m_oServerConfig, forceTempID, pManagerConnect.get());
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

    if(m_pPrepareInitializeFunc )
    {
        res = m_pPrepareInitializeFunc(pConfig);
        if (res.IsFialed())
        {
            m_pLog->Fatal(res.GetMessage());
            return res;
        }
    }
    
    res = m_iC->Initialize(pConfig->mqthreadquantity(), m_pLog.get() );
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
        Json::Reader reader;
        reader.parse(pConfig->moduleconfig(), m_oModuleConfig);
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
        auto event = make_shared<Events::RegisteConnectionEvent>(pManagerConnect->m_strAddress, pManagerConnect->m_nPort);
        event->SetCallbackFunc([s=&m_ManagerSession](IEvent* e,int64_t sessionid) {
            *s = sessionid;
        });
        m_iC->CallMessage(event);
    }

    res = m_pModuleInitializedFunc();
    if (res.IsFialed())
        m_pLog->Fatal(res.GetMessage());
    m_pLog->Debug("Module initialized succeed.");

    if (!ManagerMode)
    {
        auto res = RegisteNode();
        if (res.IsFialed())
        {
            m_pLog->Fatal(res.GetMessage());
            return res;
        }
    }

    return CResult::Succeed();
}

CResult CSloongBaseService::InitModule()
{
    // Load the module library
    string libFullPath = m_oServerConfig.templateconfig().modulepath() + m_oServerConfig.templateconfig().modulename();

    m_pLog->Debug(Helper::Format("Start init module[%s] and load module functions", libFullPath.c_str()));
    m_pModule = dlopen(libFullPath.c_str(), RTLD_LAZY);
    if (m_pModule == nullptr)
    {
        string errMsg = Helper::Format("Load library [%s] error[%s].", libFullPath.c_str(), dlerror());
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    char *errmsg;
    m_pModuleCreateProcessEvnFunc = (CreateProcessEnvironmentFunction)dlsym(m_pModule, "CreateProcessEnvironment");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = Helper::Format("Load function CreateProcessEnvironment error[%s].", errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleRequestHandler = (RequestPackageProcessFunction)dlsym(m_pModule, "RequestPackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = Helper::Format("Load function RequestPackageProcesser error[%s].", errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleResponseHandler = (ResponsePackageProcessFunction)dlsym(m_pModule, "ResponsePackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = Helper::Format("Load function ResponsePackageProcesser error[%s].", errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleEventHandler = (EventPackageProcessFunction)dlsym(m_pModule, "EventPackageProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = Helper::Format("Load function EventPackageProcessFunction error[%s].", errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleAcceptHandler = (NewConnectAcceptProcessFunction)dlsym(m_pModule, "NewConnectAcceptProcesser");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = Helper::Format("Load function NewConnectAcceptProcesser error[%s]. Use default function.", errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pModuleInitializationFunc = (ModuleInitializationFunction)dlsym(m_pModule, "PrepareInitialize");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = Helper::Format("Load function PrepareInitialize error[%s]. maybe module no need.", errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pModuleInitializationFunc = (ModuleInitializationFunction)dlsym(m_pModule, "ModuleInitialization");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = Helper::Format("Load function ModuleInitialize error[%s]. maybe module no need initiliaze.", errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pModuleInitializedFunc = (ModuleInitializedFunction)dlsym(m_pModule, "ModuleInitialized");
    if ((errmsg = dlerror()) != NULL)
    {
        string errMsg = Helper::Format("Load function ModuleInitialize error[%s]. maybe module no need initiliaze.", errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pLog->Debug("load module functions done.");
    return CResult::Succeed();
}

CResult CSloongBaseService::RegisteNode()
{
    RegisteNodeRequest req_pack;
    req_pack.set_templateid(m_oServerConfig.templateid());

    auto event = make_shared<SendPackageToManagerEvent>(Manager::Functions::RegisteNode, ConvertObjToStr(&req_pack));
    auto response_str = make_shared<string>();
    auto result = make_shared<ResultType>(ResultType::Invalid);
    auto sync = make_shared<EasySync>();
    event->SetCallbackFunc([result, sync, response_str](IEvent *e, DataPackage *p) {
        (*result) = p->result();
        *response_str = p->content();
        sync->notify_one();
    });
    m_iC->CallMessage(event);

    if (!sync->wait_for(5000))
    {
        return CResult::Make_Error("RegisteNode timeout");
    }
     if (*result != ResultType::Succeed)
    {
        return CResult::Make_Error(*response_str);
    }
    return CResult::Succeed();
}

CResult CSloongBaseService::Run()
{
    m_pLog->Info("Application begin running.");
    m_iC->SendMessage(EVENT_TYPE::ProgramStart);
    m_emStatus = RUN_STATUS::Running;

    bool enableReport = false;
    if (enableReport)
    {
        auto prev_status = make_shared<CPU_OCCUPY>();
        CUtility::RecordCPUStatus(prev_status.get());
        int mem_total, mem_free;
        // Report server load status each one minutes.
        while (!m_oExitSync.wait_for(REPORT_LOAD_STATUS_INTERVAL))
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
    }
    while(  m_emStatus != RUN_STATUS::Exit)
    {        
        m_oExitSync.wait_for(1000);
    }
    m_pLog->Info("Application main work loop end with result " + ResultType_Name(m_oExitResult.GetResult()) );

    return m_oExitResult;
}

void CSloongBaseService::Stop()
{
    m_pLog->Info("Application will exit.");
    m_iC->SendMessage(EVENT_TYPE::ProgramStop);
    if( m_emStatus == RUN_STATUS::Created )
        m_emStatus = RUN_STATUS::Exit;
}

void CSloongBaseService::OnProgramRestartEventHandler(SharedEvent event)
{
    // Restart service. use the Exit Sync object, notify the wait thread and return the ExitResult.
    // in main function, check the result, if is Retry, do the init loop.
    m_oExitResult = CResult(ResultType::Retry);
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
    req->SetCallbackFunc([event](IEvent *e, DataPackage *p) {
        event->CallCallbackFunc(p);
    });
    req->SetRequest(IData::GetRuntimeData()->nodeuuid(), snowflake::Instance->nextid(), Base::HEIGHT_LEVEL, event->GetFunctionID(), event->GetContent());
    m_iC->SendMessage(req);
}

void CSloongBaseService::OnGetManagerSocketEventHandler(SharedEvent e)
{
}
