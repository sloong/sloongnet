#include "sockinfo.h"
#include <univ/luapacket.h>
using namespace Sloong;
using namespace Sloong::Universal;

Sloong::CSockInfo::CSockInfo(int nPriorityLevel)
{
	if ( nPriorityLevel < 1 )
	{
		nPriorityLevel = 1;
	}
	m_nPriorityLevel = nPriorityLevel;
	m_pReadList = new queue<string>[nPriorityLevel]();
	m_pSendList = new queue<SENDINFO*>[nPriorityLevel]();
	m_pUserInfo = new CLuaPacket();
	m_nLastSentTags = -1;
}

CSockInfo::~CSockInfo()
{
	SAFE_DELETE(m_pUserInfo);
	SAFE_DELETE_ARR(m_pReadList);
	for (int i = 0; i < m_nPriorityLevel;i++)
	{
		while (m_pSendList[i].size())
		{
			SENDINFO* si = m_pSendList[i].front();
			m_pSendList[i].pop();
			SAFE_DELETE_ARR(si->pSendBuffer);
			SAFE_DELETE(si);
		}
	}
	SAFE_DELETE_ARR(m_pSendList);
}
