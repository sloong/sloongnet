/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:03:43
 * @Description: file content
 */
#pragma once
#include "NetworkEvent.hpp"
#include "protocol/core.pb.h"
namespace Sloong
{
	class CDataTransPackage;
	namespace Events
	{
		typedef std::function<CResult(IEvent*,CDataTransPackage*)> CallbackFunc;
		class CSendPackageEvent : public CNetworkEvent
		{
		public:
			CSendPackageEvent():CNetworkEvent(EVENT_TYPE::SendPackage){}
			virtual	~CSendPackageEvent(){}

			inline void SetCallbackFunc(CallbackFunc p){ m_pCallback = p; }
			inline CResult CallCallbackFunc(CDataTransPackage* p){ 
				if(m_pCallback) 
					return m_pCallback(this,p); 
				return CResult::Invalid();
			}
			
			void SetRequest( SOCKET target, string sender, int serialnumber, int priority, int func, string content,  string extend = "", DataPackage_PackageType type = DataPackage_PackageType::DataPackage_PackageType_RequestPackage)
			{
				m_pData.set_sender(sender);
				m_pData.set_type(type);
				m_pData.set_function(func);
				m_pData.set_content(content);
				m_pData.set_extend(extend);
				m_pData.set_prioritylevel(priority);
				m_pData.set_serialnumber(serialnumber);

				m_nSocketID = target;
			}

			DataPackage* GetDataPackage()
			{
				return &m_pData;
			}
		protected:
			DataPackage m_pData;
			CallbackFunc		m_pCallback = nullptr;
		};

	}
}

