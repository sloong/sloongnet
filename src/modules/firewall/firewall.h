/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 14:16:37
 * @Description: file content
 */
#ifndef SLOONGNET_FIREWALL_SERVICE_H
#define SLOONGNET_FIREWALL_SERVICE_H


#include "core.h"
#include "export.h"

extern "C" {
	PackageResult RequestPackageProcesser(void*,DataPackage*);
	PackageResult ResponsePackageProcesser(void*,DataPackage*);
	CResult EventPackageProcesser(DataPackage*);
	CResult NewConnectAcceptProcesser(SOCKET);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(SOCKET, IControl *);
	CResult CreateProcessEnvironment(void**);
}

namespace Sloong
{
	class SloongNetFirewall
	{
	public:
		SloongNetFirewall(){}

		CResult Initialized(IControl*);

		PackageResult RequestPackageProcesser(DataPackage*);
		PackageResult ResponsePackageProcesser(DataPackage*);
		inline CResult CreateProcessEnvironmentHandler(void**);

		void EventPackageProcesser(DataPackage*);
		
	protected:

	protected:
		IControl* 	m_iC = nullptr;
		CLog*		m_pLog =nullptr;
		GLOBAL_CONFIG* m_pConfig;
	public:
		static unique_ptr<SloongNetFirewall> Instance;
	};

}

#endif