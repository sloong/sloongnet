/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-05-14 17:43:08
 * @LastEditTime: 2020-07-24 19:29:44
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/events/ConnectionBreak.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: ConnectionBreakedEvent object
 */


#pragma once
#include "NormalEvent.hpp"

namespace Sloong
{
	namespace Events
	{
		class ConnectionBreakedEvent : public NormalEvent
		{
		public:
			ConnectionBreakedEvent(uint64_t session):NormalEvent( EVENT_TYPE::ConnectionBreaked ){
				m_SessionID = session;
			}
			virtual	~ConnectionBreakedEvent(){}

			inline uint64_t GetSessionID() { return m_SessionID; }

			inline void SetJustClose(bool b){ m_JustClose = b;}

			inline bool GetJustClose(){ return m_JustClose; }
		protected:
			uint64_t m_SessionID;

			bool m_JustClose;
		};
	}	
}