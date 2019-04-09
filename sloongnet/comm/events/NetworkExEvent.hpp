#pragma once
#include "NormalEvent.hpp"
#include "ExtendEvent.h"
#include "main.h"
namespace Sloong
{
	class CDataTransPackage;
	namespace Events
	{
		class CNetworkExEvent : public CNormalEvent, public CExtendEvent
		{
		public:
			CNetworkExEvent(EVENT_TYPE t){ m_emType = t; }
			virtual	~CNetworkExEvent(){}

			inline int GetSocketID() { return m_nSocketID; }
			inline void SetSocketID(int id) { m_nSocketID = id; }

			inline CLuaPacket* GetUserInfo(){return m_pInfo;}
			inline void SetUserInfo(CLuaPacket* info){m_pInfo = info;}

			inline shared_ptr<CDataTransPackage> GetDataPackage() { return m_pData; };
			inline void SetDataPackage(shared_ptr<CDataTransPackage> data) { m_pData = data; };
		protected:
			int m_nSocketID;
			CLuaPacket* m_pInfo;
			shared_ptr<CDataTransPackage> m_pData;
		};

	}
}
