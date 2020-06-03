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
#include "IObject.h"

#include "dbhub.h"

extern "C" {
	CResult RequestPackageProcesser(void *, CDataTransPackage *);
	CResult ResponsePackageProcesser(void *, CDataTransPackage *);
	CResult EventPackageProcesser(CDataTransPackage *);
	CResult NewConnectAcceptProcesser(CSockInfo *);
	CResult ModuleInitialization(GLOBAL_CONFIG *);
	CResult ModuleInitialized(SOCKET, IControl *);
	CResult CreateProcessEnvironment(void **);
}

namespace Sloong
{	
	class CDataCenter : public IObject
	{
	public:
		CDataCenter() {}

		CResult Initialization(GLOBAL_CONFIG *);
		CResult Initialized(SOCKET, IControl *);

		inline CResult CreateProcessEnvironmentHandler(void **);
		void EventPackageProcesser(CDataTransPackage *);

	protected:
		list<unique_ptr<DBHub>> m_listDBHub;

		GLOBAL_CONFIG* m_pConfig;
		Json::Value *m_pModuleConfig;
		RuntimeDataPackage *m_pRuntimeData = nullptr;
		SOCKET m_nManagerConnection = -1;

	public:
		static unique_ptr<CDataCenter> Instance;
	};

}

#endif