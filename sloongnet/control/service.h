#ifndef SloongNetService_H
#define SloongNetService_H

#include "IEvent.h"
#include "IControl.h"
namespace Sloong
{
	class CServerConfig;
	class CControlCenter;
	class CDataCenter;
	class CMessageCenter;
	class SloongNetService : IControl
	{
	public:
		SloongNetService();
		~SloongNetService();

		bool Initialize(int argc, char** args);

		void SendMessage(MSG_TYPE msgType);
		void SendMessage(SmartEvent evt);

		void RegisterEvent(MSG_TYPE t);
		void RegisterEventHandler(MSG_TYPE t, MsgHandlerFunc func);

		void Run();
		void Exit();

		void MessageWorkLoop(SMARTER param);

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
		
		void ExitEventHandler(SmartEvent event);
		
	protected:
		unique_ptr<CControlCenter> m_pCC;

		// Data
		map<DATA_ITEM, void*> m_oDataList;
		map<string, void*> m_oTempDataList;

		// Message
		map<MSG_TYPE, vector<MsgHandlerFunc>> m_oMsgHandlerList;
		queue<shared_ptr<IEvent>> m_oMsgList;
		mutex m_oMsgListMutex;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;

		unique_ptr<CLog>	m_pLog;
		CEasySync	m_oSync;
	};

}



#endif //SloongNetService_H
