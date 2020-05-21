#include "sockinfo.h"
#include <univ/luapacket.h>
#include "DataTransPackage.h"
#include "NetworkEvent.hpp"
#include "IData.h"
using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;
Sloong::CSockInfo::CSockInfo()
{
	m_pSendList = new queue_ex<SmartPackage>[s_PriorityLevel]();
	m_pCon = make_unique<EasyConnect>();
}

CSockInfo::~CSockInfo()
{
	m_pCon->Close();
	for (int i = 0; i < s_PriorityLevel;i++){
		while (!m_pSendList[i].empty()){
			m_pSendList[i].pop();
        }
	}
	SAFE_DELETE_ARR(m_pSendList);

    while (!m_oPrepareSendList.empty()){
        m_oPrepareSendList.pop();
    }
}


void Sloong::CSockInfo::Initialize(IControl* iMsg, int sock, SSL_CTX* ctx)
{
	IObject::Initialize(iMsg);
	m_ActiveTime = time(NULL);
	auto serv_config = IData::GetGlobalConfig();
	m_ReceiveTimeout = serv_config->receivetime();
	m_pCon->Initialize(sock,ctx);
}

ResultType Sloong::CSockInfo::SendDataPackage(SmartPackage pack)
{	
	// if have exdata, directly add to epoll list.
	if (pack->IsBigPackage()){
		AddToSendList(pack);
		return ResultType::Retry;
	}else{
		// check the send list size. if all empty, try send message directly.
		if ((m_bIsSendListEmpty == false && !m_oPrepareSendList.empty()) || m_oSockSendMutex.try_lock() == false){
			AddToSendList(pack);
			return ResultType::Retry;
		}
	}

	unique_lock<mutex> lck(m_oSockSendMutex, std::adopt_lock);
	// if code run here. the all list is empty. and no have exdata. try send message
	auto res = pack->SendPackage();
	if ( res == ResultType::Error ){
		// TODO: 这里应该对错误进行区分处理
		m_pLog->Warn(CUniversal::Format("Send data failed.[%s]", m_pCon->m_strAddress));//, m_pCon->G_FormatSSLErrorMsg(nMsgSend)));
		return ResultType::Error;
	}
	if (res == ResultType::Retry ){
		AddToSendList(pack);
		return ResultType::Retry;
	}
	lck.unlock();
	return ResultType::Succeed;
}


void Sloong::CSockInfo::AddToSendList(SmartPackage pack)
{
	unique_lock<mutex> lck(m_oPreSendMutex);
	m_oPrepareSendList.push(pack);
	m_pLog->Debug(CUniversal::Format("Add send package to prepare send list. list size:[%d]",m_oPrepareSendList.size()));
	m_bIsSendListEmpty = false;
}


ResultType Sloong::CSockInfo::OnDataCanReceive( queue<SmartPackage>& readList )
{
	if(m_oSockReadMutex.try_lock()==false) return ResultType::Invalid;
	unique_lock<mutex> srlck(m_oSockReadMutex,std::adopt_lock);

	// 已经连接的用户,收到数据,可以开始读入
	bool bLoop = false;
	do {
		auto package = make_shared<CDataTransPackage>(m_pCon.get());
		auto res = package->RecvPackage(m_ReceiveTimeout);
		if( !bLoop && res == ResultType::Error){
			// 读取错误,将这个连接从监听中移除并关闭连接
			return ResultType::Error;
		}else if( !bLoop && res == ResultType::Invalid){
			auto event = make_shared<CNetworkEvent>(EVENT_TYPE::MonitorSendStatus);
			event->SetSocketID(m_pCon->GetSocketID());
			m_iC->SendMessage(event);
			AddToSendList(package);
		}else if ( res == ResultType::Succeed ){
			bLoop = true;
			// update the socket time
			m_ActiveTime = time(NULL);
			readList.push(package);
		}else{
			//由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读在这里就当作是该次事件已处理过。
			// 或者是在第二次读取的时候，发生了错误，仍视为成功
			return ResultType::Succeed;
		}
	}while (bLoop);

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
	SmartPackage pack = nullptr;
	while ( m_oPrepareSendList.TryPop(pack) ){
		auto priority = pack->GetPriority();
		m_pSendList[priority].push(pack);
		m_pLog->Debug(CUniversal::Format("Add package to send list[%d]. send list size[%d], prepare send list size[%d]",
								priority,m_pSendList[priority].size(),m_oPrepareSendList.size()));
	}
}



ResultType Sloong::CSockInfo::ProcessSendList()
{
	if(m_oSockSendMutex.try_lock()==false) return ResultType::Invalid;
	unique_lock<mutex> srlck(m_oSockSendMutex,std::adopt_lock);

	if( m_pSendingPackage != nullptr )
	{
		auto res =m_pSendingPackage->SendPackage();
		if( res == ResultType::Error ){
			m_pLog->Error(CUniversal::Format("Send data package error. close connect:[%s:%d]",m_pCon->m_strAddress,m_pCon->m_nPort));
			return ResultType::Error;
		}else if( res == ResultType::Retry ){
			m_pLog->Debug(CUniversal::Format("Send data package done but not all data is send All[%d]:Sent[%d]. wait next write sign.", m_pSendingPackage->GetPackageSize(),m_pSendingPackage->GetSentSize()));
			return ResultType::Retry;
		}else{
			m_pSendingPackage = nullptr;
		}
	}

	bool bTrySend = true;
	while(bTrySend)
	{
		auto list = GetSendPackage();
		if( list == nullptr )
		{
			m_pLog->Debug("All data is sent, the send info list is empty.");
			m_bIsSendListEmpty = true;
			return ResultType::Succeed;
		}
		
		while( list->TryPop(m_pSendingPackage) )
		{
			m_pLog->Debug(CUniversal::Format("Send new package, the list size[%d]",list->size()));
			auto res =m_pSendingPackage->SendPackage();
			if( res == ResultType::Error ){
				m_pLog->Error(CUniversal::Format("Send data package error. close connect:[%s:%d]",m_pCon->m_strAddress,m_pCon->m_nPort));
				return ResultType::Error;
			}else if( res == ResultType::Retry ){
				m_pLog->Debug(CUniversal::Format("Send data package done but not all data is send All[%d]:Sent[%d]. wait next write sign.", m_pSendingPackage->GetPackageSize(),m_pSendingPackage->GetSentSize()));
				bTrySend = false;
				return ResultType::Retry;
			}else{
				m_pSendingPackage = nullptr;
				bTrySend = true;
			}
		}
	}
}


/// 获取发送信息列表
// 首先判断上次发送标志，如果不为-1，表示上次的发送列表没有发送完成。直接返回指定的列表
// 如果为-1，表示需要发送新的列表。按照优先级逐级的进行寻找。
queue_ex<SmartPackage>* Sloong::CSockInfo::GetSendPackage()
{
	for (int i = 0; i < s_PriorityLevel; i++)
	{
		if (m_pSendList[i].empty())
			continue;
		else
		{
			m_pLog->Debug(CUniversal::Format("Send list, Priority level:%d", i));
			return &m_pSendList[i];
		}
	}
	return nullptr;
}
