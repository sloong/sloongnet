#ifndef CONTROL_HUB_H
#define CONTROL_HUB_H

#include "IEvent.h"
#include "IControl.h"

namespace Sloong
{
    class CControlHub : public IControl
    {
	public:
		// Always return true
		bool Initialize(int);

		void Run();
		void Exit();

        // Message
        void SendMessage(EVENT_TYPE msgType);
		void SendMessage(SmartEvent evt);

		void RegisterEvent(EVENT_TYPE t);
		void RegisterEventHandler(EVENT_TYPE t, MsgHandlerFunc func);

        void MessageWorkLoop(SMARTER param);

        // Data
        bool Add(DATA_ITEM item, void* object);
		void* Get(DATA_ITEM item);

		template<typename T>
		T GetAs(DATA_ITEM item) 
		{
			T tmp = static_cast<T>(Get(item));
			assert(tmp);
			return tmp;
		}

		bool Remove(DATA_ITEM item);

		bool AddTemp(string name, void* object);
		void* GetTemp(string name);

    protected:
         // Data
		map<DATA_ITEM, void*> m_oDataList;
		map<string, void*> m_oTempDataList;
        // Message
        map<EVENT_TYPE, vector<MsgHandlerFunc>> m_oMsgHandlerList;
		queue<shared_ptr<IEvent>> m_oMsgList;
		mutex m_oMsgListMutex;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;
		CEasySync	m_oSync;
    };
}

#endif