#pragma once

#include "main.h"
#include "config.pb.h"

namespace Sloong
{
	class CConfiguation;
	class IData
	{
	public:
		static void Initialize(IControl* ic){
			if ( m_iC == nullptr) m_iC = ic;
		}
		static MessageConfig::DATA_CONFIG* GetDataConfig(){
			return TYPE_TRANS<MessageConfig::DATA_CONFIG*>(m_iC->Get(DATA_ITEM::Configuation));
		}	
		static CLog*	GetLog(){
			return TYPE_TRANS<CLog*>(m_iC->Get(DATA_ITEM::Logger));
		}
	public:
		static IControl* m_iC;
	};
}