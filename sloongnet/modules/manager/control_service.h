/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-26 11:58:22
 * @Description: file content
 */
#ifndef SLOONGNET_CONTROL_SERVICE_H
#define SLOONGNET_CONTROL_SERVICE_H


#include "core.h"
#include "export.h"
#include "servermanage.h"

extern "C" {
	CResult MessagePackageProcesser(void*,CDataTransPackage*);
	//CResult NewConnectAcceptProcesser(CSockInfo*);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(IControl*);
	CResult CreateProcessEnvironment(void**);
}

namespace Sloong
{
	
	typedef std::function<bool(Functions, string, CDataTransPackage*)> FuncHandler;

	class CConfiguation;
	class SloongControlService
	{
	public:
		SloongControlService() {}
		virtual ~SloongControlService() {}

		CResult Initialization(GLOBAL_CONFIG*);
		CResult Initialized(IControl*);

		inline CResult CreateProcessEnvironmentHandler(void**);

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
