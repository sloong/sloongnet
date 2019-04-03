#ifndef SLOONGNET_PROXY_SERVICE_H
#define SLOONGNET_PROXY_SERVICE_H


#include "IEvent.h"
#include "IControl.h"
namespace Sloong
{
	class CControlHub;
	class CNetworkHub;
	class lConnect;
	class SloongNetProxy
	{
	public:
		SloongNetProxy();
		~SloongNetProxy();

		bool Initialize(int argc, char** args);
		bool ConnectToControl(string controlAddress);
		void Run();
		void Exit();

		void OnReceivePackage(SmartEvent evt);
		void OnSocketClose(SmartEvent evt);
	protected:
		unique_ptr<CNetworkHub> m_pNetwork;
		unique_ptr<CControlHub> m_pControl;
		shared_ptr<lConnect>	m_pSocket;
		unique_ptr<CLog>	m_pLog;
		ProtobufMessage::PROXY_CONFIG m_oConfig;
	};

}

#endif