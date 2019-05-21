#ifndef SLOONGNET_DATA_SERVICE_H
#define SLOONGNET_DATA_SERVICE_H


#include "base_service.h"
namespace Sloong
{
	class SloongNetDataCenter : public CSloongBaseService
	{
	public:
		SloongNetDataCenter() : CSloongBaseService(ModuleType::DataCenter){}

		bool Initialize(int argc, char** args);
		
		void MessagePackageProcesser(SmartPackage);
		void OnSocketClose(SmartEvent evt);
	protected:
		ProtobufMessage::DATA_CONFIG m_oConfig;
	};

}

#endif