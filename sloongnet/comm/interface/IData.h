#pragma once

#include "main.h"
#include "MessageTypeDef.h"

namespace Sloong
{
	class CConfiguation;
	class IData
	{
	public:
		static void Initialize(IControl* ic){
			if ( m_iC == nullptr) m_iC = ic;
		}
		static ProtobufMessage::DATA_CONFIG* GetDataConfig(){
			return TYPE_TRANS<ProtobufMessage::DATA_CONFIG*>(m_iC->Get(DATA_ITEM::Configuation));
		}	
		static CLog*	GetLog(){
			return TYPE_TRANS<CLog*>(m_iC->Get(DATA_ITEM::Logger));
		}
	public:
		static IControl* m_iC;
	};
}