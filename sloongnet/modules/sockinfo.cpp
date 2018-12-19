#include "sockinfo.h"
#include <univ/luapacket.h>
#include "DataTransPackage.h"
#include "NetworkEvent.h"
using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;
Sloong::CSockInfo::CSockInfo()
{
	m_pSendList = new queue<shared_ptr<CDataTransPackage>>[g_pConfig->m_nPriorityLevel]();
	m_pCon = make_shared<lConnect>();
	m_pUserInfo = make_unique<CLuaPacket>();
}

CSockInfo::~CSockInfo()
{
	for (int i = 0; i < g_pConfig->m_nPriorityLevel;i++)
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


void Sloong::CSockInfo::Initialize(IControl* iMsg, int sock, SSL_CTX* ctx)
{
	IObject::Initialize(iMsg);
	m_ActiveTime = time(NULL);
	m_pCon->Initialize(sock,ctx);
	m_pUserInfo->SetData("ip", m_pCon->m_strAddress);
	m_pUserInfo->SetData("port", CUniversal::ntos(m_pCon->m_nPort));
}

NetworkResult Sloong::CSockInfo::ResponseDataPackage(SmartPackage pack)
{	
	// if have exdata, directly add to epoll list.
	if (pack->IsBigPackage())
	{
		AddToSendList(pack);
		return NetworkResult::Retry;
	}
	else
	{
		// check the send list size. if all empty, try send message directly.
		if ((m_bIsSendListEmpty == false && !m_oPrepareSendList.empty()) || m_oSockSendMutex.try_lock() == false)
		{
			AddToSendList(pack);
			return NetworkResult::Retry;
		}
	}

	unique_lock<mutex> lck(m_oSockSendMutex, std::adopt_lock);
	// if code run here. the all list is empty. and no have exdata. try send message
	auto res = pack->SendPackage();
	if ( res == NetworkResult::Error )
	{
		// TODO: 这里应该对错误进行区分处理
		m_pLog->Warn(CUniversal::Format("Send data failed.[%s]", m_pCon->m_strAddress));//, m_pCon->G_FormatSSLErrorMsg(nMsgSend)));
		return NetworkResult::Error;
	}
	if (res == NetworkResult::Retry )
	{
		AddToSendList(pack);
		return NetworkResult::Retry;
	}
	lck.unlock();
	return NetworkResult::Succeed;
}


void Sloong::CSockInfo::AddToSendList(SmartPackage pack)
{
	unique_lock<mutex> lck(m_oPreSendMutex);
	m_oPrepareSendList.push(pack);
	m_pLog->Debug(CUniversal::Format("Add send package to prepare send list. list size:[%d]",m_oPrepareSendList.size()));
	m_bIsSendListEmpty = false;
}


NetworkResult Sloong::CSockInfo::OnDataCanReceive()
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
			return NetworkResult::Error;
		}
		else if (nRecvSize == 0)
		{
			//由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读在这里就当作是该次事件已处理过。
			return NetworkResult::Succeed;
		}
		else
		{
			bLoop = true;
			long long dtlen = CUniversal::BytesToLong(pLongBuffer);
			// package length cannot big than 2147483648. this is max value for int.
			if (dtlen <= 0 || dtlen > 2147483648 || nRecvSize != s_llLen)
			{
				m_pLog->Error("Receive data length error.");
				return NetworkResult::Error;
			}

			auto package = make_shared<CDataTransPackage>();
			package->Initialize(m_iMsg,m_pCon);
			auto res = package->RecvPackage(dtlen);
			if ( res == NetworkResult::Invalid ){
				AddToSendList(package);
			}else if( res == NetworkResult::Error ){
				return NetworkResult::Error;
			}

			// update the socket time
			m_ActiveTime = time(NULL);
			
			// Add the sock event to list
			auto event = make_shared<CNetworkEvent>(MSG_TYPE::ReveivePackage);
			
			event->SetSocketID(m_pCon->GetSocketID());
			event->SetUserInfo(m_pUserInfo.get());
			event->SetDataPackage(package);
			m_iMsg->SendMessage(event);
		}
	}while (bLoop);

	srlck.unlock();
}


NetworkResult Sloong::CSockInfo::OnDataCanSend()
{
	ProcessPrepareSendList();
	return ProcessSendList();
}



