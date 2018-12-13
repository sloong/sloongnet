#pragma once

#include "main.h"
#include "IEvent.h"
namespace Sloong
{
	namespace Interface
	{
		typedef std::function<void(shared_ptr<IEvent>)> MsgHandlerFunc;
		
		class IMessage
		{
		public:
			virtual void SendMessage(MSG_TYPE msgType) = 0;
			virtual void SendMessage(SmartEvent evt) = 0;
			virtual void CallMessage(MSG_TYPE msgType, void* msgParams) = 0;

			virtual void RegisterEvent(MSG_TYPE t) = 0;
			virtual void RegisterEventHandler(MSG_TYPE t, MsgHandlerFunc func) = 0;
		};
	}
}

