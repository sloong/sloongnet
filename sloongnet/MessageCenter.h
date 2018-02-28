#pragma once
#include "defines.h"
#include <queue>
#include <mutex>
#include <map>
#include <memory>
#include <condition_variable>
#include "IMessage.h"
#include "structs.h"
namespace Sloong
{
	using namespace Interface;
	class CMessageCenter : public IMessage
	{
	public:
		CMessageCenter();
		~CMessageCenter();

		void Initialize(int nWorkLoopNum, int ThreadPoolNum);

		void SendMessage(MSG_TYPE msgType);
		void SendMessage(IEvent* evt);
		void CallMessage(MSG_TYPE msgType, void* msgParams);

		void RegisterEvent(MSG_TYPE t);
		void RegisterEventHandler(MSG_TYPE t, void* object, LPCALLBACK2FUNC func);

		void Run();
		void Exit();

	protected:
		static void* MessageWorkLoop(void* param);

	protected:
		map<MSG_TYPE, vector<HandlerItem>> m_oMsgHandlerList;
		queue<IEvent*> m_oMsgList;
		mutex m_oWorkLoopMutex;
		condition_variable m_oWrokLoopCV;
		mutex m_oMsgListMutex;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;
	};
}

