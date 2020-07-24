/*** 
 * @Author: Chuanbin Wang
 * @Date: 2019-11-05 08:59:19
 * @LastEditTime: 2020-07-24 16:30:35
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
	CResult ModuleInitialization(GLOBAL_CONFIG *);
	CResult ModuleInitialized(SOCKET, IControl *);
	CResult CreateProcessEnvironment(void **);
}

namespace Sloong
{
	class CConfiguation;
	class SloongControlService
	{
	public:
		SloongControlService() {}
		virtual ~SloongControlService() {}

		CResult Initialization(GLOBAL_CONFIG *);
		CResult Initialized(IControl *);

		inline CResult CreateProcessEnvironmentHandler(void **);
		void EventPackageProcesser(DataPackage *);

	protected:
		void ResetControlConfig(GLOBAL_CONFIG *);

		void OnConnectionBreak(SharedEvent);

	protected:
		list<unique_ptr<CServerManage>> m_listServerManage;
		IControl *m_iC = nullptr;
		CLog *m_pLog = nullptr;
		GLOBAL_CONFIG *m_pConfig;
		string m_strDBFilePath="./configuation.db";

	public:
		static unique_ptr<SloongControlService> Instance;
	};

} // namespace Sloong
