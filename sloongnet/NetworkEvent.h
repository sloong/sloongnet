#pragma once
#include "NormalEvent.h"
namespace Sloong
{
	class CSockInfo;
	namespace Events
	{
		class CNetworkEvent : public CNormalEvent
		{
		public:
			CNetworkEvent();
			CNetworkEvent(MSG_TYPE t){ m_emType = t; }
			virtual	~CNetworkEvent();

			int GetSocketID() { return m_nSocketID; }
			void SetSocketID(int id) { m_nSocketID = id; }

			CSockInfo* GetSocketInfo();
			void SetSocketInfo(CSockInfo* info);

			int GetPriority() { return m_nPriority; }
			void SetPriority(int n) { m_nPriority = n; }

			RECVINFO* GetRecvPackage() { return &m_oInfo; };
			void SetRecvPackage(RECVINFO info) { m_oInfo = info; };
		protected:
			int m_nSocketID;
			CSockInfo* m_pInfo;
			int m_nPriority;
			RECVINFO m_oInfo;
		};

	}
}

