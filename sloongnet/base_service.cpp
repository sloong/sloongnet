/*
 * @Author: WCB
 * @Date: 2019-10-15 10:41:43
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:17:47
 * @Description: Main instance for sloongnet application.
 */

#include "base_service.h"
#include "utility.h"
#include "SendPackageEvent.hpp"
#include "snowflake.h"

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

CResult CSloongBaseService::InitlializeForWorker(RuntimeDataPackage *data)
{
    m_pManagerConnect = make_unique<EasyConnect>();
    m_pManagerConnect->Initialize(data->manageraddress(), data->managerport(), nullptr);
    if (!m_pManagerConnect->Connect())
    {
        return CResult::Make_Error("Connect to control fialed.");
    }
    cout << "Connect to control succeed. Start registe and get configuation." << endl;

    int64_t uuid;
    while (true)
    {
        CDataTransPackage dataPackage(m_pManagerConnect.get());
        auto req = dataPackage.GetDataPackage();
        req->set_type(DataPackage_PackageType::DataPackage_PackageType_RequestPackage);
        req->set_function(Manager::Functions::RegisteWorker);
        req->set_sender(uuid);
        dataPackage.RequestPackage();
        ResultType result = dataPackage.SendPackage();
        if (result != ResultType::Succeed)
            return CResult::Make_Error("Send get config request error.");
        result = dataPackage.RecvPackage(true);
        if (result != ResultType::Succeed)
            return CResult::Make_Error("Receive get config result error.");
        auto response = dataPackage.GetDataPackage();
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
            break;
        }
        else
        {
            return CResult::Make_Error(Helper::Format("Control return an unexpected result [%s]. Message [%s].", ResultType_Name(response->result()).c_str(), response->content().c_str()));
        }
    };
    cout << "Get configuation succeed." << endl;
    return CResult::Succeed();
}

CResult CSloongBaseService::InitlializeForManager(RuntimeDataPackage *data)
{
    data->set_nodeuuid(0);
    auto config = data->mutable_templateconfig();
    config->set_listenport(data->managerport());
    config->set_modulepath("./modules/");
    config->set_modulename("libmanager.so");
    return CResult::Succeed();
}

void CSloongBaseService::InitSystemEventHandler()
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

