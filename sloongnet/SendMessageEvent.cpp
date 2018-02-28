#include "SendMessageEvent.h"


Sloong::Events::CSendMessageEvent::CSendMessageEvent(int nSocket, int nPriority, long long swift)
{
	m_nSocketID = nSocket; 
	m_nPriority = nPriority; 
	m_llSwift = swift; 
	m_emType = SendMessage;
}
