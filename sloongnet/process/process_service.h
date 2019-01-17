#ifndef SLOONGNET_PROCESS_SERVICE_H
#define SLOONGNET_PROCESS_SERVICE_H


#include "IEvent.h"
#include "IControl.h"
namespace Sloong
{
	class CControlHub;
	class CNetworkHub;
	class CLuaProcessCenter;
	class SloongNetProcess
	{
	public:
		SloongNetProcess();
		~SloongNetProcess();

		bool Initialize(int argc, char** args);

		void Run();
		void Exit();

		void OnReceivePackage(SmartEvent evt);
		void OnSocketClose(SmartEvent evt);
	protected:
		unique_ptr<CNetworkHub> m_pNetwork;
		unique_ptr<CControlHub> m_pControl;
		unique_ptr<CLuaProcessCenter> m_pProcess;

		unique_ptr<CLog>	m_pLog;
	};

}


#endif