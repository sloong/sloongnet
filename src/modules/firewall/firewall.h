/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-28 20:04:28
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/firewall/firewall.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */

#ifndef SLOONGNET_FIREWALL_SERVICE_H
#define SLOONGNET_FIREWALL_SERVICE_H


#include "core.h"
#include "export.h"
#include "IObject.h"

extern "C" {
	PackageResult RequestPackageProcesser(void*,Package*);
	PackageResult ResponsePackageProcesser(void*,Package*);
	CResult EventPackageProcesser(Package*);
	CResult NewConnectAcceptProcesser(SOCKET);
	CResult ModuleInitialization(IControl*);
	CResult ModuleInitialized();
	CResult CreateProcessEnvironment(void**);
}

namespace Sloong
{
	class SloongNetFirewall : public IObject
	{
	public:
		SloongNetFirewall(){}

		CResult Initialization(IControl*);
		CResult Initialized();

		PackageResult RequestPackageProcesser(Package*);
		PackageResult ResponsePackageProcesser(Package*);
		inline CResult CreateProcessEnvironmentHandler(void**);

		void EventPackageProcesser(Package*);
		
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