
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

		virtual void Run();

		virtual void Exit();

		virtual bool ConnectToControl(string controlAddress);

    protected:
        static void sloong_terminator();

        static void on_sigint(int signal);

        static void on_SIGINT_Event(int signal);

	protected:
		unique_ptr<CNetworkHub> m_pNetwork = make_unique<CNetworkHub>();
		unique_ptr<CControlHub> m_pControl = make_unique<CControlHub>();
		unique_ptr<CLog>		m_pLog = make_unique<CLog>();;
		unique_ptr<GLOBAL_CONFIG> m_pServerConfig;
		shared_ptr<EasyConnect>	m_pSocket;
		CEasySync				m_oSync;
		u_int64_t				m_nSerialNumber=0;
		string					m_strUUID;
		
    public:
        static unique_ptr<CSloongBaseService> g_pAppService;
	};

    
}



#endif