#ifndef SLOONGNET_PROXY_SERVICE_H
#define SLOONGNET_PROXY_SERVICE_H


#include "IEvent.h"
#include "IControl.h"
namespace Sloong
{
	class CServerConfig;
	class CControlHub;
	class CNetworkHub;
	class SloongNetProxy
	{
	public:
		SloongNetProxy();
		~SloongNetProxy();

		bool Initialize(int argc, char** args);

		void Run();
		void Exit();

		void OnReceivePackage(SmartEvent evt);
		void OnSocketClose(SmartEvent evt);
	protected:
		unique_ptr<CNetworkHub> m_pNetwork;
		unique_ptr<CControlHub> m_pControl;
		CServerConfig config;
		
		unique_ptr<CLog>	m_pLog;
	};

}

#endif