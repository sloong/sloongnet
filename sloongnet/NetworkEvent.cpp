#include "NetworkEvent.h"


using namespace Sloong::Events;
CNetworkEvent::CNetworkEvent()
{
}


CNetworkEvent::~CNetworkEvent()
{
}

Sloong::CSockInfo* Sloong::Events::CNetworkEvent::GetSocketInfo()
{
	return m_pInfo;
}

void Sloong::Events::CNetworkEvent::SetSocketInfo(CSockInfo* info)
{
	m_pInfo = info;
}
