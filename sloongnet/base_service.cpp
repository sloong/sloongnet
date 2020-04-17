/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 17:40:54
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

CResult CSloongBaseService::Initialize(unique_ptr<GLOBAL_CONFIG>& config)
{
    m_pServerConfig = move(config);
    m_oExitResult = CResult::Succeed();

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
    
    #ifdef DEBUG
    m_pServerConfig->set_debugmode(true);
    m_pServerConfig->set_loglevel(Protocol::LogLevel::All);
    #endif
    
    m_pLog->Initialize(m_pServerConfig->logpath(), "", (LOGOPT) (LOGOPT::WriteToSTDOut|LOGOPT::WriteToFile), LOGLEVEL(m_pServerConfig->loglevel()), LOGTYPE::DAY);

    // Load the module library
    string libFullPath = m_pServerConfig->modulepath()+m_pServerConfig->modulename();


    m_pModule = dlopen(libFullPath.c_str(),RTLD_LAZY);
    if(m_pModule == nullptr)
    {
        string errMsg = CUniversal::Format("Load library [%s] error[%s].",libFullPath.c_str(),dlerror());
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    char *errmsg;
    m_pHandler = (MessagePackageProcesserFunction)dlsym(m_pModule, "MessagePackageProcesser");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function MessagePackageProcesser error[%s].",errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pAccept = (NewConnectAcceptProcesserFunction)dlsym(m_pModule, "NewConnectAcceptProcesser");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function NewConnectAcceptProcesser error[%s]. Use default function.",errmsg);
        m_pLog->Info(errMsg);
    }
    auto beforeInit = (ModuleInitializationFunction)dlsym(m_pModule, "ModuleInitialization");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function ModuleInitialize error[%s]. maybe module no need initiliaze.",errmsg);
        m_pLog->Info(errMsg);
    }
    auto afterInit = (ModuleInitializedFunction)dlsym(m_pModule, "ModuleInitialized");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function ModuleInitialize error[%s]. maybe module no need initiliaze.",errmsg);
        m_pLog->Info(errMsg);
    }

    auto res = beforeInit(m_pServerConfig.get());
    if( res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }

    res = m_pControl->Initialize(m_pServerConfig->mqthreadquantity());
    if( res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }
    
    m_pControl->Add(DATA_ITEM::ServerConfiguation, m_pServerConfig.get());
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
    res = afterInit(m_pControl.get());
    if( res.IsFialed() )
        m_pLog->Fatal(res.Message());
    return res;
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
