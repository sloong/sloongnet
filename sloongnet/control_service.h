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
#include "configuation.h"
namespace Sloong
{
	class CConfiguation;
	class SloongControlService : public CSloongBaseService
	{
	public:
		SloongControlService() {}
		virtual ~SloongControlService() {}

		CResult Initialize(unique_ptr<GLOBAL_CONFIG>& config);
		void AfterInit();

		void MessagePackageProcesser(SmartPackage);

	protected:
		void ResetControlConfig();

	protected:
		unique_ptr<CConfiguation>	m_pAllConfig = make_unique<CConfiguation>();;
		CONTROL_CONFIG m_oConfig;
	};

}



#endif //SLOONGNET_CONTROL_SERVICE_H
