#pragma once
#include "IEvent.h"
#include "defines.h"

namespace Sloong
{
	using namespace Interface;
	namespace Events
	{
		class CNormalEvent : public IEvent
		{
		public:
			CNormalEvent();
			~CNormalEvent();

			void SetEvent(MSG_TYPE t);
			MSG_TYPE GetEvent();

			void SetParams(LPVOID p, bool bRelase  = true);
			LPVOID GetParams();

			void SetMessage(string str);
			string GetMessage();

			void SetCallbackFunc(LPCALLBACK2FUNC func);
			LPCALLBACK2FUNC GetCallbackFunc();
			void CallCallbackFunc(LPVOID pParams);

			void SetProcessingFunc(LPCALLBACK2FUNC func);
			LPCALLBACK2FUNC GetProcessingFunc();

			LPVOID GetHandler();
			void SetHandler(LPVOID obj);
		protected:
			LPCALLBACK2FUNC m_pCallbackFunc = nullptr;
			LPCALLBACK2FUNC m_pProcessingFunc = nullptr;
			LPVOID m_pParams = nullptr;
			bool m_bReleaseWhenShutdown = true;
			MSG_TYPE m_emType;
			LPVOID m_pObj;
			string m_strMessage;
		};
	}	
}