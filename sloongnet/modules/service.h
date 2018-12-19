#ifndef SloongNetService_H
#define SloongNetService_H

#include "IEvent.h"

namespace Sloong
{
	class CServerConfig;
	class CControlCenter;
	class CDataCenter;
	class CMessageCenter;
	class SloongNetService
	{
	public:
		SloongNetService();
		~SloongNetService();

		void Initialize(CServerConfig* config);
		void Run();
		void Exit();
		
		void EventHandler(SmartEvent event);
		
	protected:
		unique_ptr<CControlCenter> m_pCC;
		unique_ptr<CDataCenter> m_pDC;
		unique_ptr<CMessageCenter> m_pMC;
		unique_ptr<CLog>	m_pLog;
		CEasySync	m_oSync;
		bool	m_bIsRunning;
	};

}



#endif //SloongNetService_H
