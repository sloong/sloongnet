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
		// Add DATE_TIME to dictory.
		virtual void Add(DATA_ITEM, void *) = 0;
		virtual void *Get(DATA_ITEM) = 0;
		virtual void Remove(DATA_ITEM) = 0;
		virtual void AddTempString(const string &, const string &) = 0;
		virtual void AddTempObject(const string &, const void *, int) = 0;
		virtual void AddTempBytes(const string &, unique_ptr<char[]> &, int) = 0;

		// Get temp string, the param is key.
		// If key not exist, return empty string
		// If exist, return the value and remove from dictory.
		virtual string GetTempString(const string &) = 0;
		virtual void *GetTempObject(const string &, int *) = 0;
		virtual unique_ptr<char[]> GetTempBytes(const string &, int *) = 0;
		virtual bool ExistTempBytes(const string &key)=0;
		virtual bool ExistTempObject(const string &key)=0;
		virtual bool ExistTempString(const string &key)=0;
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