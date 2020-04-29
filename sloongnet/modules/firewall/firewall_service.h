/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-29 15:49:07
 * @Description: file content
 */
#ifndef SLOONGNET_FIREWALL_SERVICE_H
#define SLOONGNET_FIREWALL_SERVICE_H


#include "core.h"
#include "export.h"

extern "C" {
	CResult MessagePackageProcesser(void*,CDataTransPackage*);
	CResult NewConnectAcceptProcesser(CSockInfo*);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(IControl*);
	CResult CreateProcessEnvironment(void**);
}

namespace Sloong
{
	typedef std::function<bool(Functions, string, CDataTransPackage*)> FuncHandler;
	class SloongNetFirewall
	{
	public:
		SloongNetFirewall(){}

		CResult Initialized(IControl*);

		CResult MessagePackageProcesser(CDataTransPackage*);
		inline CResult CreateProcessEnvironmentHandler(void**);

		void OnSocketClose(SmartEvent evt);
	protected:

	protected:
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;
		GLOBAL_CONFIG* m_pConfig;
	public:
		static unique_ptr<SloongNetFirewall> Instance;
	};

}

#endif