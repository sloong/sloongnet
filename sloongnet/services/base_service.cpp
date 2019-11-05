/*
 * @Author: WCB
 * @Date: 2019-10-15 10:41:43
 * @LastEditors: WCB
 * @LastEditTime: 2019-10-15 10:41:43
 * @Description: file content
 */
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


bool ReadAllText(string path, string& out)
{
    FILE* fp = NULL;

	fp = fopen(path.c_str(), "r");
    if( fp )
    {
        fseek(fp, 0, SEEK_END);
        int len = ftell(fp);
        rewind(fp);    //指针复位
        char* buf = new char[len];
        memset(buf,0,len);
        fread(buf, 1, len, fp);
        fclose(fp);
        out = string(buf,len);
        SAFE_DELETE_ARR(buf);
        return true;
    }
    else
    {
        return false;
    }
}


string CSloongBaseService::GetConfigFromControl()
{
    auto get_config_request_buf = make_shared<MessagePackage>();
    get_config_request_buf->set_function(MessageFunction::GetServerConfig);
    get_config_request_buf->set_receiver(Protocol::ModuleType::Control);
    get_config_request_buf->set_type(Protocol::MsgTypes::Request);
    string uuid;
    if( ReadAllText("uuid.dat",uuid))
        get_config_request_buf->set_sender(uuid.c_str());
    
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
    
    // 这里参数有两种形式。
    // 1 只有端口。 此时为控制中心模式
    // 2 地址：端口。此时为非控制中心模式
    if (argc != 2)
    {
        PrientHelp();
        return CResult(false);
    }

    vector<string> addr;
    CUniversal::splitString(args[1],addr,":");

    if(addr.size()>1)
    {
        if( !ConnectToControl(args[1]))
            return CResult(false,"Connect to control fialed.");
        else
            cout << "Connect to control succeed." << endl;

        do
        {
            cout << "Start get configuation." << endl;
            auto serverConfig = GetConfigFromControl();
            if(!m_oServerConfig.ParseFromString(serverConfig))
                return CResult(false,"Parse the config struct error.");
        } while (m_oServerConfig.type() != ModuleType::Unconfigured);

        cout << "Get configuation succeed." << endl;
    }

    #ifdef DEBUG
    m_oServerConfig.set_debugmode(true);
    m_oServerConfig.set_loglevel(LOGLEVEL::All);
    #endif
    
    m_pLog->Initialize(m_oServerConfig.logpath(), "", m_oServerConfig.debugmode(), LOGLEVEL(m_oServerConfig.loglevel()), LOGTYPE::DAY);
        
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
}