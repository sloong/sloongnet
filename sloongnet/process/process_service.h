#ifndef SLOONGNET_PROCESS_SERVICE_H
#define SLOONGNET_PROCESS_SERVICE_H


#include "base_service.h"

namespace Sloong
{
	class CLuaProcessCenter;
	class SloongNetProcess: public CSloongBaseService
	{
	public:
		SloongNetProcess():CSloongBaseService(ModuleType::Process){
			m_pProcess = make_unique<CLuaProcessCenter>();
		}

		bool Initialize(int argc, char** args);

		void MessagePackageProcesser(SmartPackage);
		void OnSocketClose(SmartEvent evt);

	protected:
		unique_ptr<CLuaProcessCenter> m_pProcess;
		ProtobufMessage::PROCESS_CONFIG m_oConfig;
		map<string, unique_ptr<CLuaPacket>> m_mapUserInfoList;
	};

}


#endif