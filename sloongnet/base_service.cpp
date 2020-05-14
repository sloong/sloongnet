/*
 * @Author: WCB
 * @Date: 2019-10-15 10:41:43
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 14:10:18
 * @Description: Main instance for sloongnet application.
 */

#include "base_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include <dlfcn.h>

#include "protocol/manager.pb.h"
using namespace Manager;

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



TResult<shared_ptr<DataPackage>> CSloongBaseService::RegisteToControl(SmartConnect con, string uuid)
{
	auto req = make_shared<DataPackage>();
    req->set_type(DataPackage_PackageType::DataPackage_PackageType_RequestPackage);
	req->set_function(Manager::Functions::RegisteWorker);
	req->set_sender(uuid);

	CDataTransPackage dataPackage(con);
	dataPackage.RequestPackage(req);
	ResultType result = dataPackage.SendPackage();
	if (result != ResultType::Succeed)
		return TResult<shared_ptr<DataPackage>>::Make_Error( "Send get config request error.");
	result = dataPackage.RecvPackage(0);
	if (result != ResultType::Succeed)
		return TResult<shared_ptr<DataPackage>>::Make_Error("Receive get config result error.");
	auto get_config_response_buf = dataPackage.GetRecvPackage();
	if (!get_config_response_buf)
		return TResult<shared_ptr<DataPackage>>::Make_Error("Parse the get config response data error.");

	return TResult<shared_ptr<DataPackage>>::Make_OK(get_config_response_buf);
}

CResult CSloongBaseService::InitlializeForWorker(RuntimeDataPackage* data)
{
	m_pManagerConnect = make_shared<EasyConnect>();
	m_pManagerConnect->Initialize(data->manageraddress(), data->managerport(), nullptr);
	if (!m_pManagerConnect->Connect())
	{
        return CResult::Make_Error("Connect to control fialed.");
	}
	cout << "Connect to control succeed. Start registe and get configuation." << endl;
	
	string uuid;
	while(true)
	{
		auto res = RegisteToControl(m_pManagerConnect,uuid);
		if (res.IsFialed()) return res;	

		auto response = res.ResultObject();

		if( response->result() == Core::ResultType::Retry ){
            if( uuid.length() == 0 ){
                uuid = response->content();
                cout << "Control assigen uuid ."<< uuid << endl;
            }else{
                sleep(1);
                cout << "Control return retry package. wait 1s and retry." << endl;
            }
            continue;
		}else if(response->result() == Core::ResultType::Succeed){
            auto res_pack = ConvertStrToObj<RegisteWorkerResponse>(response->content());
	        string serverConfig = res_pack->configuation();
            if (serverConfig.size() == 0)
                return CResult::Make_Error("Control no return config infomation.");
            if (!data->mutable_templateconfig()->ParseFromString(serverConfig))
                return CResult::Make_Error("Parse the config struct error.");

            data->set_templateid(res_pack->templateid());
            data->set_nodeuuid(uuid);
            break;
		}else{
            return CResult::Make_Error(CUniversal::Format("Control return an unexpected result [%s]. Message [%s].",Core::ResultType_Name(response->result()),response->content()));
		}
	};
	cout << "Get configuation succeed." << endl;
    return CResult::Succeed();
}

