/*** 
 * @Author: Chuanbin Wang
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-27 14:58:28
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/events/SendPackage.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */

#pragma once
#include "NormalEvent.hpp"
#include "protocol/core.pb.h"
namespace Sloong
{
	namespace Events
	{
		class SendPackageEvent : public NormalEvent
		{
		public:
			SendPackageEvent(int64_t id):NormalEvent(EVENT_TYPE::SendPackage){
				m_ConnectionHashCode = id;
			}
			virtual	~SendPackageEvent(){}

			inline void SetCallbackFunc(std::function<void(IEvent*,DataPackage*)> p){ m_pCallback = p; }
			inline void CallCallbackFunc(DataPackage* p){ 
				if(m_pCallback) 
					m_pCallback(this,p); 
			}
			inline bool HaveCallbackFunc(){ return m_pCallback != nullptr; }
			
			void SetRequest( uint64_t sender, uint64_t serialnumber, int32_t priority, int32_t func, string content,  string extend = "", DataPackage_PackageType type = DataPackage_PackageType::DataPackage_PackageType_NormalPackage)
			{
				m_pData = make_unique<DataPackage>();
				m_pData->set_sender(sender);
				m_pData->set_type(type);
				m_pData->set_function(func);
				m_pData->set_content(content);
				m_pData->set_extend(extend);
				m_pData->set_priority(priority);
				m_pData->set_id(serialnumber);
				m_pData->mutable_reserved()->set_sessionid(m_ConnectionHashCode);
			}

			inline DataPackage* GetDataPackage()
			{
				return m_pData.get();
			}

			inline unique_ptr<DataPackage> MoveDataPackage()
			{
				return std::move(m_pData);
			}

			inline int64_t GetConnectionHashCode(){ return m_ConnectionHashCode; }
			
		protected:
			int64_t m_ConnectionHashCode = 0;
			unique_ptr<DataPackage> m_pData = nullptr;
			std::function<void(IEvent*,DataPackage*)>		m_pCallback = nullptr;
		};

	}
}

