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
		/// Register a connected socket to network hub. 
		/// when it's done, will call the callback function, with the connection id for network hub.
		/// if you need do something when the socket is connected, set the ReconnectCallback function.
		class RegisterConnectionEvent : public NormalEvent
		{
		public:
			RegisterConnectionEvent(const string& address, int port):NormalEvent( EVENT_TYPE::RegisterConnection ){
                Address = address;
                Port = port;
            }
			virtual	~RegisterConnectionEvent(){}

			inline void SetCallbackFunc(std::function<void(IEvent*,uint64_t)> p){ m_pCallback = p; }
			inline void CallCallbackFunc(uint64_t id){ 
				if(m_pCallback) 
					m_pCallback(this,id); 
			}

			inline void EnableReconnectCallback(std::function<void(uint64_t,int,int)> p ){ m_pReconnectCallback = p; }
			inline bool HaveReconnectCallback(){ return m_pReconnectCallback != nullptr ;}
			inline std::function<void(uint64_t,int,int)> MoveReconnectCallbackFunc(){ return move(m_pReconnectCallback); }

			inline bool HaveCallbackFunc(){ return m_pCallback != nullptr; }

			inline const string& GetAddress() { return Address; }
			inline int GetPort() { return Port; };
		protected:
			string Address;
            int Port;
			std::function<void(IEvent*,uint64_t)> m_pCallback = nullptr;
			std::function<void(uint64_t,int,int)> m_pReconnectCallback = nullptr;
		};

	}
}

