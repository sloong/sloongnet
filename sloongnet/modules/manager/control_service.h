/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-11 18:30:55
 * @Description: file content
 */
#ifndef SLOONGNET_CONTROL_SERVICE_H
#define SLOONGNET_CONTROL_SERVICE_H


#include "core.h"
#include "export.h"
#include "servermanage.h"

extern "C" {
	CResult MessagePackageProcesser(void*,CDataTransPackage*);
	CResult EventPackageProcesser(CDataTransPackage*);
	CResult NewConnectAcceptProcesser(CSockInfo*);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(SOCKET,IControl*);
	CResult CreateProcessEnvironment(void**);
}

namespace Sloong
{
	class CConfiguation;
	class SloongControlService
	{
	public:
		SloongControlService() {}
		virtual ~SloongControlService() {}

		CResult Initialization(GLOBAL_CONFIG*);
		CResult Initialized(IControl*);

		inline CResult CreateProcessEnvironmentHandler(void**);
		void EventPackageProcesser(CDataTransPackage*);

	protected:
		void ResetControlConfig(GLOBAL_CONFIG* config);

		void OnSocketClose(SmartEvent event);
		
	protected:
		list<shared_ptr<CServerManage>> m_listServerManage;
		unique_ptr<CServerManage>	m_pServer = make_unique<CServerManage>();
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;
		GLOBAL_CONFIG* m_pConfig;
	public:
		static unique_ptr<SloongControlService> Instance;
	};

}



#endif //SLOONGNET_CONTROL_SERVICE_H
