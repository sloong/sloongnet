#ifndef SLOONGWALLUS_H
#define SLOONGWALLUS_H

namespace Sloong
{
	namespace Universal
	{
		class CThreadPool;
		class CLuaPacket;
	}
	using namespace Universal;

	class CServerConfig;
	class CEpollEx;
	class CMsgProc;
	struct _stRecvInfo;
	class SloongWallUS
	{
	public:
		SloongWallUS();
		~SloongWallUS();

		void Initialize(CServerConfig* config);
		void Run();
		void Exit();

	public:// static function
		static void* HandleEventWorkLoop(void* pParam);
		static void ProcessEvent(int id, _stRecvInfo* info, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc);
		static void ProcessEventList(int id, queue<_stRecvInfo>* pList, mutex& oLock, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc);

	protected:
		int m_sockServ;
		int* m_sockClient;
		int m_nPriorityLevel;
        CServerConfig* m_pConfig;
		CEpollEx* m_pEpoll;
		CMsgProc* m_pMsgProc;
		CLog* m_pLog;
		sem_t m_oSem; 
		bool	m_bIsRunning;
	};

}



#endif //SLOONGWALLUS_H
