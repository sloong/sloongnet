#include "sockinfo.h"
#include "DataTransPackage.h"
#include "NetworkEvent.hpp"
#include "IData.h"
using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;
Sloong::CSockInfo::CSockInfo()
{
	m_pSendList = new queue_ex<UniqueTransPackage>[s_PriorityLevel]();
	m_pCon = make_unique<EasyConnect>();
}

CSockInfo::~CSockInfo()
{
	m_pCon->Close();
	for (int i = 0; i < s_PriorityLevel; i++)
	{
		while (!m_pSendList[i].empty())
		{
			m_pSendList[i].pop_move();
		}
	}
	SAFE_DELETE_ARR(m_pSendList);

	while (!m_oPrepareSendList.empty())
	{
		m_oPrepareSendList.pop_move();
	}
}

void Sloong::CSockInfo::Initialize(IControl *iMsg, int sock, LPVOID ctx)
{
	IObject::Initialize(iMsg);
	m_ActiveTime = time(NULL);
	m_pCon->Initialize(sock, ctx);
}

ResultType Sloong::CSockInfo::SendDataPackage(UniqueTransPackage pack)
{
	if (pack->GetConnection() == nullptr)
		pack->SetConnection(this->m_pCon.get());
	// if have exdata, directly add to epoll list.
	if (pack->IsBigPackage() || m_pSendingPackage != nullptr || (m_bIsSendListEmpty == false && !m_oPrepareSendList.empty()) || m_oSockSendMutex.try_lock() == false)
	{
		AddToSendList(std::move(pack));
		return ResultType::Retry;
	}

	unique_lock<mutex> lck(m_oSockSendMutex, std::adopt_lock);
	// if code run here. the all list is empty. and no have exdata. try send message
	m_pSendingPackage = std::move(pack);
	auto res = m_pSendingPackage->SendPackage();
	lck.unlock();
	if (res == ResultType::Succeed)	{
		m_pSendingPackage = nullptr;
		return ResultType::Succeed;
	}else if( res == ResultType::Error)
	{
		m_pLog->Warn(Helper::Format("Send data failed.[%s]", m_pCon->m_strAddress.c_str()));
		return ResultType::Error;
	}else if (res == ResultType::Retry)
	{
		m_pLog->Verbos(Helper::Format("Send data done. wait next send time.[%d/%d]", m_pSendingPackage->GetPackageSize(), m_pSendingPackage->GetSentSize()));
		return ResultType::Retry;
	}else{
		m_pLog->Error(Helper::Format("Unintended result[%s] in SendDataPackage in SockInfo.",ResultType_Name(res)));
		return ResultType::Error;
	}
}

void Sloong::CSockInfo::AddToSendList(UniqueTransPackage pack)
{
	m_oPrepareSendList.push_move(std::move(pack));
	m_pLog->Debug(Helper::Format("Add send package to prepare send list. list size:[%d]", m_oPrepareSendList.size()));
	m_bIsSendListEmpty = false;
}

ResultType Sloong::CSockInfo::OnDataCanReceive(queue<UniqueTransPackage> &readList)
{
	unique_lock<mutex> srlck(m_oSockReadMutex, std::adopt_lock);

	// 已经连接的用户,收到数据,可以开始读入
	bool bLoop = false;
	do
	{
		UniqueTransPackage pack = nullptr;
		if (m_pReceiving == nullptr)
			pack = make_unique<CDataTransPackage>(m_pCon.get());
		else
			pack = std::move(m_pReceiving);

		auto res = pack->RecvPackage();
		if (res == ResultType::Succeed)
		{
			bLoop = true;
			m_ActiveTime = time(NULL);
			readList.push(std::move(pack));
		} else if (res == ResultType::Warning && bLoop)
		{
			// Receive data pageage length return 11(EAGAIN) error. so if in bLoop mode, this is OK.
			return ResultType::Succeed;
		}
		else if(res == ResultType::Retry)
		{
			m_pReceiving = std::move(pack);
			return res;
		}
		else if (res == ResultType::Error && !bLoop)
			// Receive error, this connect weill be closed.
			return ResultType::Error;
		else if (res == ResultType::Invalid)
		{
			// The data package is invalid(Hash check error.)
			auto event = make_unique<CNetworkEvent>(EVENT_TYPE::MonitorSendStatus);
			event->SetSocketID(m_pCon->GetSocketID());
			m_iC->SendMessage(std::move(event));
			AddToSendList(std::move(pack));
		}else
		{
			m_pLog->Error(Helper::Format("Unintended result[%s].Loop[%s].",ResultType_Name(res),bLoop?"true":"false"));
		}
	} while (bLoop);

	srlck.unlock();
	return ResultType::Succeed;
}

