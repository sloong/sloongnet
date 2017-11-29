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
	m_pReadList = new queue<RECVINFO>[nPriorityLevel]();
	m_pSendList = new queue<SENDINFO*>[nPriorityLevel]();
	m_pProcessMutexList = new mutex[nPriorityLevel]();
    m_pPrepareSendList = new queue<PRESENDINFO>;
	m_pUserInfo = new CLuaPacket();
	m_nLastSentTags = -1;
    m_bIsSendListEmpty = true;
	m_ssl = NULL;
}

CSockInfo::~CSockInfo()
{
	SAFE_DELETE(m_pUserInfo);
	SAFE_DELETE_ARR(m_pReadList);
	for (int i = 0; i < m_nPriorityLevel;i++)
	{
		while (!m_pSendList[i].empty())
		{
			SENDINFO* si = m_pSendList[i].front();
			m_pSendList[i].pop();
			SAFE_DELETE_ARR(si->pSendBuffer);
            SAFE_DELETE_ARR(si->pExBuffer);
			SAFE_DELETE(si);
        }
	}
	SAFE_DELETE_ARR(m_pSendList);

    while (!m_pPrepareSendList->empty())
    {
        PRESENDINFO* psi = &m_pPrepareSendList->front();
        SENDINFO* si = psi->pSendInfo;
        m_pPrepareSendList->pop();
        SAFE_DELETE_ARR(si->pSendBuffer);
        SAFE_DELETE_ARR(si->pExBuffer);
        SAFE_DELETE(si);
    }
    SAFE_DELETE(m_pPrepareSendList);
	SAFE_DELETE_ARR(m_pProcessMutexList);
}
