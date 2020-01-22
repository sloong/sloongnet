/*
 * @Author: WCB
 * @Date: 2019-10-15 10:41:43
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 16:42:27
 * @Description: file content
 */

#include "base_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"

IControl* Sloong::IData::m_iC = nullptr;
unique_ptr<CSloongBaseService> Sloong::CSloongBaseService::g_pAppService = nullptr;


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
    g_pAppService->Exit();
}

CResult CSloongBaseService::Initialize(unique_ptr<GLOBAL_CONFIG>& config)
{
	m_pServerConfig = move(config);
    m_oExitResult = CResult::Succeed;

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

    auto res = m_pControl->Initialize(m_pServerConfig->mqthreadquantity());
    if( res.IsSucceed() ){
        m_pControl->Add(DATA_ITEM::ServerConfiguation, m_pServerConfig.get());
        m_pControl->Add(Logger, m_pLog.get());
        m_pControl->RegisterEvent(ProgramExit);
        m_pControl->RegisterEvent(ProgramStart);
        IData::Initialize(m_pControl.get());
        res = m_pNetwork->Initialize(m_pControl.get());
		if (res.IsSucceed())
		{
            m_pNetwork->RegisterMessageProcesser(std::bind(&CSloongBaseService::MessagePackageProcesser, this, std::placeholders::_1));
			AfterInit();
			return CResult::Succeed;
		}
    }
    
    m_pLog->Fatal(res.Message());
    return res;
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
}
bool CSloongBaseService::ConnectToControl(string controlAddress){
    m_pSocket = make_shared<EasyConnect>();
    m_pSocket->Initialize(controlAddress,nullptr);
    return m_pSocket->Connect();
}

void CSloongBaseService::RegistFunctionHandler(Functions func, FuncHandler handler)
{
    m_oFunctionHandles[func] = handler;
}

void CSloongBaseService::MessagePackageProcesser(SmartPackage pack)
{
    auto msgPack = pack->GetRecvPackage();
    auto sender = msgPack->sender();
    auto func = msgPack->function();
    m_pLog->Verbos(CUniversal::Format("Porcess [%s] request: sender[%d]", Functions_Name(func), sender));
    if (m_oFunctionHandles.exist(func))
    {
        if (!m_oFunctionHandles[func](func, sender, pack))
        {
            return;
        }
    }
    else
    {
        switch (func)
        {
        case Functions::RestartService:
        {
            // Restart service. use the Exit Sync object, notify the wait thread and return the ExitResult.
            // in main function, check the result, if is Retry, do the init loop.
            m_oExitResult = CResult(ResultType::Retry);
            m_oExitSync.notify_all();
            return;
        }break;
        default:
            m_pLog->Verbos(CUniversal::Format("No handler for [%s] request: sender[%d]", Functions_Name(func), sender));
            pack->ResponsePackage(ResultType::Error, "No hanlder to process request.");
        }
    }
	
	auto response_event = make_shared<Events::CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(pack->GetSocketID());
	response_event->SetDataPackage(pack);
	m_pControl->SendMessage(response_event);
}