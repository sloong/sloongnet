/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-08-20 14:14:17
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/events/SendPackage.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */

#pragma once
#include "NormalEvent.hpp"
#include "package.hpp"
#include "protocol/core.pb.h"
namespace Sloong
{
	namespace Events
	{
		class SendPackageEvent : public NormalEvent, public std::enable_shared_from_this<SendPackageEvent>
		{
		public:
			SendPackageEvent(uint64_t id):NormalEvent(EVENT_TYPE::SendPackage){
				m_ConnectionHashCode = id;
			}
			virtual	~SendPackageEvent(){}

			inline void SetCallbackFunc(std::function<void(IEvent*,Package*)> p){ m_pCallback = p; }
			inline void CallCallbackFunc(Package* p){ 
				if(m_pCallback) 
					m_pCallback(this,p); 
			}
			inline bool HaveCallbackFunc(){ return m_pCallback != nullptr; }
			
			void SetRequest( uint64_t sender, uint64_t serialnumber, int32_t priority, int32_t func, string content, DataPackage_PackageType type = DataPackage_PackageType::DataPackage_PackageType_NormalPackage)
			{
				m_pData = make_unique<Package>();
				m_pData->set_sender(sender);
				m_pData->set_type(type);
				m_pData->set_function(func);
				m_pData->set_content(content);
				m_pData->set_priority(priority);
				m_pData->set_id(serialnumber);
				m_pData->set_sessionid(m_ConnectionHashCode);
			}

			inline Package* GetDataPackage()
			{
				return m_pData.get();
			}

			inline unique_ptr<Package> MoveDataPackage()
			{
				return std::move(m_pData);
			}

			inline uint64_t GetConnectionHashCode(){ return m_ConnectionHashCode; }

			CResult SyncCall( IControl* ic, int timeout )
			{
				auto response_str = make_shared<string>();
				auto result = make_shared<ResultType>(ResultType::Invalid);
				auto sync = make_shared<EasySync>();
				SetCallbackFunc([result, sync, response_str](IEvent *event, Package *pack) {
					(*result) = pack->result();
					(*response_str) = pack->content();
					sync->notify_one();
				});
				ic->CallMessage( shared_from_this() );

				if( timeout > 0 )
				{
					if (!sync->wait_for(timeout))
						return CResult::Make_Error("Timeout");
				}
				else
				{
					sync->wait();
				}
				
				return CResult(*result, *response_str);
			}
			
		protected:
			uint64_t m_ConnectionHashCode = 0;
			unique_ptr<Package> m_pData = nullptr;
			std::function<void(IEvent*,Package*)>		m_pCallback = nullptr;
		};

	}
}