CResult CSloongBaseService::Initialize(bool ManagerMode, string address, int port)
{
    m_oServerConfig.set_manageraddress(address);
    m_oServerConfig.set_managerport(port);
    m_oExitResult = CResult::Succeed();

    InitSystemEventHandler();

    CResult res = CResult::Succeed();
    if (ManagerMode)
    {
        res = InitlializeForManager(&m_oServerConfig);
    }
    else
    {
        res = InitlializeForWorker(&m_oServerConfig);
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

    CDataTransPackage::InitializeLog(m_pLog.get());
    res = InitModule();
    if (res.IsFialed())
        return res;

    res = m_pModuleInitializationFunc(pConfig);
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }

    res = m_pControl->Initialize(pConfig->mqthreadquantity());
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }

    m_pControl->Add(DATA_ITEM::ServerConfiguation, m_oServerConfig.mutable_templateconfig());
    m_pControl->Add(Logger, m_pLog.get());
    m_pControl->Add(DATA_ITEM::RuntimeData, &m_oServerConfig);
    if (pConfig->moduleconfig().length() > 0)
    {
        Json::Reader reader;
        reader.parse(pConfig->moduleconfig(), m_oModuleConfig);
        m_pControl->Add(DATA_ITEM::ModuleConfiguation, &m_oModuleConfig);
    }
    else
    {
        m_pControl->Add(DATA_ITEM::ModuleConfiguation, nullptr);
    }

    m_pControl->RegisterEvent(ProgramStop);
    m_pControl->RegisterEvent(ProgramStart);
    m_pControl->RegisterEvent(ProgramRestart);
    m_pControl->RegisterEventHandler(ProgramRestart, std::bind(&CSloongBaseService::OnRestart, this, std::placeholders::_1));
    m_pControl->RegisterEventHandler(ProgramStop, std::bind(&CSloongBaseService::OnStop, this, std::placeholders::_1));
    IData::Initialize(m_pControl.get());
    res = m_pNetwork->Initialize(m_pControl.get());
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }

    m_pNetwork->RegisterEnvCreateProcesser(m_pModuleCreateProcessEvnFunc);
    m_pNetwork->RegisterProcesser(m_pModuleRequestHandler, m_pModuleResponseHandler, m_pModuleEventHandler);
    m_pNetwork->RegisterAccpetConnectProcesser(m_pModuleAcceptHandler);
    auto sock = INVALID_SOCKET;
    if (m_pManagerConnect)
        sock = m_pManagerConnect->GetSocketID();
    res = m_pModuleInitializedFunc(sock, m_pControl.get());
    if (res.IsFialed())
        m_pLog->Fatal(res.Message());

    if (!ManagerMode)
    {
        res = RegisteNode();
        if (res.IsFialed())
        {
            m_pLog->Fatal(res.Message());
            return res;
        }
        auto event = make_unique<Events::CNetworkEvent>(EVENT_TYPE::RegisteConnection);
        event->SetSocketID(sock);
        m_pControl->SendMessage(std::move(event));
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

    CDataTransPackage dataPackage(m_pManagerConnect.get());
    auto req = dataPackage.GetDataPackage();
    req->set_type(DataPackage_PackageType::DataPackage_PackageType_RequestPackage);
    req->set_function(Manager::Functions::RegisteNode);
    req->set_sender(m_oServerConfig.nodeuuid());
    req->set_content(ConvertObjToStr(&req_pack));
    dataPackage.RequestPackage();

    ResultType result = dataPackage.SendPackage();
    if (result != ResultType::Succeed)
        return CResult::Make_Error("Send RegisteNode request error.");
    result = dataPackage.RecvPackage(true);
    if (result != ResultType::Succeed)
        return CResult::Make_Error(" Get RegisteNode result error.");
    auto response = dataPackage.GetDataPackage();
    if (!response)
        return CResult::Make_Error("Parse the get config response data error.");
    if (response->result() != ResultType::Succeed)
        return CResult::Make_Error(Helper::Format("RegisteNode request return error. message: %s", response->content().c_str()));
    return CResult::Succeed();
}

CResult CSloongBaseService::Run()
{
    m_pLog->Info("Application begin running.");
    m_pControl->SendMessage(EVENT_TYPE::ProgramStart);
    m_emStatus = RUN_STATUS::Running;
    auto prev_time = chrono::system_clock::now();
    auto prev_status = 	make_shared<CPU_OCCUPY>();
    CUtility::RecordCPUStatus(prev_status.get());
    int mem_total, mem_free;
    // Report server load status each one minutes.
    while(!m_oExitSync.wait_for(REPORT_LOAD_STATUS_INTERVAL))
    {
        Manager::ReportLoadStatusRequest req;
        auto load = CUtility::CalculateCPULoad(prev_status.get());        
        CUtility::GetMemory(mem_total,mem_free);
        req.set_cpuload(load);
        req.set_memroyused(mem_total/mem_free);
        auto event = make_unique<Events::CSendPackageEvent>();
	    event->SetRequest(m_pManagerConnect->GetSocketID(), m_oServerConfig.nodeuuid(), snowflake::Instance->nextid(),  Core::PRIORITY_LEVEL::LOW_LEVEL , (int)Functions::ReportLoadStatus, ConvertObjToStr(&req));
	    m_pControl->SendMessage(std::move(event));

        CUtility::RecordCPUStatus(prev_status.get());
    }
    return m_oExitResult;
}

void CSloongBaseService::Stop()
{
    if(m_emStatus == RUN_STATUS::Exit) return;
    m_pLog->Info("Application will exit.");
    m_emStatus = RUN_STATUS::Exit;
    m_pControl->SendMessage(EVENT_TYPE::ProgramStop);
}

void CSloongBaseService::OnRestart(IEvent *event)
{
    // Restart service. use the Exit Sync object, notify the wait thread and return the ExitResult.
    // in main function, check the result, if is Retry, do the init loop.
    m_oExitResult = CResult(ResultType::Retry);
    m_oExitSync.notify_all();
}

void CSloongBaseService::OnStop(IEvent *event)
{
    m_oExitSync.notify_all();
    m_pControl->Exit();
    m_pLog->End();
    CThreadPool::Exit();
    if (m_pModule)
        dlclose(m_pModule);
}
