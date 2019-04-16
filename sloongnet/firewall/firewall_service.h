#ifndef SLOONGNET_FIREWALL_SERVICE_H
#define SLOONGNET_FIREWALL_SERVICE_H


#include "IEvent.h"
#include "IControl.h"
namespace Sloong
{
	class CControlHub;
	class CNetworkHub;
	class SloongNetFirewall
	{
	public:
		SloongNetFirewall();
		~SloongNetFirewall();

		bool Initialize(int argc, char** args);
		bool ConnectToControl(string controlAddress);
		void Run();
		void Exit();

		void OnReceivePackage(SmartEvent evt);
		void OnSocketClose(SmartEvent evt);
	protected:
		unique_ptr<CNetworkHub> m_pNetwork;
		unique_ptr<CControlHub> m_pControl;
		shared_ptr<EasyConnect>	m_pSocket;
		unique_ptr<CLog>		m_pLog;
		u_int64_t	m_nSerialNumber=0;
		ProtobufMessage::FIREWALL_CONFIG m_oConfig;
	};

}

#endif