/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-26 10:48:37
 * @Description: file content
 */
/*
 * @Author: WCB
 * @Date: 2019-10-15 10:41:43
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-15 20:26:58
 * @Description: file content
 */
#include "base_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include <dlfcn.h>

IControl* Sloong::IData::m_iC = nullptr;
unique_ptr<CSloongBaseService> Sloong::CSloongBaseService::Instance = nullptr;

void CSloongBaseService::sloong_terminator()
{
    cout << "terminator function is called, system will shutdown. " << endl;
    CUtility::write_call_stack();
    exit(0);
}

void CSloongBaseService::on_sigint(int signal)
{
    cout << "SIGSEGV signal happened, system will shutdown. signal:" << signal << endl;
    CUtility::write_call_stack();
    exit(0);
}

void CSloongBaseService::on_SIGINT_Event(int signal)
{
    Instance->Exit();
}

void CSloongBaseService::InitSystemEventHandler()
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
}

CResult CSloongBaseService::Initialize(unique_ptr<RunTimeData>& config)
{
    m_pServerConfig = move(config);
    m_oExitResult = CResult::Succeed();

    InitSystemEventHandler();
    #ifdef DEBUG
    m_pServerConfig->TemplateConfig.set_debugmode(true);
    m_pServerConfig->TemplateConfig.set_loglevel(Protocol::LogLevel::All);
    #endif
    
    m_pLog->Initialize(m_pServerConfig->TemplateConfig.logpath(), "", (LOGOPT) (LOGOPT::WriteToSTDOut|LOGOPT::WriteToFile), LOGLEVEL(m_pServerConfig->TemplateConfig.loglevel()), LOGTYPE::DAY);

    auto res = InitModule();
    if( res.IsFialed() ) return res;


    res = m_pModuleInitializationFunc(&m_pServerConfig->TemplateConfig);
    if( res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }

    res = m_pControl->Initialize(m_pServerConfig->TemplateConfig.mqthreadquantity());
    if( res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }
    
    m_pControl->Add(DATA_ITEM::ServerConfiguation, &m_pServerConfig->TemplateConfig);
    m_pControl->Add(Logger, m_pLog.get());
    m_pControl->RegisterEvent(ProgramExit);
    m_pControl->RegisterEvent(ProgramStart);
    m_pControl->RegisterEvent(ProgramRestart);
    m_pControl->RegisterEventHandler(ProgramRestart,std::bind(&CSloongBaseService::Restart, this, std::placeholders::_1));
    IData::Initialize(m_pControl.get());
    res = m_pNetwork->Initialize(m_pControl.get());
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }
    
    m_pNetwork->RegisterMessageProcesser(m_pHandler);
    m_pNetwork->RegisterAccpetConnectProcesser(m_pAccept);
    res = m_pModuleInitializedFunc(m_pControl.get());
    if( res.IsFialed() )
        m_pLog->Fatal(res.Message());

    return RegisteNode();
}

CResult CSloongBaseService::InitModule()
{
    // Load the module library
    string libFullPath = m_pServerConfig->TemplateConfig.modulepath()+m_pServerConfig->TemplateConfig.modulename();

    m_pLog->Verbos(CUniversal::Format("Start init module[%s] and load module functions",libFullPath));
    m_pModule = dlopen(libFullPath.c_str(),RTLD_LAZY);
    if(m_pModule == nullptr)
    {
        string errMsg = CUniversal::Format("Load library [%s] error[%s].",libFullPath.c_str(),dlerror());
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    char *errmsg;
    m_pCreateEvnFunc = (CreateProcessEnvironmentFunction)dlsym(m_pModule, "CreateProcessEnvironment");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function CreateProcessEnvironment error[%s].",errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pHandler = (MessagePackageProcesserFunction)dlsym(m_pModule, "MessagePackageProcesser");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function MessagePackageProcesser error[%s].",errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pAccept = (NewConnectAcceptProcesserFunction)dlsym(m_pModule, "NewConnectAcceptProcesser");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function NewConnectAcceptProcesser error[%s]. Use default function.",errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pModuleInitializationFunc = (ModuleInitializationFunction)dlsym(m_pModule, "ModuleInitialization");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function ModuleInitialize error[%s]. maybe module no need initiliaze.",errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pModuleInitializedFunc = (ModuleInitializedFunction)dlsym(m_pModule, "ModuleInitialized");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function ModuleInitialize error[%s]. maybe module no need initiliaze.",errmsg);
        m_pLog->Warn(errMsg);
    }
    m_pLog->Verbos("load module functions done.");
}

CResult CSloongBaseService::RegisteNode()
{
     Json::Value jReq;
    jReq["Function"] = "RegisteNode";
    jReq["TemplateID"] = m_pServerConfig->TemplateID;

    auto req = make_shared<DataPackage>();
	req->set_function(Functions::ProcessMessage);
	req->set_sender( m_pServerConfig->NodeUUID );
	req->set_content(jReq.toStyledString());

	CDataTransPackage dataPackage;
	dataPackage.Initialize( m_pServerConfig->ManagerConnect );
	dataPackage.RequestPackage(req);
	ResultType result = dataPackage.SendPackage();
    return CResult::Succeed();
}



void CSloongBaseService::Restart(SmartEvent event)
{
    // Restart service. use the Exit Sync object, notify the wait thread and return the ExitResult.
    // in main function, check the result, if is Retry, do the init loop.
    m_oExitResult = CResult(ResultType::Retry);
    m_oExitSync.notify_all();
}


CResult CSloongBaseService::Run(){
    m_pLog->Info("Application begin running.");
    m_pControl->SendMessage(EVENT_TYPE::ProgramStart);
    m_oExitSync.wait();
    return m_oExitResult;
}
void CSloongBaseService::Exit(){
    m_pLog->Info("Application will exit.");
    m_pControl->SendMessage(EVENT_TYPE::ProgramExit);
    m_pControl->Exit();
    m_oExitSync.notify_one();
    dlclose(m_pModule);
}

bool CSloongBaseService::ConnectToControl(string controlAddress){
    m_pSocket = make_shared<EasyConnect>();
    m_pSocket->Initialize(controlAddress,nullptr);
    return m_pSocket->Connect();
}
