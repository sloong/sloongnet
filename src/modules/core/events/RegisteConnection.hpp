/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 17:44:45
 * @Description: file content
 */
#pragma once
#pragma once

#include "NormalEvent.hpp"
namespace Sloong
{
	namespace Events
	{
		class RegisteConnectionEvent : public NormalEvent
		{
		public:
			RegisteConnectionEvent(const string& address, int port):NormalEvent( EVENT_TYPE::RegisteConnection ){
                Address = address;
                Port = port;
            }
			virtual	~RegisteConnectionEvent(){}

			inline void SetCallbackFunc(std::function<void(IEvent*,uint64_t)> p){ m_pCallback = p; }
			inline void CallCallbackFunc(uint64_t hashcode){ 
				if(m_pCallback) 
					m_pCallback(this,hashcode); 
			}
			inline bool HaveCallbackFunc(){ return m_pCallback != nullptr; }

			inline const string& GetAddress() { return Address; }
			inline int GetPort() { return Port; };
		protected:
			string Address;
            int Port;
			std::function<void(IEvent*,uint64_t)> m_pCallback = nullptr;
		};

	}
}

