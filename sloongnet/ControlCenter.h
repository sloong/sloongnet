#pragma once
#include "IMessage.h"
#include "IData.h"
#include <condition_variable>
#include <memory>
using namespace std;
namespace Sloong
{
	using namespace Interface;
	class CEpollEx;
	class CLuaProcessCenter;
	class CGlobalFunction;
	class CServerConfig;
	class CControlCenter
	{
	public:
		CControlCenter();
		~CControlCenter();

		void Initialize(IMessage* iM,IData* iData);
		void Run();
		void Exit();

		void OnReceivePackage(IEvent* evt);
		void OnSocketClose(IEvent* evt);

	public: 
		static LPVOID EventHandler(LPVOID t,LPVOID object);

	protected:
		IMessage * m_iM;
		IData* m_iData;
		CLog*	m_pLog;
		CEpollEx* m_pEpoll;
		CLuaProcessCenter* m_pProcess;
		CGlobalFunction* m_pGFunc;
		CServerConfig* m_pConfig;
	};
}

