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
namespace Sloong
{
	class CSloongBaseService
	{
	public:
		CSloongBaseService() {}

		virtual ~CSloongBaseService(){
            Exit();
            CThreadPool::Exit();
            m_pLog->End();
        }

        // Just call it without Control module.
		virtual CResult Initialize(bool, string, int);

		virtual CResult Run();
		virtual void Restart(SmartEvent event);
		virtual void Exit();

		TResult<shared_ptr<DataPackage>> RegisteToControl(SmartConnect con, string uuid);
	protected:
		virtual CResult InitlializeForWorker(RuntimeDataPackage*);
		virtual CResult InitlializeForManager(RuntimeDataPackage*);
		
		CResult RegisteNode();
		CResult	InitModule();
		void	InitSystemEventHandler();

    protected:
        static void sloong_terminator();
        static void on_sigint(int signal);
        static void on_SIGINT_Event(int signal);

	protected:
		unique_ptr<CNetworkHub> m_pNetwork = make_unique<CNetworkHub>();
		unique_ptr<CControlHub> m_pControl = make_unique<CControlHub>();
		unique_ptr<CLog>		m_pLog = make_unique<CLog>();;
		RuntimeDataPackage		m_oServerConfig;
		SmartConnect 			m_pManagerConnect = nullptr;
		Json::Value 			m_oModuleConfig;
		shared_ptr<EasyConnect>	m_pSocket;
		CEasySync				m_oExitSync;
		CResult					m_oExitResult = CResult::Succeed();
		u_int64_t				m_nSerialNumber=0;
		string					m_strUUID;
		void*					m_pModule = nullptr;

		CreateProcessEnvironmentFunction	m_pModuleCreateProcessEvnFunc = nullptr;
		RequestPackageProcessFunction 	m_pModuleRequestHandler = nullptr;
		ResponsePackageProcessFunction 	m_pModuleResponseHandler = nullptr;
		EventPackageProcessFunction 	m_pModuleEventHandler = nullptr;
		NewConnectAcceptProcessFunction m_pModuleAcceptHandler = nullptr;
		ModuleInitializationFunction	m_pModuleInitializationFunc = nullptr;
		ModuleInitializedFunction		m_pModuleInitializedFunc = nullptr;
		
    public:
        static unique_ptr<CSloongBaseService> Instance;
	};
}
#endif