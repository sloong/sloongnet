#pragma once

#include "NetworkEvent.h"
namespace Sloong
{
	namespace Events
	{
		class CSendMessageEvent : public CNetworkEvent
		{
		public:
			CSendMessageEvent();
			CSendMessageEvent(int nSocket, int nPriority, long long swift);
			virtual ~CSendMessageEvent() {}

			long long GetSwift() { return m_llSwift; }
			void SetSwift(long long swift) { m_llSwift = swift; }

			const char* GetSendExData() { return m_pExData; }
			int GetSendExDataSize() { return m_nExDataLength; }
			void SetSendExData(const char* ex, int len) { m_pExData = ex; m_nExDataLength = len; }

		protected:
			long long m_llSwift;
			const char* m_pExData = nullptr;
			int m_nExDataLength = 0;
		};

	}
}

