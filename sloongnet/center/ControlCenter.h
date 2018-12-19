#pragma once


#include "IObject.h"

namespace Sloong
{
	class CNetworkCenter;
	class CLuaProcessCenter;
	class CServerConfig;
	class CControlCenter : IObject
	{
	public:
		CControlCenter();
		~CControlCenter();

		void Initialize(IMessage* iMsg);
		void Run(SmartEvent event);
		void Exit(SmartEvent event);
		void OnReceivePackage(SmartEvent event);
		void OnSocketClose(SmartEvent event);
	protected:
		unique_ptr<CNetworkCenter> m_pNetwork;
		unique_ptr<CLuaProcessCenter> m_pProcess;
		CServerConfig* m_pConfig;
	};
}

