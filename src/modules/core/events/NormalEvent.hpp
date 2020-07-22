/*
 * @Author: WCB
 * @Date: 2020-05-14 17:43:08
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 17:45:39
 * @Description: file content
 */

#pragma once
#include "IEvent.h"

namespace Sloong
{
	namespace Events
	{
		class CNormalEvent : public IEvent
		{
		public:
			CNormalEvent(){}
			CNormalEvent(EVENT_TYPE t):m_emType(t){}
			virtual ~CNormalEvent(){}

			inline void SetEvent(EVENT_TYPE t){ m_emType = t; }
			inline EVENT_TYPE GetEvent(){ return m_emType; }

			inline void SetMessage(string str){ m_strMessage = str; }
			inline string GetMessage(){ return m_strMessage; }
		protected:
			EVENT_TYPE m_emType;
			string m_strMessage;
		};
	}	
}