ResultType Sloong::CSockInfo::OnDataCanSend()
{
	ProcessPrepareSendList();
	return ProcessSendList();
}

void Sloong::CSockInfo::ProcessPrepareSendList()
{
	// progress the prepare send list first
	UniqueTransPackage pack = nullptr;
	while (m_oPrepareSendList.TryMovePop(pack))
	{
		auto priority = pack->GetPriority();
		m_pSendList[priority].push_move(std::move(pack));
		m_pLog->Debug(Helper::Format("Add package to send list[%d]. send list size[%d], prepare send list size[%d]",
									 priority, m_pSendList[priority].size(), m_oPrepareSendList.size()));
	}
}

ResultType Sloong::CSockInfo::ProcessSendList()
{
	unique_lock<mutex> srlck(m_oSockSendMutex, std::adopt_lock);
	if (m_pSendingPackage != nullptr)
	{
		auto res = m_pSendingPackage->SendPackage();
		if (res == ResultType::Error)
		{
			m_pLog->Error(Helper::Format("Send data package error. close connect:[%s:%d]", m_pCon->m_strAddress.c_str(), m_pCon->m_nPort));
			return ResultType::Error;
		}
		else if (res == ResultType::Retry)
		{
			m_pLog->Debug(Helper::Format("Send data package done but not all data is send All[%d]:Sent[%d]. wait next write sign.", m_pSendingPackage->GetPackageSize(), m_pSendingPackage->GetSentSize()));
			return ResultType::Retry;
		}
		else
		{
			m_pSendingPackage = nullptr;
		}
	}

	bool bTrySend = true;
	while (bTrySend)
	{
		auto list = GetSendPackage();
		if (list == nullptr)
		{
			m_pLog->Debug("All data is sent, the send info list is empty.");
			m_bIsSendListEmpty = true;
			return ResultType::Succeed;
		}

		while (list->TryMovePop(m_pSendingPackage))
		{
			m_pLog->Debug(Helper::Format("Send new package, the list size[%d]", list->size()));
			auto res = m_pSendingPackage->SendPackage();
			if (res == ResultType::Error)
			{
				m_pLog->Error(Helper::Format("Send data package error. close connect:[%s:%d]", m_pCon->m_strAddress.c_str(), m_pCon->m_nPort));
				return ResultType::Error;
			}
			else if (res == ResultType::Retry)
			{
				m_pLog->Debug(Helper::Format("Send data package done but not all data is send All[%d]:Sent[%d]. wait next write sign.", m_pSendingPackage->GetPackageSize(), m_pSendingPackage->GetSentSize()));
				bTrySend = false;
				return ResultType::Retry;
			}
			else
			{
				m_pSendingPackage = nullptr;
				bTrySend = true;
			}
		}
	}
	return ResultType::Error;
}

/// 获取发送信息列表
// 首先判断上次发送标志，如果不为-1，表示上次的发送列表没有发送完成。直接返回指定的列表
// 如果为-1，表示需要发送新的列表。按照优先级逐级的进行寻找。
queue_ex<UniqueTransPackage> *Sloong::CSockInfo::GetSendPackage()
{
	for (int i = 0; i < s_PriorityLevel; i++)
	{
		if (m_pSendList[i].empty())
			continue;
		else
		{
			m_pLog->Debug(Helper::Format("Send list, Priority level:%d", i));
			return &m_pSendList[i];
		}
	}
	return nullptr;
}
