#pragma once
#include "NormalEvent.h"
#include "main.h"
namespace Sloong
{
	class CDataTransPackage;
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

			CLuaPacket* GetUserInfo();
			void SetUserInfo(CLuaPacket* info);

			int GetPriority() { return m_nPriority; }
			void SetPriority(int n) { m_nPriority = n; }

			shared_ptr<CDataTransPackage> GetRecvPackage() { return m_pData; };
			void SetRecvPackage(shared_ptr<CDataTransPackage> data) { m_pData = data; };
		protected:
			int m_nSocketID;
			CLuaPacket* m_pInfo;
			int m_nPriority;
			shared_ptr<CDataTransPackage> m_pData;
		};

	}
}

