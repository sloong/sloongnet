#include "base_service.h"

#include "ControlHub.h"
#include "NetworkHub.h"
#include "DataTransPackage.h"

#include "utility.h"

using namespace ProtobufMessage;


IControl* Sloong::IData::m_iC = nullptr;
unique_ptr<CSloongBaseService> Sloong::CSloongBaseService::g_pAppService = nullptr;

void CSloongBaseService::PrientHelp()
{
    cout << "param: address:port" << endl;
}

 void CSloongBaseService::sloong_terminator()
{
    cout << "Unkonw error happened, system will shutdown. " << endl;
    CUtility::write_call_stack();
    exit(0);
}


void CSloongBaseService::on_sigint(int signal)
{
    cout << "Unhandle signal happened, system will shutdown. signal:" << signal << endl;
    CUtility::write_call_stack();
    exit(0);
}

void CSloongBaseService::on_SIGINT_Event(int signal)
{
    g_pAppService->Exit();
}


string CSloongBaseService::GetConfigFromControl(MessageFunction func)
{
    auto get_config_request_buf = make_shared<MessagePackage>();
    get_config_request_buf->set_function(func);
    get_config_request_buf->set_sender(m_emModuleType);
    get_config_request_buf->set_receiver(ModuleType::ControlCenter);
    get_config_request_buf->set_type(MessagePackage_Types::MessagePackage_Types_Request);
    
    
    CDataTransPackage dataPackage;
    dataPackage.Initialize(m_pSocket);
    dataPackage.RequestPackage(get_config_request_buf);
    NetworkResult result = dataPackage.SendPackage();
    if(result != NetworkResult::Succeed)
        throw string("Send get config request error.");
    result = dataPackage.RecvPackage(0);
    if(result != NetworkResult::Succeed)
        throw string("Receive get config result error.");
    auto get_config_response_buf = dataPackage.GetRecvPackage();
    if(!get_config_response_buf)
        throw string("Parse the get config response data error.");
    
    return get_config_response_buf->extenddata();
}

CResult CSloongBaseService::Initialize(int argc, char** args)
{
    if(m_emModuleType == ModuleType::Undefine)
        return CResult(false,"Module type is no define."); 
        
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
    
        if (argc != 2)
        {
            PrientHelp();
            return CResult(false);
        }

        if(m_emModuleType != ModuleType::ControlCenter )
        {
            if( !ConnectToControl(args[1]))
                return CResult("Connect to control fialed.");

            cout << "Connect to control succeed." << endl;
            cout << "Start get configuation." << endl;
            auto serverConfig = GetConfigFromControl(MessageFunction::GetGeneralConfig);
            if(!m_oServerConfig.ParseFromString(serverConfig))
                return CResult("Parse the config struct error.");
            
            cout << "Get configuation succeed." << endl;
            
            cout << "Start get special configuation." << endl;
            m_szConfigData = GetConfigFromControl(MessageFunction::GetSpecialConfig);
            cout << "Get special configuation succeed." << endl;
        }
        
        //m_pLog->Initialize(serv_config.logpath(), "", serv_config.debugmode(), LOGLEVEL(serv_config.loglevel()), LOGTYPE::DAY);
        m_pLog->Initialize(m_oServerConfig.logpath(), "", true, LOGLEVEL::All, LOGTYPE::DAY);
        
        auto res = m_pControl->Initialize(m_oServerConfig.mqthreadquantity());
        if( res.IsSucceed() ){
            m_pControl->Add(DATA_ITEM::GlobalConfiguation, &m_oServerConfig);
            m_pControl->Add(Logger, m_pLog.get());
            m_pControl->RegisterEvent(ProgramExit);
            m_pControl->RegisterEvent(ProgramStart);
            IData::Initialize(m_pControl.get());
            res = m_pNetwork->Initialize(m_pControl.get());
            if( res .IsSucceed())
                return CResult::Succeed;
        }
        
        m_pLog->Fatal(res.Message());
        return res;
}


void CSloongBaseService::Run(){
    m_pLog->Info("Application begin running.");
    m_pControl->SendMessage(EVENT_TYPE::ProgramStart);
    m_oSync.wait();
}
void CSloongBaseService::Exit(){
    m_pLog->Info("Application will exit.");
    m_pControl->SendMessage(EVENT_TYPE::ProgramExit);
    m_pControl->Exit();
    m_oSync.notify_one();
}
bool CSloongBaseService::ConnectToControl(string controlAddress){
    m_pSocket = make_shared<EasyConnect>();
    m_pSocket->Initialize(controlAddress,nullptr);
    m_pSocket->Connect();
    /*string clientCheckKey = "c2xvb25nYzJ4dmIyNW5PRFJtT0dWa01ERTBNalZsTkRBd01XUmlZV1UxT0RZM05tRmlaamd3TmpsbmJtOXZiSE1nbm9vbHM";
    m_pSocket->Send(clientCheckKey);*/
}