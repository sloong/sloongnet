#ifndef SLOONGNET_PROXY_SERVICE_H
#define SLOONGNET_PROXY_SERVICE_H


#include "IEvent.h"
#include "IControl.h"
#include "sockinfo.h"
#include "DataTransPackage.h"
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

		void MessagePackageProcesser(SmartPackage evt);
		void AcceptConnectProcesser(shared_ptr<CSockInfo>);
		void OnSocketClose(SmartEvent evt);
	protected:
		unique_ptr<CNetworkHub> m_pNetwork;
		unique_ptr<CControlHub> m_pControl;
		shared_ptr<EasyConnect>	m_pSocket;
		unique_ptr<CLog>	m_pLog;
		// 
		map<int,int>	m_mapProcessLoadList;
		map<int,SmartConnect> 	m_mapProcessList;
		map<int,string>		m_mapUUIDList;
		map<u_int64_t,SmartPackage> m_mapPackageList;
		u_int64_t	m_nSerialNumber=0;
		ProtobufMessage::PROXY_CONFIG m_oConfig;
		CEasySync			m_oSync;
	};

}

#endif