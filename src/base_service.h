/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 19:40:31
 * @Description: file content
 */

#ifndef SLOONGNET_BASE_SERVICE_H
#define SLOONGNET_BASE_SERVICE_H

#include "main.h"
#include "IData.h"
#include "IEvent.h"
#include "IControl.h"
#include "DataTransPackage.h"
#include "ControlHub.h"
#include "NetworkHub.h"
#include <dlfcn.h>
namespace Sloong
{
	class CSloongBaseService
	{
	public:
		CSloongBaseService() {}

		virtual ~CSloongBaseService(){}

		// Just call it without Control module.
		virtual CResult Initialize(bool, string, int, int=0);

		virtual CResult Run();
		virtual void Stop();

		TResult<shared_ptr<DataPackage>> RegisteToControl(EasyConnect *con, string uuid);

	protected:
		virtual CResult InitlializeForWorker(RuntimeDataPackage *, int);
		virtual CResult InitlializeForManager(RuntimeDataPackage *);

		CResult RegisteNode();
		CResult InitModule();
		void InitSystem();
		
	protected:
		void OnProgramRestartEventHandler(SharedEvent);
		void OnProgramStopEventHandler(SharedEvent);
		void OnSendPackageToManagerEventHandler(SharedEvent);
		void OnGetManagerSocketEventHandler(SharedEvent);

	protected:
		static void sloong_terminator();
		static void sloong_unexpected();
		static void on_sigint(int);
		static void on_SIGINT_Event(int);

	protected:
		unique_ptr<CNetworkHub> m_pNetwork = make_unique<CNetworkHub>();
		unique_ptr<CControlHub> m_iC = make_unique<CControlHub>();
		unique_ptr<CLog> m_pLog = make_unique<CLog>();
		RuntimeDataPackage m_oServerConfig;
		UniqueConnection m_pManagerConnect = nullptr;
		Json::Value m_oModuleConfig;
		EasySync m_oExitSync;
		CResult m_oExitResult = CResult::Succeed();
		void *m_pModule = nullptr;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;

		CreateProcessEnvironmentFunction m_pModuleCreateProcessEvnFunc = nullptr;
		RequestPackageProcessFunction m_pModuleRequestHandler = nullptr;
		ResponsePackageProcessFunction m_pModuleResponseHandler = nullptr;
		EventPackageProcessFunction m_pModuleEventHandler = nullptr;
		NewConnectAcceptProcessFunction m_pModuleAcceptHandler = nullptr;
		ModuleInitializationFunction m_pModuleInitializationFunc = nullptr;
		ModuleInitializedFunction m_pModuleInitializedFunc = nullptr;

		static constexpr int REPORT_LOAD_STATUS_INTERVAL = 1000*60; // one mintue
	public:
		static unique_ptr<CSloongBaseService> Instance;
	};
} // namespace Sloong
#endif