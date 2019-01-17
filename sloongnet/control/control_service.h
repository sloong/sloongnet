#ifndef SloongNetService_H
#define SloongNetService_H


#include "IEvent.h"
#include "IControl.h"
namespace Sloong
{
	class CNetworkHub;
	class CControlHub;
	class SloongNetService
	{
	public:
		SloongNetService();
		~SloongNetService();

		bool Initialize(int argc, char** args);

		void Run();
		void Exit();
		
		void OnReceivePackage(SmartEvent evt);
		void OnSocketClose(SmartEvent evt);
	protected:
		unique_ptr<CNetworkHub> m_pNetwork;
		unique_ptr<CControlHub> m_pControl;
		
		unique_ptr<CLog>	m_pLog;
	};

}



#endif //SloongNetService_H
