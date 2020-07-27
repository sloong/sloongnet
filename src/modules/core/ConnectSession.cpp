/*** 
 * @Author: Chuanbin Wang
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-27 15:18:18
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/ConnectSession.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: Connect session object. 
 */

#include "ConnectSession.h"

#include "IData.h"

Sloong::ConnectSession::ConnectSession()
{
	m_pSendList = new queue_ex<UniquePackage>[s_PriorityLevel]();
}

ConnectSession::~ConnectSession()
{
	if (m_pConnection)
		m_pConnection->Close();
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

void Sloong::ConnectSession::Initialize(IControl *iMsg, UniqueConnection conn)
{
	IObject::Initialize(iMsg);
	m_ActiveTime = time(NULL);
	m_pConnection = std::move(conn);
}

size_t g_max_package_size = 5 * 1024 * 1024;

bool IsOverflowPackage(DataPackage *pack)
{
	if (pack->extend().size() + pack->content().size() > g_max_package_size)
		return true;
	else
		return false;
}

bool IsBigPackage(DataPackage *pack)
{
	if (pack->extend().size() > 0 || pack->content().size() > 500000)
		return true;
	else
		return false;
}

ResultType Sloong::ConnectSession::SendDataPackage(UniquePackage pack)
{
	if (IsOverflowPackage(pack.get()))
	{
		m_pLog->Assert("The package size is to bigger, this's returned and replaced with an error message package.");
		pack->set_result(ResultType::Error);
		pack->set_content("The package size is to bigger.");
		pack->clear_extend();
	}

	
	pack->clear_reserved();
	m_pLog->Debug(Helper::Format("SEND>>>[%d]>>No[%llu]>>[%d]byte", m_pConnection->GetSocketID() , pack->id(), pack->ByteSize()));

	// if have exdata, directly add to epoll list.
	if (IsBigPackage(pack.get()) || m_pConnection->IsSending() || (m_bIsSendListEmpty == false && !m_oPrepareSendList.empty()) || m_oSockSendMutex.try_lock() == false)
	{
		AddToSendList(move(pack));
		return ResultType::Retry;
	}

	unique_lock<mutex> lck(m_oSockSendMutex, std::adopt_lock);
	// if code run here. the all list is empty. and no have exdata. try send message
	auto res = m_pConnection->SendPackage(std::move(pack));
	lck.unlock();
	if (res.GetResult() == ResultType::Succeed)
	{
		return ResultType::Succeed;
	}
	else if (res.GetResult() == ResultType::Error)
	{
		m_pLog->Warn(Helper::Format("Send data failed.[%s]", m_pConnection->m_strAddress.c_str()));
		return ResultType::Error;
	}
	else if (res.GetResult() == ResultType::Retry)
	{
		m_pLog->Verbos(Helper::Format("Send data done. wait next send time.[%d/%d]", m_pConnection->m_SendPackageSize, m_pConnection->m_SentSize));
		return ResultType::Retry;
	}
	else
	{
		m_pLog->Error(Helper::Format("Unintended result[%s] in SendDataPackage in SockInfo.", ResultType_Name(res.GetResult())));
		return ResultType::Error;
	}
}

void Sloong::ConnectSession::AddToSendList(UniquePackage pack)
{
	m_oPrepareSendList.push_move(std::move(pack));
	m_pLog->Debug(Helper::Format("Add send package to prepare send list. list size:[%d]", m_oPrepareSendList.size()));
	m_bIsSendListEmpty = false;
}

ReceivePackageListResult Sloong::ConnectSession::OnDataCanReceive()
{
	unique_lock<mutex> srlck(m_oSockReadMutex, std::adopt_lock);

	ReceivePackageList readList;
	bool bLoop = false;
	do
	{
		auto res = m_pConnection->RecvPackage();
		if (res.GetResult() == ResultType::Succeed)
		{
			auto package = res.MoveResultObject();

			if (IsOverflowPackage(package.get()))
			{
				m_pLog->Warn("The package size is to bigger.");
				AddToSendList(Package::MakeErrorResponse(package.get(),"The package size is to bigger."));
			}
			else
			{
				m_pLog->Debug(Helper::Format("RECV<<<[%d]<<No[%llu]<<[%d]byte", m_pConnection->GetSocketID(), package->id(), package->ByteSize()));
				package->mutable_reserved()->add_clocks(GetClock());
				package->mutable_reserved()->set_sessionid(m_pConnection->GetHashCode());

				if (package->priority() > s_PriorityLevel || package->priority() < 0)
				{
					m_pLog->Error(Helper::Format("Receive priority level error. the data is %d, the config level is %d. add this message to last list", package->priority(), s_PriorityLevel));
					package->set_priority(s_PriorityLevel);
				}
				if (package->hash().length() > 0)
				{
					m_pLog->Warn("Now don't support hash check, So the hash string will ignored.");
				}

				bLoop = true;
				m_ActiveTime = time(NULL);
				readList.push(std::move(package));
			}
		}
		else if (res.GetResult() == ResultType::Warning && bLoop)
		{
			// Receive data pageage length return 11(EAGAIN) error. so if in bLoop mode, this is OK.
			break;
		}
		else if (res.GetResult() == ResultType::Retry)
		{
			// Package is no receive done. need receive in next time.
			m_pLog->Verbos(res.GetMessage());
			break;
		}
		else if (res.GetResult() == ResultType::Error && !bLoop)
		{
			// Receive error, this connect weill be closed.
			return ReceivePackageListResult::Make_Error(res.GetMessage());
		}
		else
		{
			m_pLog->Error(Helper::Format("Unintended result[%s].Loop[%s].", ResultType_Name(res.GetResult()), bLoop ? "true" : "false"));
			break;
		}
	} while (bLoop);

	srlck.unlock();
	return ReceivePackageListResult::Make_OK(std::move(readList));
}

ResultType Sloong::ConnectSession::OnDataCanSend()
{
	ProcessPrepareSendList();
	return ProcessSendList();
}

void Sloong::ConnectSession::ProcessPrepareSendList()
{
	// progress the prepare send list first
	auto pack = m_oPrepareSendList.TryMovePop();
	while (pack != nullptr)
	{
		auto priority = pack->priority();
		m_pSendList[priority].push_move(std::move(pack));
		m_pLog->Debug(Helper::Format("Add package to send list[%d]. send list size[%d], prepare send list size[%d]",
									 priority, m_pSendList[priority].size(), m_oPrepareSendList.size()));

		pack = m_oPrepareSendList.TryMovePop();
	}
}

ResultType Sloong::ConnectSession::ProcessSendList()
{
	unique_lock<mutex> srlck(m_oSockSendMutex, std::adopt_lock);
	if (m_pConnection->IsSending())
	{
		m_pLog->Debug(Helper::Format("Start send package : AllSize[%d],Sent[%d]", m_pConnection->m_SendPackageSize, m_pConnection->m_SentSize));
		auto res = m_pConnection->SendPackage(nullptr);
		if (res.GetResult() == ResultType::Error)
		{
			m_pLog->Error(Helper::Format("Send data package error. close connect:[%s:%d]", m_pConnection->m_strAddress.c_str(), m_pConnection->m_nPort));
			return ResultType::Error;
		}
		else if (res.GetResult() == ResultType::Retry)
		{
			m_pLog->Debug(Helper::Format("Send data package done but not all data is send All[%d]:Sent[%d]. wait next write sign.", m_pConnection->m_SendPackageSize, m_pConnection->m_SentSize));
			return ResultType::Retry;
		}
		else
		{
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

		auto pack = list->TryMovePop();
		if (pack == nullptr)
			return ResultType::Retry;

		m_pLog->Debug(Helper::Format("Send new package, the list size[%d],Size[%d],", list->size(), pack->ByteSize()));
		auto res = m_pConnection->SendPackage(move(pack));

		if (res.GetResult() == ResultType::Error)
		{
			m_pLog->Error(Helper::Format("Send data package error. close connect:[%s:%d]", m_pConnection->m_strAddress.c_str(), m_pConnection->m_nPort));
			return ResultType::Error;
		}
		else if (res.GetResult() == ResultType::Retry)
		{
			m_pLog->Debug(Helper::Format("Send data package done but not all data is send All[%d]:Sent[%d]. wait next write sign.", m_pConnection->m_SendPackageSize, m_pConnection->m_SentSize));
			bTrySend = false;
			return ResultType::Retry;
		}
		else
		{
			bTrySend = true;
		}
	}
	return ResultType::Error;
}

/// 获取发送信息列表
// 首先判断上次发送标志，如果不为-1，表示上次的发送列表没有发送完成。直接返回指定的列表
// 如果为-1，表示需要发送新的列表。按照优先级逐级的进行寻找。
queue_ex<UniquePackage> *Sloong::ConnectSession::GetSendPackage()
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
