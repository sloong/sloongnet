#ifndef SLOONGNET_PROXY_SERVICE_H
#define SLOONGNET_PROXY_SERVICE_H


#include "sockinfo.h"
#include "base_service.h"
namespace Sloong
{
	class SloongNetProxy : public CSloongBaseService
	{
	public:
		SloongNetProxy() : CSloongBaseService(){}

		void AfterInit();
		
		bool ConnectToProcess();

		// Event handler
		void OnStart(SmartEvent);
		void OnSocketClose(SmartEvent);

		// Network processer
		bool ProcessMessageHanlder(Functions func, string sender, SmartPackage pack);
		void AcceptConnectProcesser(shared_ptr<CSockInfo>);
	private:
		void MessageToProcesser(SmartPackage pack);
		void MessageToClient(SmartPackage pack);
	protected:
		// 
		map<int,int>	m_mapProcessLoadList;
		map<int,SmartConnect> 	m_mapProcessList;
		map<int,string>		m_mapUUIDList;
		map<u_int64_t,SmartPackage> m_mapPackageList;
		GATEWAY_CONFIG m_oConfig;
	};

}

#endif