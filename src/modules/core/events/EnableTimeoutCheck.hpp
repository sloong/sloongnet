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
		// the unit as second
		class EnableTimeoutCheckEvent : public NormalEvent
		{
		public:
			EnableTimeoutCheckEvent(uint64_t time, uint64_t interval):NormalEvent( EVENT_TYPE::EnableTimeoutCheck ){
				m_TimeoutTime = time;
				m_CheckInterval = interval;
			}
			virtual	~EnableTimeoutCheckEvent(){}

			inline uint64_t GetTimeoutTime() { return m_TimeoutTime; }
			inline uint64_t GetCheckInterval() { return m_CheckInterval; }
		protected:
			uint64_t m_TimeoutTime;
			uint64_t m_CheckInterval;
		};
	}	
}