/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 19:15:53
 * @Description: file content
 */
#ifndef SLOONGNET_INTERFACE_DATA_H
#define SLOONGNET_INTERFACE_DATA_H

#include "IControl.h"
#include "core.h"
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
			return STATIC_TRANS<GLOBAL_CONFIG*>(m_iC->Get(DATA_ITEM::ServerConfiguation ));
		}
		static Json::Value* GetModuleConfig(){
			return STATIC_TRANS<Json::Value*>(m_iC->Get(DATA_ITEM::ModuleConfiguation ));
		}
		static CLog* GetLog(){
			return STATIC_TRANS<CLog*>(m_iC->Get(DATA_ITEM::Logger));
		}
		static RuntimeDataPackage* GetRuntimeData(){
			return STATIC_TRANS<RuntimeDataPackage*>(m_iC->Get(DATA_ITEM::RuntimeData ));
		}
	public:
		static IControl* m_iC;
	};
}

#endif