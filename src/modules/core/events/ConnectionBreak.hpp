/*** 
 * @Author: Chuanbin Wang
 * @Date: 2020-05-14 17:43:08
 * @LastEditTime: 2020-07-24 16:03:12
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/events/ConnectionBreak.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: ConnectionBreakEvent object
 */


#pragma once
#include "NormalEvent.hpp"

namespace Sloong
{
	namespace Events
	{
		class ConnectionBreakEvent : public NormalEvent
		{
		public:
			ConnectionBreakEvent(int64_t session):NormalEvent( EVENT_TYPE::ConnectionBreak ){
				m_SessionID = session;
			}
			virtual	~ConnectionBreakEvent(){}

			inline int64_t GetSessionID() { return m_SessionID; }
		protected:
			int64_t m_SessionID;
		};
	}	
}