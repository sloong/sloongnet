#include "NetworkEvent.h"


using namespace Sloong::Events;
CNetworkEvent::CNetworkEvent()
{
}


CNetworkEvent::~CNetworkEvent()
{
}

CLuaPacket* Sloong::Events::CNetworkEvent::GetUserInfo()
{
	return m_pInfo;
}

void Sloong::Events::CNetworkEvent::SetUserInfo(CLuaPacket* info)
{
	m_pInfo = info;
}
