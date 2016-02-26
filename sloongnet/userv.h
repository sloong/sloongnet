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
	class SloongWallUS
	{
	public:
		SloongWallUS();
		~SloongWallUS();

		void Initialize(CServerConfig* config);
		void Run();

	public:// static function
		static void* HandleEventWorkLoop(void* pParam);
		static void ProcessEvent(int id, string& strMsg, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc);
		static void ProcessEventList(int id, queue<string>* pList, mutex& oLock, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc);

	protected:
		int m_sockServ;
		int* m_sockClient;
		int m_nPriorityLevel;
		int m_nSleepInterval;
        CServerConfig* m_pConfig;
		CEpollEx* m_pEpoll;
		CMsgProc* m_pMsgProc;
		CLog* m_pLog;
	};

}



#endif //SLOONGWALLUS_H
