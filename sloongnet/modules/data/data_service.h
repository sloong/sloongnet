/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-07 16:56:33
 * @Description: file content
 */
#ifndef SLOONGNET_DATA_SERVICE_H
#define SLOONGNET_DATA_SERVICE_H


#include "core.h"
#include "export.h"

extern "C" {
	CResult MessagePackageProcesser(CDataTransPackage*);
	CResult NewConnectAcceptProcesser(CSockInfo*);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(IControl*);
}

namespace Sloong
{	
	class SloongNetDataCenter
	{
	public:
		SloongNetDataCenter() {}

		CResult Initialized(IControl*);

		CResult MessagePackageProcesser(CDataTransPackage*);
		
		void OnSocketClose(SmartEvent evt);
	protected:

	protected:
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;
		GLOBAL_CONFIG* m_pConfig;
	public:
		static unique_ptr<SloongNetDataCenter> Instance;
	};

}

#endif