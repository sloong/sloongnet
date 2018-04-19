#pragma once

#include "defines.h"
#include "IEvent.h"
namespace Sloong
{
	namespace Interface
	{
		class IMessage
		{
		public:
			virtual void SendMessage(MSG_TYPE msgType) = 0;
			virtual void SendMessage(IEvent* evt) = 0;
			virtual void CallMessage(MSG_TYPE msgType, void* msgParams) = 0;

			virtual void RegisterEvent(MSG_TYPE t) = 0;
			virtual void RegisterEventHandler(MSG_TYPE t, void* object, LPCALLBACK2FUNC func) = 0;
		};
	}
}