void Sloong::CSockInfo::ProcessPrepareSendList()
{
	// progress the prepare send list first
	if (!m_oPrepareSendList.empty())
	{
		unique_lock<mutex> prelck(m_oPreSendMutex);
		if (m_oPrepareSendList.empty())
		{
			return;
		}
		unique_lock<mutex> sendListlck(m_oSendListMutex);
		
		while (!m_oPrepareSendList.empty())
		{
			auto pack = m_oPrepareSendList.front();
			m_oPrepareSendList.pop();
			m_pSendList[pack->nPriority].push(pack);
			m_pLog->Debug(CUniversal::Format("Add send package to send list[%d]. send list size[%d], prepare send list size[%d]",
								pack->nPriority,m_pSendList[pack->nPriority].size(),m_oPrepareSendList.size()));
		}
		prelck.unlock();
		sendListlck.unlock();
	}
}



NetworkResult Sloong::CSockInfo::ProcessSendList()
{
	// when prepare list process done, do send operation.
	bool bTrySend = true;

	// 这里始终从list开始循环，保证高优先级的信息先被处理
	while (bTrySend)
	{
		unique_lock<mutex> lck(m_oSendListMutex);

		queue<shared_ptr<CDataTransPackage>>* list = nullptr;
		int sendTags = GetSendInfoList(list);
		if (list == nullptr){
			m_pLog->Error("Send info list empty, no need send.");
			break;
		}

		// if no find send info, is no need send anything , remove this sock from epoll.'
		auto si = GetSendInfo(list);
		if ( si != nullptr )
		{
			lck.unlock();
			unique_lock<mutex> ssend_lck(m_oSockSendMutex);
			int res =si->SendPackage();
			ssend_lck.unlock();
			if( res < 0){
				m_pLog->Error(CUniversal::Format("Send data package error. close connect:[%s:%d]",m_pCon->m_strAddress,m_pCon->m_nPort));
				return NetworkResult::Error;
			}else if( res == 0){
				m_pLog->Verbos("Send data package done. wait next write sign.");
				bTrySend = false;
				m_nLastSentTags = sendTags;
				return NetworkResult::Retry;
			}else{
				list->pop();
				m_nLastSentTags = -1;
				bTrySend = true;
			}
		}
	}
	return NetworkResult::Succeed;
}


/// 获取发送信息列表
// 首先判断上次发送标志，如果不为-1，表示上次的发送列表没有发送完成。直接返回指定的列表
// 如果为-1，表示需要发送新的列表。按照优先级逐级的进行寻找。
int Sloong::CSockInfo::GetSendInfoList( queue<shared_ptr<CDataTransPackage>>*& list )
{
	list = nullptr;
	// prev package no send end. find and try send it again.
	if (-1 != m_nLastSentTags)
	{
		m_pLog->Verbos(CUniversal::Format("Send prev time list, Priority level:%d", m_nLastSentTags));
		list = &m_pSendList[m_nLastSentTags];
		if( list->empty() )
			m_nLastSentTags = -1;
		else
			return m_nLastSentTags;
	}
	
	for (int i = 0; i < g_pConfig->m_nPriorityLevel; i++)
	{
		if (m_pSendList[i].empty())
			continue;
		else
		{
			list = &m_pSendList[i];
			m_pLog->Verbos(CUniversal::Format("Send list, Priority level:%d", i));
			return i;
		}
	}
	return -1;
}


shared_ptr<CDataTransPackage> Sloong::CSockInfo::GetSendInfo(queue<shared_ptr<CDataTransPackage>>* list)
{
	shared_ptr<CDataTransPackage> si = nullptr;
	while (si == nullptr)
	{
		if (!list->empty())
		{
			m_pLog->Verbos(CUniversal::Format("Get send info from list, list size[%d].", list->size()));
			si = list->front();
			if (si == nullptr)
			{
				m_pLog->Verbos("The list front is NULL, pop it and get next.");
				list->pop();
			}
		}
		else
		{
			// the send list is empty, so no need loop.
			m_pLog->Verbos("Send list is empty list. no need send message");
			break;
		}
	}
	if (si == nullptr)
	{
		if (m_nLastSentTags != -1)
		{
			m_pLog->Verbos("Current list no send message, clear the LastSentTags flag.");
			m_nLastSentTags = -1;
		}
		else
		{
			m_pLog->Verbos(CUniversal::Format("No message need send, remove socket[%d] from Epoll", m_pCon->GetSocketID()));
			m_bIsSendListEmpty = true;
		}
	}
	return si;
}