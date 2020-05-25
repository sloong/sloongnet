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
	typedef unique_ptr<IEvent> UniqueEvent;
	typedef std::function<void(IEvent *)> MsgHandlerFunc;
	typedef std::function<void(UniqueEvent)> MsgHookFunc;
	class IControl
	{
	public:
		// Data
		virtual bool Add(DATA_ITEM, void *) = 0;
		virtual void *Get(DATA_ITEM) = 0;
		virtual bool Remove(DATA_ITEM) = 0;
		virtual bool AddTemp(const string &, void *) = 0;
		virtual void *GetTemp(const string &) = 0;
		// Message
		virtual void SendMessage(EVENT_TYPE) = 0;
		virtual void SendMessage(UniqueEvent) = 0;
		virtual void CallMessage(UniqueEvent) = 0;
		virtual void RegisterEvent(EVENT_TYPE) = 0;
		virtual void RegisterEventHandler(EVENT_TYPE, MsgHandlerFunc) = 0;
		virtual void RegisterEventHook(EVENT_TYPE, MsgHookFunc) = 0;
	};
} // namespace Sloong

#endif