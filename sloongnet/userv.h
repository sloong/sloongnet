#ifndef SLOONGWALLUS_H
#define SLOONGWALLUS_H

namespace Sloong
{
	namespace Universal
	{
		class CThreadPool;
	}
	using namespace Universal;

	class CServerConfig;
	class CEpollEx;
	class CMsgProc;
	class SloongWallUS
	{
	public:
		SloongWallUS();
		~SloongWallUS();

		void Initialize(CServerConfig* config);
		void Run();

	public:// static function
		static void* HandleEventWorkLoop(void* pParam);

	protected:
		int m_sockServ;
		int* m_sockClient;
		CEpollEx* m_pEpoll;
		CMsgProc* m_pMsgProc;
		CThreadPool* m_pThreadPool;
		CLog* m_pLog;
	};

}



#endif //SLOONGWALLUS_H
