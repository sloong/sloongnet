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
	m_pSendList = new queue<shared_ptr<CSendInfo>>[nPriorityLevel]();
	m_pCon = make_shared<lConnect>();
	m_pUserInfo = make_unique<CLuaPacket>();
}

CSockInfo::~CSockInfo()
{
	for (int i = 0; i < m_nPriorityLevel;i++)
	{
		while (!m_pSendList[i].empty())
		{
			m_pSendList[i].pop();
        }
	}
	SAFE_DELETE_ARR(m_pSendList);

    while (!m_oPrepareSendList.empty())
    {
        m_oPrepareSendList.pop();
    }
}
