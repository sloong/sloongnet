#include "data_service.h"
#include "NetworkHub.h"
#include "ControlHub.h"
#include "IData.h"
#include "utility.h"
#include "NetworkEvent.hpp"
#include "DataTransPackage.h"
using namespace Sloong;
using namespace Sloong::Events;

int main(int argc, char **args)
{
	try
	{
		Sloong::CSloongBaseService::g_pAppService = make_unique<SloongNetDataCenter>();

		if (Sloong::CSloongBaseService::g_pAppService->Initialize(argc, args))
			Sloong::CSloongBaseService::g_pAppService->Run();
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. " << endl;
		CUtility::write_call_stack();
	}
}


bool SloongNetDataCenter::Initialize(int argc, char **args)
{
	try
	{
		if( !CSloongBaseService::Initialize(argc,args))
			throw string("Error in CSloongBaseService::Initialize");

		if(!m_oConfig.ParseFromString(m_szConfigData))
			throw string("Parse the config struct error.");
		else
			cout << "Parse special configuation succeed." << endl;

		m_pNetwork->RegisterMessageProcesser(std::bind(&SloongNetDataCenter::MessagePackageProcesser, this, std::placeholders::_1));
		m_pControl->RegisterEventHandler(SocketClose, std::bind(&SloongNetDataCenter::OnSocketClose, this, std::placeholders::_1));

		return true;
	}
	catch (exception &e)
	{
		cout << "exception happened, system will shutdown. message:" << e.what() << endl;
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. " << endl;
		CUtility::write_call_stack();
	}

	return false;
}

void Sloong::SloongNetDataCenter::MessagePackageProcesser(SmartPackage pack)
{
	pack->ResponsePackage("{\"errno\": \"-1\",\"errmsg\" : \"server process happened error\"}");
	auto response_event = make_shared<CNetworkEvent>(EVENT_TYPE::SendMessage);
	response_event->SetSocketID(pack->GetSocketID());
	response_event->SetDataPackage(pack);
	m_pControl->CallMessage(response_event);
}

void Sloong::SloongNetDataCenter::OnSocketClose(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto info = net_evt->GetUserInfo();
	if (!info)
	{
		m_pLog->Error(CUniversal::Format("Get socket info from socket list error, the info is NULL. socket id is: %d", net_evt->GetSocketID()));
		return;
	}
	// call close function.
	//m_pProcess->CloseSocket(info);
	//net_evt->CallCallbackFunc(net_evt);
}
