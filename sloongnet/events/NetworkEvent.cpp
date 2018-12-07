#include "NetworkEvent.h"


using namespace Sloong::Events;
CNetworkEvent::CNetworkEvent()
{
}


CNetworkEvent::~CNetworkEvent()
{
}

shared_ptr<CSockInfo> Sloong::Events::CNetworkEvent::GetSocketInfo()
{
	return m_pInfo;
}

void Sloong::Events::CNetworkEvent::SetSocketInfo(shared_ptr<CSockInfo> info)
{
	m_pInfo = info;
}
