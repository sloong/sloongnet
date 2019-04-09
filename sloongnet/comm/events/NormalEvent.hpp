
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
			virtual ~CNormalEvent(){}

			void SetEvent(EVENT_TYPE t){
				m_emType = t;
			}
			EVENT_TYPE GetEvent(){
				return m_emType;
			}

			void SetMessage(string str){
				m_strMessage = str;
			}
			string GetMessage(){
				return m_strMessage;
			}
		protected:
			EVENT_TYPE m_emType;
			string m_strMessage;
		};
	}	
}