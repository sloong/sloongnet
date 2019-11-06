#ifndef SLOONGNET_FIREWALL_SERVICE_H
#define SLOONGNET_FIREWALL_SERVICE_H


#include "base_service.h"
namespace Sloong
{
	class SloongNetFirewall : public CSloongBaseService
	{
	public:
		SloongNetFirewall() : CSloongBaseService(){}

		void AfterInit();

		void MessagePackageProcesser(SmartPackage);

		void OnSocketClose(SmartEvent evt);
	protected:
		FIREWALL_CONFIG m_oConfig;
	};

}

#endif