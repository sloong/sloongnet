/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2019-11-05 08:59:19
 * @LastEditTime: 2020-07-28 20:37:34
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/manager/manager.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */

#pragma once

#include "core.h"
#include "export.h"
#include "servermanage.h"

extern "C"
{
	PackageResult RequestPackageProcesser(void *, DataPackage *);
	PackageResult ResponsePackageProcesser(void *, DataPackage *);
	CResult EventPackageProcesser(DataPackage *);
	CResult NewConnectAcceptProcesser(SOCKET);
	CResult PrepareInitialize(GLOBAL_CONFIG*);
	CResult ModuleInitialization(IControl*);
	CResult ModuleInitialized();
	CResult CreateProcessEnvironment(void **);
}

namespace Sloong
{
	class CConfiguation;
	class SloongControlService : public IObject
	{
	public:
		static CResult PrepareInitialize(GLOBAL_CONFIG*);
		static void ResetControlConfig(GLOBAL_CONFIG *);
		
	public:
		SloongControlService() {}
		virtual ~SloongControlService() {}

		CResult Initialization(IControl *);
		CResult Initialized();

		inline CResult CreateProcessEnvironmentHandler(void **);
		void EventPackageProcesser(DataPackage *);

	protected:
		void OnConnectionBreaked(SharedEvent);

	protected:
		list<unique_ptr<CServerManage>> m_listServerManage;
		GLOBAL_CONFIG *m_pConfig;

	public:
		static string g_strDBFilePath;

	public:
		static unique_ptr<SloongControlService> Instance;
	};

} // namespace Sloong