CResult CSloongBaseService::InitlializeForManager(RuntimeDataPackage* data)
{
    auto config = data->mutable_templateconfig();
	config->set_listenport( data->managerport() );
	config->set_modulepath("./modules/");
	config->set_modulename("libmanager.so");
    return CResult::Succeed();
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

CResult CSloongBaseService::Initialize(bool ManagerMode, string address, int port)
{
    m_oServerConfig.set_manageraddress(address);
    m_oServerConfig.set_managerport(port);
    m_oExitResult = CResult::Succeed();

    InitSystemEventHandler();
    #ifdef DEBUG
    m_oServerConfig.mutable_templateconfig()->set_debugmode(true);
    m_oServerConfig.mutable_templateconfig()->set_loglevel(Core::LogLevel::All);
    #endif
    
    CResult res = CResult::Succeed();
    if( ManagerMode )
	{
		res = InitlializeForManager(&m_oServerConfig);
	}
	else
	{
		res = InitlializeForWorker(&m_oServerConfig);
	}
    if (res.IsFialed()) return res;
    
    m_pLog->Initialize(m_oServerConfig.templateconfig().logpath(), "", (LOGOPT) (LOGOPT::WriteToSTDOut|LOGOPT::WriteToFile), LOGLEVEL(m_oServerConfig.templateconfig().loglevel()), LOGTYPE::DAY);
    CDataTransPackage::InitializeLog(m_pLog.get());
    res = InitModule();
    if( res.IsFialed() ) return res;

    res = m_pModuleInitializationFunc(m_oServerConfig.mutable_templateconfig());
    if( res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }

    res = m_pControl->Initialize(m_oServerConfig.templateconfig().mqthreadquantity());
    if( res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }
    
    m_pControl->Add(DATA_ITEM::ServerConfiguation, m_oServerConfig.mutable_templateconfig());
    m_pControl->Add(Logger, m_pLog.get());
    m_pControl->Add(DATA_ITEM::RuntimeData, &m_oServerConfig );

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
    
    m_pNetwork->RegisterEnvCreateProcesser(m_pModuleCreateProcessEvnFunc);
    m_pNetwork->RegisterProcesser(m_pModuleRequestHandler,m_pModuleRequestHandler,m_pModuleEventHandler);
    m_pNetwork->RegisterAccpetConnectProcesser(m_pModuleAcceptHandler);
    auto sock = INVALID_SOCKET;
    if( m_pManagerConnect ) sock= m_pManagerConnect->GetSocketID();
    res = m_pModuleInitializedFunc( sock, m_pControl.get());
    if( res.IsFialed() )
        m_pLog->Fatal(res.Message());

    if(!ManagerMode )
    {
        res = RegisteNode();
        if (res.IsFialed())
        {
            m_pLog->Fatal(res.Message());
            return res;
        }
        m_pNetwork->RegisteConnection(sock);
    }

    return CResult::Succeed();
}

CResult CSloongBaseService::InitModule()
{
    // Load the module library
    string libFullPath = m_oServerConfig.templateconfig().modulepath()+m_oServerConfig.templateconfig().modulename();

    m_pLog->Verbos(CUniversal::Format("Start init module[%s] and load module functions",libFullPath));
    m_pModule = dlopen(libFullPath.c_str(),RTLD_LAZY);
    if(m_pModule == nullptr)
    {
        string errMsg = CUniversal::Format("Load library [%s] error[%s].",libFullPath.c_str(),dlerror());
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    char *errmsg;
    m_pModuleCreateProcessEvnFunc = (CreateProcessEnvironmentFunction)dlsym(m_pModule, "CreateProcessEnvironment");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function CreateProcessEnvironment error[%s].",errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleRequestHandler = (RequestPackageProcessFunction)dlsym(m_pModule, "RequestPackageProcesser");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function RequestPackageProcesser error[%s].",errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleResponseHandler = (ResponsePackageProcessFunction)dlsym(m_pModule, "ResponsePackageProcesser");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function ResponsePackageProcesser error[%s].",errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleEventHandler = (EventPackageProcessFunction)dlsym(m_pModule, "EventPackageProcesser");
    if ((errmsg = dlerror()) != NULL)  {
        string errMsg = CUniversal::Format("Load function EventPackageProcessFunction error[%s].",errmsg);
        m_pLog->Error(errMsg);
        return CResult::Make_Error(errMsg);
    }
    m_pModuleAcceptHandler = (NewConnectAcceptProcessFunction)dlsym(m_pModule, "NewConnectAcceptProcesser");
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
    return CResult::Succeed();
}

CResult CSloongBaseService::RegisteNode()
{
    RegisteNodeRequest req_pack;
    req_pack.set_templateid(m_oServerConfig.templateid());
    
    auto req = make_shared<DataPackage>();
    req->set_type(DataPackage_PackageType::DataPackage_PackageType_RequestPackage);
	req->set_function(Manager::Functions::RegisteNode);
	req->set_sender( m_oServerConfig.nodeuuid() );
	req->set_content(ConvertObjToStr(&req_pack));

	CDataTransPackage dataPackage(m_pManagerConnect);
	dataPackage.RequestPackage(req);

	ResultType result = dataPackage.SendPackage();
    if (result != ResultType::Succeed)
		return CResult::Make_Error( "Send RegisteNode request error.");
	result = dataPackage.RecvPackage(0);
	if (result != ResultType::Succeed)
		return CResult::Make_Error(" Get RegisteNode result error.");
	auto response = dataPackage.GetRecvPackage();
	if (!response)
		return CResult::Make_Error("Parse the get config response data error.");
    if(response->result() != ResultType::Succeed)
        return CResult::Make_Error(CUniversal::Format("RegisteNode request return error. message: %s", response->content()));
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
    if( m_pModule)
        dlclose(m_pModule);
}

