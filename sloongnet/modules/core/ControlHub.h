/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-16 16:26:29
 * @Description: file content
 */
#ifndef CONTROL_HUB_H
#define CONTROL_HUB_H

#include "IEvent.h"
#include "IControl.h"
#include "core.h"
namespace Sloong
{
	class CControlHub : public IControl
	{
	public:
		// Always return true
		CResult Initialize(int);

		void Run();
		void Exit();

		// Message
		void SendMessage(EVENT_TYPE);
		void SendMessage(UniqueEvent);

		void CallMessage(UniqueEvent);

		void RegisterEvent(EVENT_TYPE t);
		void RegisterEventHandler(EVENT_TYPE, MsgHandlerFunc);
		void RegisterEventHook(EVENT_TYPE, MsgHookFunc);

		void MessageWorkLoop();

		// Data
		bool Add(DATA_ITEM, void *);
		void *Get(DATA_ITEM);

		template <typename T>
		T GetAs(DATA_ITEM item)
		{
			T tmp = static_cast<T>(Get(item));
			assert(tmp);
			return tmp;
		}

		bool Remove(DATA_ITEM);

		bool AddTemp(const string &, void *);
		void *GetTemp(const string &);

	protected:
		// Data
		map<DATA_ITEM, void *> m_oDataList;
		map<string, void *> m_oTempDataList;
		// Message
		map<EVENT_TYPE, vector<MsgHandlerFunc>> m_oMsgHandlerList;
		map<EVENT_TYPE, MsgHookFunc> m_listMsgHook;
		queue_ex<UniqueEvent> m_oMsgList;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;
		CEasySync m_oSync;
	};
} // namespace Sloong

#endif