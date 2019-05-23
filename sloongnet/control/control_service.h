#ifndef SLOONGNET_CONTROL_SERVICE_H
#define SLOONGNET_CONTROL_SERVICE_H


#include "base_service.h"

namespace Sloong
{
	class CConfiguation;
	class SloongControlService : public CSloongBaseService
	{
	public:
		SloongControlService() : CSloongBaseService(ModuleType::ControlCenter)
		{
			m_pAllConfig = make_unique<CConfiguation>();
		}

		CResult Initialize(int argc, char** args);
		
		void MessagePackageProcesser(SmartPackage);
		void OnSocketClose(SmartEvent);
		void PrientHelp();
	protected:
		unique_ptr<CConfiguation>	m_pAllConfig;
		ProtobufMessage::CONTROL_CONFIG m_oConfig;
	};

}



#endif //SLOONGNET_CONTROL_SERVICE_H
