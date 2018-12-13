#pragma once

#include "IMessage.h"
#include "IData.h"
namespace Sloong
{
	using namespace Interface;
	
	class CMessageCenter : public IMessage
	{
	public:
		CMessageCenter();
		~CMessageCenter();

		void Initialize(IData* iData, int nWorkThreadNum);

		void SendMessage(MSG_TYPE msgType);
		void SendMessage(SmartEvent evt);
		void CallMessage(MSG_TYPE msgType, void* msgParams);

		void RegisterEvent(MSG_TYPE t);
		void RegisterEventHandler(MSG_TYPE t, MsgHandlerFunc func);

		void Run();
		void Exit();

		void MessageWorkLoop(SMARTER param);
	protected:
		

	protected:
		map<MSG_TYPE, vector<MsgHandlerFunc>> m_oMsgHandlerList;
		queue<shared_ptr<IEvent>> m_oMsgList;
		CSmartSync m_oSync;
		mutex m_oMsgListMutex;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;
		IData* m_iData;
	};
}

