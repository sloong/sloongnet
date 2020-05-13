/* File Name: server.c */
#include "gateway_service.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "IData.h"
#include "NormalEvent.hpp"
#include "SendMessageEvent.hpp"
#include "transpond.h"

#include "protocol/manager.pb.h"
using namespace Manager;
using namespace Sloong;
using namespace Sloong::Events;


unique_ptr<SloongNetGateway> Sloong::SloongNetGateway::Instance = nullptr;

extern "C" CResult MessagePackageProcesser(void* env,CDataTransPackage* pack)
{
	auto pTranspond = TYPE_TRANS<GatewayTranspond*>(env);
	if( pTranspond)
		return pTranspond->PackageProcesser(pack);
	else
		return CResult::Make_Error("Environment convert error. cannot process message.");
}

extern "C" CResult EventPackageProcesser(CDataTransPackage* pack)
{
	SloongNetGateway::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
}

extern "C" CResult NewConnectAcceptProcesser(CSockInfo* info)
{
	return CResult::Succeed();
}
	
extern "C" CResult ModuleInitialization(GLOBAL_CONFIG* confiog){
	SloongNetGateway::Instance = make_unique<SloongNetGateway>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(SOCKET sock,IControl* iC){
	return SloongNetGateway::Instance->Initialized(sock,iC);
}

extern "C" CResult CreateProcessEnvironment(void** out_env)
{
	return SloongNetGateway::Instance->CreateProcessEnvironmentHandler(out_env);
}


CResult SloongNetGateway::Initialized(SOCKET sock,IControl* iC)
{
	m_pControl = iC;
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	Json::Reader reader;
	if ( m_pConfig->moduleconfig().length() > 0 && reader.parse(m_pConfig->moduleconfig(), m_oExConfig))
	{
		shared_ptr<CNormalEvent> event = make_shared<CNormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableTimeoutCheck);
		event->SetMessage(CUniversal::Format("{\"TimeoutTime\":\"%d\", \"CheckInterval\":%d}",m_oExConfig["TimeoutTime"].asInt(),m_oExConfig["TimeoutCheckInterval"].asInt()));
		m_pControl->SendMessage(event);

		event->SetEvent(EVENT_TYPE::EnableClientCheck);
		event->SetMessage(CUniversal::Format("{\"ClientCheckKey\":\"%s\", \"ClientCheckTime\":%d}",m_oExConfig["ClientCheckKey"].asString(),m_oExConfig["ClientCheckKey"].asInt()));
		m_pControl->SendMessage(event);
	}
	m_pLog = IData::GetLog();
	m_nManagerConnection = sock;
	m_pControl->RegisterEventHandler(ProgramStart,std::bind(&SloongNetGateway::OnStart, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetGateway::OnSocketClose, this, std::placeholders::_1));
	return CResult::Succeed();
}


void SloongNetGateway::QueryProcessList()
{
	auto references = CUniversal::split(m_pConfig->modulereference(), ';');
	if( references.size() == 0 )
		return;

	Manager::QueryNodeRequest req;
	for (auto item : references)
		req.add_templateid(atoi(item.c_str()));

	auto event = make_shared<CSendMessageEvent>();
	event->SetRequest( m_nManagerConnection,  m_nSerialNumber, 1, (int)Functions::QueryNode, ConvertObjToStr(&req));
	m_nSerialNumber++;
	m_pControl->SendMessage(event);

}

inline CResult SloongNetGateway::CreateProcessEnvironmentHandler(void** out_env)
{
	auto item = make_shared<GatewayTranspond>();
	auto res = item->Initialize(m_pControl);
	if (res.IsFialed())
		return res;
	m_listTranspond.push_back(item);
	(*out_env) = item.get();
	return CResult::Succeed();
}


void SloongNetGateway::OnStart(SmartEvent evt)
{	
	QueryProcessList();
}


void Sloong::SloongNetGateway::OnSocketClose(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
}


void Sloong::SloongNetGateway::EventPackageProcesser(CDataTransPackage* trans_pack)
{
	auto event = Events_MIN;
	auto data_pack = trans_pack->GetRecvPackage();
	if(!Manager::Events_Parse(data_pack->content(),&event))
	{
		m_pLog->Error(CUniversal::Format("Receive event but parse error. content:[%s]",data_pack->content()));
		return;
	}

	switch (event)
	{
	case Manager::Events::ReferenceModuleOnline:{
		m_pLog->Info("Receive ReferenceModuleOnline event");
		}break;
	case Manager::Events::ReferenceModuleOffline:{
		m_pLog->Info("Receive ReferenceModuleOffline event");
		}break;
	default:{
		}break;
	}
}


