#ifndef SLOONGWALLUS_H
#define SLOONGWALLUS_H

#include<mutex>
#include<condition_variable>

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
		void ProcessEvent(int id, _stRecvInfo* info, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc);
		void ProcessEventList(int id, queue<_stRecvInfo>* pList, mutex& oLock, int sock, int nPriorityLevel, CLuaPacket* pUserInfo, CEpollEx* pEpoll, CMsgProc* pMsgProc);

	public:// static function
		static void* HandleEventWorkLoop(void* pParam);
		
	protected:
		int m_sockServ;
		int* m_sockClient;
		int m_nPriorityLevel;
        CServerConfig* m_pConfig;
		CEpollEx* m_pEpoll;
		CMsgProc* m_pMsgProc;
		CLog* m_pLog;
        mutex m_oEventMutex;
        condition_variable m_oEventCV;
		bool	m_bIsRunning;
	};

}



#endif //SLOONGWALLUS_H
