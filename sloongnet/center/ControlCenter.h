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
	class CServerConfig;
	class CControlCenter
	{
	public:
		CControlCenter();
		~CControlCenter();

		void Initialize(IMessage* iM,IData* iData);
		void Run(SmartEvent event);
		void Exit(SmartEvent event);
		void OnReceivePackage(SmartEvent event);
		void OnSocketClose(SmartEvent event);
	protected:
		IMessage * m_iM;
		IData* m_iData;
		CLog*	m_pLog;
		unique_ptr<CEpollEx> m_pEpoll;
		unique_ptr<CLuaProcessCenter> m_pProcess;
		CServerConfig* m_pConfig;
	};
}

