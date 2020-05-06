/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-30 11:51:47
 * @Description: file content
 */
#ifndef SLOONGNET_GATEWAY_SERVICE_H
#define SLOONGNET_GATEWAY_SERVICE_H


#include "sockinfo.h"
#include "core.h"
#include "export.h"

#include "protocol/manager.pb.h"
using namespace Manager;
#include "protocol/gateway.pb.h"
using namespace Gateway;

#include "transpond.h"
#include <jsoncpp/json/json.h>

extern "C" {
	CResult MessagePackageProcesser(void*,CDataTransPackage*);
	CResult EventPackageProcesser(CDataTransPackage*);
	CResult NewConnectAcceptProcesser(CSockInfo*);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(IControl*);
	CResult CreateProcessEnvironment(void**);
}

namespace Sloong
{
	typedef std::function<bool(Functions, string, CDataTransPackage*)> FuncHandler;
	class SloongNetGateway
	{
	public:
		SloongNetGateway(){}

		CResult Initialization(GLOBAL_CONFIG*);
		CResult Initialized(IControl*);
		
		bool ConnectToProcess();

		inline CResult CreateProcessEnvironmentHandler(void**);
		CResult EventPackageProcesser(CDataTransPackage*);

		// Event handler
		void OnStart(SmartEvent);
		void OnSocketClose(SmartEvent);
	protected:
		list<shared_ptr<GatewayTranspond>> m_listTranspond;

	protected:
		map<int,string> m_mapEventType;
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;
		GLOBAL_CONFIG* m_pConfig;
		Json::Value    m_oExConfig;
	public:
		static unique_ptr<SloongNetGateway> Instance;
	};

}

#endif