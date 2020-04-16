/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-16 15:41:07
 * @Description: file content
 */
#ifndef SLOONGNET_INTERFACE_CONTROL_H
#define SLOONGNET_INTERFACE_CONTROL_H

#include "IEvent.h"
namespace Sloong
{
	typedef std::function<void(shared_ptr<IEvent>)> MsgHandlerFunc;
	
	class IControl
	{
	public:
		// Data 
		virtual bool Add(DATA_ITEM item, void* object) = 0;
		virtual void* Get(DATA_ITEM item) = 0;
		virtual bool Remove(DATA_ITEM item) = 0;
		virtual bool AddTemp(string name, void* object) = 0;
		virtual void* GetTemp(string name) = 0;
		// Message
		virtual void SendMessage(EVENT_TYPE msgType) = 0;
		virtual void SendMessage(SmartEvent evt) = 0;
		virtual void CallMessage(SmartEvent evt) = 0;
		virtual void RegisterEvent(EVENT_TYPE t) = 0;
		virtual void RegisterEventHandler(EVENT_TYPE t, MsgHandlerFunc func) = 0;
	};
}

#endif