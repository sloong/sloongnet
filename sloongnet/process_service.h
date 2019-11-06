#ifndef SLOONGNET_PROCESS_SERVICE_H
#define SLOONGNET_PROCESS_SERVICE_H


#include "base_service.h"
#include "LuaProcessCenter.h"
namespace Sloong
{
	class SloongNetProcess: public CSloongBaseService
	{
	public:
		SloongNetProcess() :CSloongBaseService() {}
		~SloongNetProcess() {}

		void AfterInit();

		void MessagePackageProcesser(SmartPackage);
		void OnSocketClose(SmartEvent evt);

	protected:
		unique_ptr<CLuaProcessCenter> m_pProcess = make_unique<CLuaProcessCenter>();
		PROCESS_CONFIG m_oConfig;
		map<string, unique_ptr<CLuaPacket>> m_mapUserInfoList;
	};

}


#endif