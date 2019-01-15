#pragma once

#include "main.h"


namespace Sloong
{
	class CServerConfig;
	class IData
	{
	public:
		static void Initialize(IControl* ic){
			if ( m_iC == nullptr) m_iC = ic;
		}
		static CServerConfig* GetServerConfig(){
			return TYPE_TRANS<CServerConfig*>(m_iC->Get(DATA_ITEM::Configuation));
		}
		static CLog*	GetLog(){
			return TYPE_TRANS<CLog*>(m_iC->Get(DATA_ITEM::Logger));
		}
	public:
		static IControl* m_iC;
	};
}