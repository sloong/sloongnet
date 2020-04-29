/*
 * @Author: WCB
 * @Date: 2019-10-15 10:41:43
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-29 17:23:06
 * @Description: Main instance for sloongnet application.
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



TResult<shared_ptr<DataPackage>> CSloongBaseService::RegisteToControl(SmartConnect con, string uuid)
{
	auto req = make_shared<DataPackage>();
	req->set_function(Functions::ProcessMessage);
	req->set_sender(uuid);
	req->set_content("{\"Function\":\"RegisteWorker\"}");

	CDataTransPackage dataPackage;
	dataPackage.Initialize(con);
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

CResult CSloongBaseService::InitlializeForWorker(RunTimeData* data)
{
	data->ManagerConnect = make_shared<EasyConnect>();
	data->ManagerConnect->Initialize(data->ManagerAddress, data->ManagerPort, nullptr);
	if (!data->ManagerConnect->Connect())
	{
        return CResult::Make_Error("Connect to control fialed.");
	}
	cout << "Connect to control succeed. Start registe and get configuation." << endl;
	
	string uuid;
	string serverConfig;
	while(true)
	{
		auto res = RegisteToControl(data->ManagerConnect,uuid);
		if (res.IsFialed()) return res;	

		auto response = res.ResultObject();

		if( response->result() == Protocol::ResultType::Retry ){
            if( uuid.length() == 0 ){
                uuid = response->content();
                cout << "Control assigen uuid ."<< uuid << endl;
			    continue;
            }else{
                sleep(1);
                cout << "Control return retry package. wait 1s and retry." << endl;
			    continue;
            }
		}else if(response->result() == Protocol::ResultType::Succeed){
			serverConfig = response->content();
		}else{
            return CResult::Make_Error(CUniversal::Format("Control return an unexpected result [%s]. Message [%s].",Protocol::ResultType_Name(response->result()),response->content()));
		}

		if (serverConfig.size() == 0)
		{
			cout << "Control no return config infomation. wait 1s and retry." << endl;
			sleep(1);
			continue;
		}

		data->NodeUUID = uuid;
		Json::Reader reader;
		Json::Value jConfig;
		if( !reader.parse(serverConfig, jConfig) || !jConfig["TemplateID"].isInt() || !jConfig["Configuation"].isString())
		{
			return CResult::Make_Error(CUniversal::Format("Parse the config error. response data: %s" ,serverConfig));
		}
		
		if (!data->TemplateConfig.ParseFromString(CBase64::Decode(jConfig["Configuation"].asString()) ))
		{
			return CResult::Make_Error("Parse the config struct error. please check.");
		}
		data->TemplateID = jConfig["TemplateID"].asInt();
		break;
	};
	cout << "Get configuation succeed." << endl;
    return CResult::Succeed();
}

CResult CSloongBaseService::InitlializeForManager(RunTimeData* data)
{
	data->TemplateConfig.set_listenport( data->ManagerPort );
	data->TemplateConfig.set_modulepath("./modules/");
	data->TemplateConfig.set_modulename("libmanager.so");
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

CResult CSloongBaseService::Initialize(RunTimeData* config)
{
    m_pServerConfig = config;
    m_oExitResult = CResult::Succeed();

    InitSystemEventHandler();
    #ifdef DEBUG
    m_pServerConfig->TemplateConfig.set_debugmode(true);
    m_pServerConfig->TemplateConfig.set_loglevel(Protocol::LogLevel::All);
    #endif
    
    CResult res = CResult::Succeed();
    if( m_pServerConfig->ManagerMode )
	{
		res = InitlializeForManager(m_pServerConfig);
	}
	else
	{
		res = InitlializeForWorker(m_pServerConfig);
	}
    if (res.IsFialed()) return res;
    
    m_pLog->Initialize(m_pServerConfig->TemplateConfig.logpath(), "", (LOGOPT) (LOGOPT::WriteToSTDOut|LOGOPT::WriteToFile), LOGLEVEL(m_pServerConfig->TemplateConfig.loglevel()), LOGTYPE::DAY);

    res = InitModule();
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
    
    m_pNetwork->RegisterEnvCreateProcesser(m_pCreateEvnFunc);
    m_pNetwork->RegisterMessageProcesser(m_pHandler);
    m_pNetwork->RegisterAccpetConnectProcesser(m_pAccept);
    res = m_pModuleInitializedFunc(m_pControl.get());
    if( res.IsFialed() )
        m_pLog->Fatal(res.Message());

    res = RegisteNode();
    if (res.IsFialed())
    {
        m_pLog->Fatal(res.Message());
        return res;
    }
    m_pNetwork->RegisteConnection(m_pServerConfig->ManagerConnect->GetSocketID());

    return CResult::Succeed();
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
    return CResult::Succeed();
}

CResult CSloongBaseService::RegisteNode()
{
    if( m_pServerConfig->ManagerMode )
        return CResult::Succeed();
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

