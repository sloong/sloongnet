/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 20:38:24
 * @Description: file content
 */
#ifndef SLOONGNET_GATEWAY_SERVICE_H
#define SLOONGNET_GATEWAY_SERVICE_H


#include "sockinfo.h"
#include "core.h"
#include "export.h"

#include "protocol/manager.pb.h"
using namespace Manager;

#include "transpond.h"
#include <jsoncpp/json/json.h>

extern "C" {
	CResult RequestPackageProcesser(void*,CDataTransPackage*);
	CResult ResponsePackageProcesser(void*,CDataTransPackage*);
	CResult EventPackageProcesser(CDataTransPackage*);
	CResult NewConnectAcceptProcesser(CSockInfo*);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(SOCKET,IControl*);
	CResult CreateProcessEnvironment(void**);
}

namespace Sloong
{
	class SloongNetGateway
	{
	public:
		SloongNetGateway(){}

		CResult Initialization(GLOBAL_CONFIG*);
		CResult Initialized(SOCKET,IControl*);
		
		void QueryProcessListRequest();
		void QueryProcessListResponse(DataPackage*,CDataTransPackage*);

		inline CResult CreateProcessEnvironmentHandler(void**);
		void EventPackageProcesser(CDataTransPackage*);
		CResult RequestPackageProcesser(void*,CDataTransPackage*);
		void ResponsePackageProcesser(CDataTransPackage*);

		// Event handler
		void OnStart(SmartEvent);
		void OnSocketClose(SmartEvent);
		void SendPackageHook(SmartEvent);
	protected:
		list<shared_ptr<GatewayTranspond>> m_listTranspond;

	protected:
		map_ex<int,SmartEvent>	m_listSendEvent;
		map<int,string> m_mapEventType;
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;
		GLOBAL_CONFIG* m_pConfig;
		Json::Value    m_oExConfig;
		SOCKET   m_nManagerConnection = -1;
		int		 m_nSerialNumber = 0;
	public:
		static unique_ptr<SloongNetGateway> Instance;
	};

}

#endif