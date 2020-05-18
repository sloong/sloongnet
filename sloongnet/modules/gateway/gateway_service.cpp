/* File Name: server.c */
#include "gateway_service.h"
#include "utility.h"
#include "IData.h"
#include "SendMessageEventEx.hpp"

#include "protocol/manager.pb.h"
using namespace Manager;

#include "protocol/processer.pb.h"
using namespace Sloong;
using namespace Sloong::Events;

unique_ptr<SloongNetGateway> Sloong::SloongNetGateway::Instance = nullptr;

extern "C" CResult RequestPackageProcesser(void *env, CDataTransPackage *pack)
{
	return SloongNetGateway::Instance->RequestPackageProcesser(env, pack);
}

extern "C" CResult ResponsePackageProcesser(void *env, CDataTransPackage *pack)
{
	return SloongNetGateway::Instance->ResponsePackageProcesser(pack);
}

extern "C" CResult EventPackageProcesser(CDataTransPackage *pack)
{
	SloongNetGateway::Instance->EventPackageProcesser(pack);
	return CResult::Succeed();
}

extern "C" CResult NewConnectAcceptProcesser(CSockInfo *info)
{
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialization(GLOBAL_CONFIG *confiog)
{
	SloongNetGateway::Instance = make_unique<SloongNetGateway>();
	return CResult::Succeed();
}

extern "C" CResult ModuleInitialized(SOCKET sock, IControl *iC)
{
	return SloongNetGateway::Instance->Initialized(sock, iC);
}

extern "C" CResult CreateProcessEnvironment(void **out_env)
{
	return SloongNetGateway::Instance->CreateProcessEnvironmentHandler(out_env);
}

CResult SloongNetGateway::Initialized(SOCKET sock, IControl *iC)
{
	m_pControl = iC;
	IData::Initialize(iC);
	m_pConfig = IData::GetGlobalConfig();
	m_pModuleConfig = IData::GetModuleConfig();
	m_pRuntimeData = IData::GetRuntimeData();
	if (m_pModuleConfig)
	{
		shared_ptr<CNormalEvent> event = make_shared<CNormalEvent>();
		event->SetEvent(EVENT_TYPE::EnableTimeoutCheck);
		event->SetMessage(CUniversal::Format("{\"TimeoutTime\":\"%d\", \"CheckInterval\":%d}", (*m_pModuleConfig)["TimeoutTime"].asInt(), (*m_pModuleConfig)["TimeoutCheckInterval"].asInt()));
		m_pControl->SendMessage(event);

		event->SetEvent(EVENT_TYPE::EnableClientCheck);
		event->SetMessage(CUniversal::Format("{\"ClientCheckKey\":\"%s\", \"ClientCheckTime\":%d}", (*m_pModuleConfig)["ClientCheckKey"].asString(), (*m_pModuleConfig)["ClientCheckKey"].asInt()));
		m_pControl->SendMessage(event);
	}
	m_pLog = IData::GetLog();
	m_nManagerConnection = sock;
	m_pControl->RegisterEventHandler(EVENT_TYPE::ProgramStart, std::bind(&SloongNetGateway::OnStart, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(EVENT_TYPE::SocketClose, std::bind(&SloongNetGateway::OnSocketClose, this, std::placeholders::_1));
	m_pControl->RegisterEventHandler(EVENT_TYPE::SendPackage, std::bind(&SloongNetGateway::SendPackageHook, this, std::placeholders::_1));
	return CResult::Succeed();
}

CResult SloongNetGateway::RequestPackageProcesser(void *env, CDataTransPackage *trans_pack)
{
	m_pLog->Verbos("Receive new request package.");
	auto res = MessageToProcesser(trans_pack);
	m_pLog->Verbos(CUniversal::Format("Response [%s][%s].", Core::ResultType_Name(res.Result()), res.Message()));
	if( res.Result() == ResultType::Invalid )
		return res;
	trans_pack->ResponsePackage(res);
	return CResult::Succeed();
}

CResult SloongNetGateway::ResponsePackageProcesser(CDataTransPackage *trans_pack)
{
	auto num = trans_pack->GetSerialNumber();
	if (!m_listSendEvent.exist(num))
		m_pLog->Error("ResponsePackageProcesser no find the package");

	auto send_evt = dynamic_pointer_cast<CSendPackageEvent>(m_listSendEvent[num]);
	auto need_send_res = send_evt->CallCallbackFunc(trans_pack);
	m_listSendEvent.erase(num);
	return need_send_res;
}

void SloongNetGateway::QueryReferenceInfo()
{
	auto event = make_shared<CSendPackageEvent>();
	event->SetCallbackFunc(std::bind(&SloongNetGateway::QueryReferenceInfoResponseHandler, this, std::placeholders::_1, std::placeholders::_2));
	event->SetRequest(m_nManagerConnection, m_pRuntimeData->nodeuuid(), m_nSerialNumber, 1, (int)Functions::QueryReferenceInfo, "");
	m_nSerialNumber++;
	m_pControl->SendMessage(event);
}

inline int SloongNetGateway::ParseFunctionValue(const string& s)
{
	int res = 0;
	auto nFunc = ConvertStrToInt(s, -1, &res);
	if (nFunc == -1)
		m_pLog->Error(CUniversal::Format("Parse function string[%s] to int error[%d].", s, res));
	return nFunc;
}

// process the provied function string to list.
list<int> SloongNetGateway::ProcessProviedFunction(const string& prov_func)
{
	list<int> res_list;
	auto funcs = CUniversal::split(prov_func, ',');
	for (auto func : funcs)
	{
		if (func.find("-") != string::npos)
		{
			auto range = CUniversal::split(func, '-');
			auto start = ParseFunctionValue(range[0]);
			auto end = ParseFunctionValue(range[1]);
			if (start == -1 || end == -1)
				return res_list;
			for (int i = start; i <= end; i++)
			{
				res_list.push_back(i);
			}
		}
		else
		{
			auto nFunc = ParseFunctionValue(func);
			if (nFunc == -1)
				return res_list;
			res_list.push_back(nFunc);
		}
	}
	return res_list;
}

CResult SloongNetGateway::QueryReferenceInfoResponseHandler(IEvent *send_pack, CDataTransPackage *res_pack)
{
	auto str_res = res_pack->GetRecvMessage();
	auto res = ConvertStrToObj<QueryReferenceInfoResponse>(str_res);
	if (res == nullptr || res->templateinfos_size() == 0)
		return CResult::Invalid();

	auto templateInfos = res->templateinfos();
	for (auto info : templateInfos)
	{
		if (info.providefunctions() == "*")
			m_mapFuncToTemplateIDs[-1].unique_insert(info.templateid());
		else
		{
			for (auto i : ProcessProviedFunction(info.providefunctions()))
				m_mapFuncToTemplateIDs[i].unique_insert(info.templateid());
		}
		for (auto item : info.nodeinfos())
		{
			m_mapUUIDToNode[item.uuid()] = item;
			m_mapTempteIDToUUIDs[info.templateid()].push_back(item.uuid());
			auto conn = make_shared<EasyConnect>();
			conn->Initialize(item.address(), item.port());
			conn->Connect();
			m_mapUUIDToConnect[item.uuid()] = conn;
		}
	}
	return CResult::Invalid();
}

CResult SloongNetGateway::CreateProcessEnvironmentHandler(void **out_env)
{
	/*auto item = make_shared<GatewayTranspond>();
	auto res = item->Initialize(m_pControl);
	if (res.IsFialed())
		return res;
	m_listTranspond.push_back(item);
	(*out_env) = item.get();*/
	return CResult::Succeed();
}

void SloongNetGateway::SendPackageHook(SmartEvent event)
{
	auto send_evt = dynamic_pointer_cast<CSendPackageEvent>(event);
	auto pack = send_evt->GetDataPackage();
	m_listSendEvent[pack->serialnumber()] = event;
}

void SloongNetGateway::OnStart(SmartEvent evt)
{
	QueryReferenceInfo();
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

void Sloong::SloongNetGateway::EventPackageProcesser(CDataTransPackage *trans_pack)
{
	auto data_pack = trans_pack->GetRecvPackage();
	auto event = (Manager::Events)data_pack->function();
	if (!Manager::Events_IsValid(event))
	{
		m_pLog->Error(CUniversal::Format("EventPackageProcesser is called.but the fucntion[%d] check error.", event));
		return;
	}

	switch (event)
	{
	case Manager::Events::ReferenceModuleOnline:
	{
		m_pLog->Info("Receive ReferenceModuleOnline event");
	}
	break;
	case Manager::Events::ReferenceModuleOffline:
	{
		m_pLog->Info("Receive ReferenceModuleOffline event");
	}
	break;
	default:
	{
		m_pLog->Error(CUniversal::Format("Event is no processed. [%s][%d].", Manager::Events_Name(event), event));
	}
	break;
	}
}

CResult Sloong::SloongNetGateway::MessageToProcesser(CDataTransPackage* pack)
{
	auto data_pack = pack->GetRecvPackage();
	if(!m_mapFuncToTemplateIDs.exist(data_pack->function()) && !m_mapFuncToTemplateIDs.exist(-1))
	{
		return CResult::Make_Error(CUniversal::Format("No service can process for [%s].",data_pack->function()));
	}

	auto tpl_list = m_mapFuncToTemplateIDs[data_pack->function()];
	tpl_list.copyfrom(m_mapFuncToTemplateIDs[-1]);

	SmartConnect target = nullptr;
	for( auto tpl: tpl_list)
	{
		if(m_mapTempteIDToUUIDs[tpl].size() ==0)
			continue;

		for( auto node: m_mapTempteIDToUUIDs[tpl]){
			target = m_mapUUIDToConnect[node];
			break;
		}
	}

	auto event = make_shared<CSendPackageExEvent>();
	event->SetCallbackFunc(std::bind(&SloongNetGateway::MessageToClient, this, std::placeholders::_1, std::placeholders::_2));
	event->SetRequestInfo(pack->GetConnection(),data_pack);
	event->SetRequest(target->GetSocketID(), m_pRuntimeData->nodeuuid(), m_nSerialNumber, pack->GetPriority() , (int)Processer::Functions::ProcessMessage , pack->GetRecvMessage());
	m_nSerialNumber++;
	m_pControl->SendMessage(event);
	return CResult::Invalid();
}

CResult Sloong::SloongNetGateway::MessageToClient(IEvent *send_event, CDataTransPackage *res_pack)
{
	auto req_event = TYPE_TRANS<CSendPackageExEvent*>(send_event);
	auto req_info = req_event->GetRequestInfo();
	auto req_pack = req_info->RequestDataPackage;

	auto res_data = res_pack->GetRecvPackage();
	req_pack->set_content(res_data->content());
	req_pack->set_result(res_data->result());
	if( res_data->extend().size()>0)
		req_pack->set_extend(res_data->extend());

	res_pack->ResponsePackage(req_pack);
	res_pack->SetConnection(req_info->RequestConnect);
	return CResult::Succeed();
}