
#ifndef SLOONGNET_BASE_SERVICE_H
#define SLOONGNET_BASE_SERVICE_H

#include "IData.h"
#include "IEvent.h"
#include "IControl.h"
#include "DataTransPackage.h"
#include "ControlHub.h"
#include "NetworkHub.h"
namespace Sloong
{
	typedef std::function<bool(Functions, string, SmartPackage)> FuncHandler;
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
		virtual CResult Initialize(unique_ptr<GLOBAL_CONFIG>& config);

		virtual void AfterInit() {}

		virtual CResult Run();

		virtual void Exit();

		virtual bool ConnectToControl(string controlAddress);

		void MessagePackageProcesser(SmartPackage);

    protected:
        static void sloong_terminator();

        static void on_sigint(int signal);

        static void on_SIGINT_Event(int signal);

		void RegistFunctionHandler(Functions func, FuncHandler handler);

	protected:
		unique_ptr<CNetworkHub> m_pNetwork = make_unique<CNetworkHub>();
		unique_ptr<CControlHub> m_pControl = make_unique<CControlHub>();
		unique_ptr<CLog>		m_pLog = make_unique<CLog>();;
		unique_ptr<GLOBAL_CONFIG> m_pServerConfig;
		shared_ptr<EasyConnect>	m_pSocket;
		CEasySync				m_oExitSync;
		CResult					m_oExitResult = CResult::Succeed;
		u_int64_t				m_nSerialNumber=0;
		string					m_strUUID;

		map_ex<Functions, FuncHandler> m_oFunctionHandles;
		
    public:
        static unique_ptr<CSloongBaseService> g_pAppService;
	};

    
}



#endif