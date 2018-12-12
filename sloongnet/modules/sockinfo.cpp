#include "sockinfo.h"
#include <univ/luapacket.h>
#include "DataTransPackage.h"
#include "NetworkEvent.h"
using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;
Sloong::CSockInfo::CSockInfo(int nPriorityLevel, CLog* log, IMessage* msg)
{
	m_pLog = log;
	m_iMsg = msg;
	if ( nPriorityLevel < 1 )
	{
		nPriorityLevel = 1;
	}
	m_nPriorityLevel = nPriorityLevel;
	m_pSendList = new queue<shared_ptr<CDataTransPackage>>[nPriorityLevel]();
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


bool Sloong::CSockInfo::OnDataCanReceive()
{
	// The app is used ET mode, so should wait the mutex. 
	unique_lock<mutex> srlck(m_oSockReadMutex);

	// 已经连接的用户,收到数据,可以开始读入
	bool bLoop = false;
	do 
	{
		// 先读取消息长度
		char pLongBuffer[s_llLen + 1] = {0};
		int nRecvSize = m_pCon->Read( pLongBuffer, s_llLen, 2);
		if (nRecvSize < 0)
		{
			// 读取错误,将这个连接从监听中移除并关闭连接
			return false;
		}
		else if (nRecvSize == 0)
		{
			//由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读在这里就当作是该次事件已处理过。
			return true;
		}
		else
		{
			bLoop = true;
			long long dtlen = CUniversal::BytesToLong(pLongBuffer);
			// package length cannot big than 2147483648. this is max value for int.
			if (dtlen <= 0 || dtlen > 2147483648 || nRecvSize != s_llLen)
			{
				m_pLog->Error("Receive data length error.");
				return false;
			}

			auto package = make_shared<CDataTransPackage>(m_pCon);
			bool res = package->RecvPackage(dtlen);
			if ( !res )
			{
				return false;
			}

			//if (m_pConfig->m_oLogInfo.ShowReceiveMessage)
				m_pLog->Verbos(CUniversal::Format("RECV<<<[%d][%s]<<<%s",package->nSwiftNumber,package->strMD5, package->strMessage));

			
			// update the socket time
			m_ActiveTime = time(NULL);
			
			// Add the sock event to list
			auto event = make_shared<CNetworkEvent>(MSG_TYPE::ReveivePackage);
			
			event->SetSocketID(m_pCon->GetSocket());
			event->SetUserInfo(m_pUserInfo.get());
			event->SetPriority(package->nPriority);
			event->SetRecvPackage(package);
			m_iMsg->SendMessage(event);
		}
	}while (bLoop);

	srlck.unlock();
}

