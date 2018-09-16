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
	m_pSendList = new queue<CSendInfo*>[nPriorityLevel]();
    m_pPrepareSendList = new queue<PRESENDINFO>;
	m_pCon = make_shared<lConnect>();
	m_pUserInfo = make_unique<CLuaPacket>();
}

CSockInfo::~CSockInfo()
{
	for (int i = 0; i < m_nPriorityLevel;i++)
	{
		while (!m_pSendList[i].empty())
		{
			CSendInfo* si = m_pSendList[i].front();
			m_pSendList[i].pop();
			SAFE_DELETE(si);
        }
	}
	SAFE_DELETE_ARR(m_pSendList);

    while (!m_pPrepareSendList->empty())
    {
        PRESENDINFO* psi = &m_pPrepareSendList->front();
        CSendInfo* si = psi->pSendInfo;
        m_pPrepareSendList->pop();
        SAFE_DELETE(si);
    }
    SAFE_DELETE(m_pPrepareSendList);
}
