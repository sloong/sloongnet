
#ifndef SLOONGNET_BASE_SERVICE_H
#define SLOONGNET_BASE_SERVICE_H

#include "IData.h"
#include "IEvent.h"
#include "IControl.h"
#include "DataTransPackage.h"
namespace Sloong
{
    class CControlHub;
    class CNetworkHub;
	class CSloongBaseService
	{
	public:
		CSloongBaseService( ModuleType moduleType ){
            m_pLog = make_unique<CLog>();
            m_pNetwork = make_unique<CNetworkHub>();
            m_pControl = make_unique<CControlHub>();
            m_emModuleType = moduleType;
        }

		virtual ~CSloongBaseService(){
            Exit();
            CThreadPool::Exit();
            m_pLog->End();
        }

        virtual string GetConfigFromControl(MessageFunction func);

        // Just call it without Control module.
		virtual bool Initialize(int argc, char** args);

		virtual void Run();

		virtual void Exit();

		virtual bool ConnectToControl(string controlAddress);

         

    protected:
        static void PrientHelp();

        static void sloong_terminator();

        static void on_sigint(int signal);

        static void on_SIGINT_Event(int signal);

	protected:
		unique_ptr<CNetworkHub> m_pNetwork;
		unique_ptr<CControlHub> m_pControl;
		shared_ptr<EasyConnect>	m_pSocket;
		unique_ptr<CLog>	m_pLog;

		u_int64_t	m_nSerialNumber=0;
        ProtobufMessage::GLOBAL_CONFIG m_oServerConfig;
		string      m_szConfigData;
		CEasySync			m_oSync;
        ModuleType      m_emModuleType = ModuleType::Undefine; 
    public:
        static unique_ptr<CSloongBaseService> g_pAppService;
	};

    
}



#endif