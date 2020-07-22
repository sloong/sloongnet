#pragma once
#include "NormalEvent.hpp"

namespace Sloong
{
	namespace Events
	{
		class CExtendEvent : public NormalEvent
		{
		public:
			CExtendEvent(){}
			virtual ~CExtendEvent(){}

			void SetEvent(EVENT_TYPE t);
			EVENT_TYPE GetEvent();

			void SetParams(SMARTER p){
				m_pParams = p;
			}
			SMARTER GetParams(){
				return m_pParams;
			}

			void SetMessage(string str);
			string GetMessage();

			void SetCallbackFunc(LPSMARTFUNC func){
				m_pCallbackFunc = func;
			}
			LPSMARTFUNC GetCallbackFunc(){
				return m_pCallbackFunc;
			}
			void CallCallbackFunc(SMARTER pParams){
				if( m_pCallbackFunc )
					(*m_pCallbackFunc)(pParams);
			}

			LPVOID GetHandler(){
				return m_pHanlderObj;
			}
			void SetHandler(LPVOID obj){
				m_pHanlderObj = obj;
			}
		protected:
			LPSMARTFUNC m_pCallbackFunc = nullptr;
			SMARTER m_pParams = nullptr;
			EVENT_TYPE m_emType;
			LPVOID m_pHanlderObj;
			string m_strMessage;
		};
	}	
}

