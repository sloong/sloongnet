#ifndef SLOONGNET_PROXY_SERVICE_H
#define SLOONGNET_PROXY_SERVICE_H


#include "IEvent.h"
#include "IControl.h"
namespace Sloong
{
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

		bool ConnectToControl(string controlAddress);
		bool ConnectToProcess();

		void OnReceivePackage(SmartEvent evt);
		void OnSocketClose(SmartEvent evt);
	protected:
		unique_ptr<CNetworkHub> m_pNetwork;
		unique_ptr<CControlHub> m_pControl;
		shared_ptr<lConnect>	m_pSocket;
		unique_ptr<CLog>	m_pLog;
		// 
		map<int,int>	m_mapProcessLoadList;
		map<int,SmartConnect> 	m_mapProcessList;
		map<u_int64_t,SmartEvent> m_mapEventList;
		u_int64_t	m_nSerialNumber=0;
		ProtobufMessage::PROXY_CONFIG m_oConfig;
		CEasySync			m_oSync;
	};

}

#endif