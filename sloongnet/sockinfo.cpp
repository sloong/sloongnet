#include "sockinfo.h"
#include <univ/luapacket.h>
using namespace Sloong;
using namespace Sloong::Universal;

CSockInfo::CSockInfo()
{
	m_pUserInfo = new CLuaPacket();
}

CSockInfo::~CSockInfo()
{
	SAFE_DELETE(m_pUserInfo);
}
