/*** 
 * @Author: Chuanbin Wang
 * @Date: 2020-05-14 17:43:08
 * @LastEditTime: 2020-07-24 19:29:44
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/events/ConnectionBreak.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: ConnectionBreakedEventn object
 */


#pragma once
#include "NormalEvent.hpp"

namespace Sloong
{
	namespace Events
	{
		class ConnectionBreakedEventn : public NormalEvent
		{
		public:
			ConnectionBreakedEventn(int64_t session):NormalEvent( EVENT_TYPE::ConnectionBreaked ){
				m_SessionID = session;
			}
			virtual	~ConnectionBreakedEventn(){}

			inline int64_t GetSessionID() { return m_SessionID; }
		protected:
			int64_t m_SessionID;
		};
	}	
}