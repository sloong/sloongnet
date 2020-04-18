/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 15:33:49
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
			return TYPE_TRANS<GLOBAL_CONFIG*>(m_iC->Get(DATA_ITEM::ServerConfiguation ));
		}
		static Json::Value* GetModuleConfig(){
			return TYPE_TRANS<Json::Value*>(m_iC->Get(DATA_ITEM::ModuleConfiguation ));
		}
		static CLog* GetLog(){
			return TYPE_TRANS<CLog*>(m_iC->Get(DATA_ITEM::Logger));
		}
	public:
		static IControl* m_iC;
	};
}

#endif