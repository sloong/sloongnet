/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 16:37:42
 * @Description: file content
 */
#ifndef SLOONGNET_CONTROL_SERVICE_H
#define SLOONGNET_CONTROL_SERVICE_H


#include "base_service.h"

namespace Sloong
{
	class CConfiguation;
	class SloongControlService : public CSloongBaseService
	{
	public:
		SloongControlService() : CSloongBaseService()
		{
			m_pAllConfig = make_unique<CConfiguation>();
		}

		CResult Initialize(int argc, char** args);
		
		void MessagePackageProcesser(SmartPackage);
		void OnSocketClose(SmartEvent);
		void PrientHelp();

	protected:
		void ResetControlConfig();

	protected:
		unique_ptr<CConfiguation>	m_pAllConfig;
		CONTROL_CONFIG m_oConfig;
	};

}



#endif //SLOONGNET_CONTROL_SERVICE_H
