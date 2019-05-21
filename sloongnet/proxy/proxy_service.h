#ifndef SLOONGNET_PROXY_SERVICE_H
#define SLOONGNET_PROXY_SERVICE_H


#include "sockinfo.h"
#include "base_service.h"
namespace Sloong
{
	class SloongNetProxy : public CSloongBaseService
	{
	public:
		SloongNetProxy() : CSloongBaseService(ModuleType::Proxy){}

		bool Initialize(int argc, char** args);
		
		bool ConnectToProcess();

		// Event handler
		void OnStart(SmartEvent);
		void OnSocketClose(SmartEvent);

		// Network processer
		void MessagePackageProcesser(SmartPackage);
		void AcceptConnectProcesser(shared_ptr<CSockInfo>);
		
	protected:
		// 
		map<int,int>	m_mapProcessLoadList;
		map<int,SmartConnect> 	m_mapProcessList;
		map<int,string>		m_mapUUIDList;
		map<u_int64_t,SmartPackage> m_mapPackageList;
		ProtobufMessage::PROXY_CONFIG m_oConfig;
	};

}

#endif