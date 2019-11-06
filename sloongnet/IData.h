/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 17:32:59
 * @Description: file content
 */
#ifndef SLOONGNET_INTERFACE_DATA_H
#define SLOONGNET_INTERFACE_DATA_H

#include "IControl.h"
namespace Sloong
{
	class CConfiguation;
	class IData
	{
	public:
		static void Initialize(IControl* ic){
			if ( m_iC == nullptr) m_iC = ic;
		}
		static GLOBAL_CONFIG* GetGlobalConfig(){
			return TYPE_TRANS<GLOBAL_CONFIG*>(m_iC->Get(DATA_ITEM::ServerConfiguation ));
		}
		static PROCESS_CONFIG GetProcessConfig(){
			GLOBAL_CONFIG* config = GetGlobalConfig();
			PROCESS_CONFIG* exconfig1 = new PROCESS_CONFIG();
			exconfig1->ParseFromString(config->exconfig());
			return *exconfig1;
		}
		
		static CLog* GetLog(){
			return TYPE_TRANS<CLog*>(m_iC->Get(DATA_ITEM::Logger));
		}
	public:
		static IControl* m_iC;
	};
}

#endif