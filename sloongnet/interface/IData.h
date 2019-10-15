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
		static ProtobufMessage::GLOBAL_CONFIG* GetGlobalConfig(){
			return TYPE_TRANS<ProtobufMessage::GLOBAL_CONFIG*>(m_iC->Get(DATA_ITEM::GlobalConfiguation));
		}
		static ProtobufMessage::DATA_CONFIG* GetDataCenterConfig(){
			return TYPE_TRANS<ProtobufMessage::DATA_CONFIG*>(m_iC->Get(DATA_ITEM::ModuleConfiguation));
		}	
		static ProtobufMessage::PROCESS_CONFIG* GetProcessConfig(){
			return TYPE_TRANS<ProtobufMessage::PROCESS_CONFIG*>(m_iC->Get(DATA_ITEM::ModuleConfiguation));
		}	
		
		static CLog*	GetLog(){
			return TYPE_TRANS<CLog*>(m_iC->Get(DATA_ITEM::Logger));
		}
	public:
		static IControl* m_iC;
	};
}

#endif