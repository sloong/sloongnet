#include "NetworkCenter.h"
#include "serverconfig.h"
#include "epollex.h"
#include "SendMessageEvent.h"
#include "sockinfo.h"

using namespace Sloong::Events;

Sloong::CNetworkCenter::CNetworkCenter()
{
    m_pEpoll = make_unique<CEpollEx>();
}

Sloong::CNetworkCenter::~CNetworkCenter()
{
	if (m_pCTX)
	{
		SSL_CTX_free(m_pCTX);
	}
}


void Sloong::CNetworkCenter::Initialize(IMessage* iMsg, IData* iData)
{
    IObject::Initialize(iMsg, iData);

    m_pConfig = (CServerConfig*)m_iData->Get(Configuation);

    m_pEpoll->Initialize(m_iMsg,m_iData);

    if (m_pConfig->m_bEnableSSL)
	{
		EnableSSL(m_pConfig->m_strCertFile, m_pConfig->m_strKeyFile, m_pConfig->m_strPasswd);
	}

	if (m_pConfig->m_nClientCheckTime > 0 && m_pConfig->m_strClientCheckKey.length() > 0)
	{
		m_bEnableClientCheck = true;
		m_nClientCheckKeyLength = m_pConfig->m_strClientCheckKey.length();
	}

	m_pEpoll->SetEventHandler(std::bind(&CNetworkCenter::OnNewAccept, this, std::placeholders::_1),
		std::bind(&CNetworkCenter::OnDataCanReceive, this, std::placeholders::_1),
		std::bind(&CNetworkCenter::OnCanWriteData, this, std::placeholders::_1),
		std::bind(&CNetworkCenter::OnOtherEventHappened, this, std::placeholders::_1));

	m_iMsg->RegisterEvent(MSG_TYPE::ReveivePackage);
	m_iMsg->RegisterEvent(MSG_TYPE::SocketClose);
    m_iMsg->RegisterEvent(MSG_TYPE::SendMessage);
	m_iMsg->RegisterEventHandler(MSG_TYPE::ProgramStart, std::bind(&CNetworkCenter::Run, this, std::placeholders::_1));
	m_iMsg->RegisterEventHandler(MSG_TYPE::ProgramExit, std::bind(&CNetworkCenter::Exit, this, std::placeholders::_1));
	m_iMsg->RegisterEventHandler(MSG_TYPE::SendMessage, std::bind(&CNetworkCenter::SendMessageEventHandler,this,std::placeholders::_1));
	m_iMsg->RegisterEventHandler(MSG_TYPE::SocketClose, std::bind(&CNetworkCenter::CloseConnectEventHandler, this, std::placeholders::_1));
}

void Sloong::CNetworkCenter::Run(SmartEvent event)
{
    m_bIsRunning = true;
	m_pEpoll->Run(m_pConfig->m_nPort,m_pConfig->m_nEPoolThreadQuantity);
    CThreadPool::AddWorkThread( std::bind(&CNetworkCenter::CheckTimeoutWorkLoop, this, std::placeholders::_1), nullptr);
}

void Sloong::CNetworkCenter::Exit(SmartEvent event)
{
    m_bIsRunning = false;
    m_oSync.notify_all();
	m_pEpoll->Exit();
}



void Sloong::CNetworkCenter::SendMessageEventHandler(SmartEvent event)
{
	auto send_evt = dynamic_pointer_cast<CSendMessageEvent>(event);
	SendMessage(send_evt->GetSocketID(), send_evt->GetPriority(), send_evt->GetSwift(), send_evt->GetMessage(), send_evt->GetSendExData(), send_evt->GetSendExDataSize());
}


void Sloong::CNetworkCenter::CloseConnectEventHandler(SmartEvent event)
{
	auto net_evt = dynamic_pointer_cast<CNetworkEvent>(event);
	auto id = net_evt->GetSocketID();
	auto item = m_SockList.find(id);
	if (item == m_SockList.end())
		return;

	shared_ptr<CSockInfo> info = m_SockList[id];
	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_SockList.erase(item);
	sockLck.unlock();
	if (!info)
		return;

	// in here no need delete the send list and read list
	// when delete the SocketInfo object , it will delete the list .
	info->m_pCon->Close();
	m_pLog->Info(CUniversal::Format("close connect:%s:%d.", info->m_pCon->m_strAddress, info->m_pCon->m_nPort));
}


void Sloong::CNetworkCenter::SendCloseConnectEvent(int socket)
{
	shared_ptr<CSockInfo> info = m_SockList[socket];
	if(info==nullptr)
		return;

	auto event = make_shared<CNetworkEvent>(MSG_TYPE::SocketClose);
	event->SetSocketID(socket);
	event->SetUserInfo(info->m_pUserInfo.get());
	event->SetHandler(this);
	m_iMsg->SendMessage(event);
}


void Sloong::CNetworkCenter::SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData, int nExSize)
{
	// process msg
	char* pBuf = NULL;
	long long nBufLen = s_llLen + msg.size();
	string md5("");
	if (m_pConfig->m_bEnableSwiftNumberSup)
	{
		nBufLen += s_llLen;
	}
	if (m_pConfig->m_bEnableMD5Check)
	{
		md5 = CMD5::Encode(msg);
		nBufLen += md5.length();
	}
	// in here, the exdata size no should include the buffer length,
	// nBufLen不应该包含exdata的长度,所以如果有附加数据,那么这里应该只增加Buff空间,但是前8位的长度中不包含buff长度的8位指示符.
	long long nMsgLen = nBufLen - s_llLen;
	if (pExData != NULL && nExSize > 0)
	{
		nBufLen += s_llLen;
	}

	pBuf = new char[nBufLen];
	memset(pBuf, 0, nBufLen);
	char* pCpyPoint = pBuf;

	CUniversal::LongToBytes(nMsgLen, pCpyPoint);
	pCpyPoint += 8;
	if (m_pConfig->m_bEnableSwiftNumberSup)
	{
		CUniversal::LongToBytes(nSwift, pCpyPoint);
		pCpyPoint += s_llLen;
	}
	if (m_pConfig->m_bEnableMD5Check)
	{
		memcpy(pCpyPoint, md5.c_str(), md5.length());
		pCpyPoint += md5.length();
	}
	memcpy(pCpyPoint, msg.c_str(), msg.length());
	pCpyPoint += msg.length();
	if (pExData != NULL && nExSize > 0)
	{
		long long Exlen = nExSize;
		CUniversal::LongToBytes(Exlen, pCpyPoint);
		pCpyPoint += 8;
	}

	shared_ptr<CSockInfo> pInfo = nullptr;
	
	if (m_pConfig->m_oLogInfo.ShowSendMessage)
	{
		m_pLog->Verbos(CUniversal::Format("SEND<<<[%d][%s]<<<%s",nSwift,md5,msg));
		if( pExData != nullptr )
			m_pLog->Verbos(CUniversal::Format("SEND_EXDATA<<<[%d][%s]<<<DATALEN[%d]",nSwift,md5,nExSize));
	}

	// if have exdata, directly add to epoll list.
	if (pExData != NULL && nExSize > 0)
	{
		AddToSendList(sock, nPriority, pBuf, nBufLen, 0, pExData, nExSize);
		return;
	}
	else
	{
		pInfo = m_SockList[sock];
		if (!pInfo)
		{
			m_pLog->Warn(CUniversal::Format("Get socket[%d] info from socket list failed. Close the socket.", sock));
			SendCloseConnectEvent(sock);
			return;
		}
		// check the send list size. if all empty, try send message directly.
		if ((pInfo->m_bIsSendListEmpty == false && !pInfo->m_oPrepareSendList.empty()) || pInfo->m_oSockSendMutex.try_lock() == false)
		{
			AddToSendList(sock, nPriority, pBuf, nBufLen, 0, pExData, nExSize);
			return;
		}
	}

	unique_lock<mutex> lck(pInfo->m_oSockSendMutex, std::adopt_lock);
	// if code run here. the all list is empty. and no have exdata. try send message
	int nMsgSend = pInfo->m_pCon->Write(pBuf, nBufLen, 0);
	if (nMsgSend < 0)
	{
		m_pLog->Warn(CUniversal::Format("Send data failed.[%s][%s]", pInfo->m_pCon->m_strAddress, pInfo->m_pCon->G_FormatSSLErrorMsg(nMsgSend)));
	}
	if (nMsgSend != nBufLen)
	{
		m_pLog->Verbos(CUniversal::Format("Add to send list with Priority[%d],Size[%d/%d].", nPriority, nMsgSend, nBufLen));
		AddToSendList(sock, nPriority, pBuf, nBufLen, nMsgSend, NULL, 0);
	}
	SAFE_DELETE_ARR(pBuf);
	lck.unlock();
}


void Sloong::CNetworkCenter::AddToSendList(int socket, int nPriority, const char *pBuf, int nSize, int nStart, const char* pExBuf, int nExSize)
{
	if (pBuf == NULL || nSize <= 0)
		return;

	auto info = m_SockList[socket];
	unique_lock<mutex> lck(info->m_oPreSendMutex);
	auto si = make_shared<CDataTransPackage>(info->m_pCon,info->m_nPriorityLevel,info->m_bEnableMD5Check,info->m_bEnableSwiftNumber);
	si->nSent = nStart;
	si->nSize = nSize;
	si->pSendBuffer = pBuf;
	si->pExBuffer = pExBuf;
	si->nExSize = nExSize;
	si->nPackSize = nSize + nExSize;
	PRESENDINFO psi;
	psi.pSendInfo = si;
	psi.nPriorityLevel = nPriority;
	info->m_oPrepareSendList.push(psi);
	m_pLog->Debug(CUniversal::Format("Add send package to prepare send list. list size:[%d]",info->m_oPrepareSendList.size()));
	info->m_bIsSendListEmpty = false;
	// 只有在需要使用EPoll来发送数据的时候才去添加EPOLLOUT标志
	m_pEpoll->CtlEpollEvent(EPOLL_CTL_MOD, socket, EPOLLOUT | EPOLLIN);
}


void Sloong::CNetworkCenter::EnableSSL(string certFile, string keyFile, string passwd)
{
	int ret = lConnect::G_InitializeSSL(m_pCTX,certFile, keyFile, passwd);
	if (ret != S_OK)
	{
		m_pLog->Error("Initialize SSL environment error.");
		m_pLog->Error(lConnect::G_FormatSSLErrorMsg(ret));
	}
}


/*************************************************
* Function: * check_connect_timeout
* Description: * 检测长时间没反应的网络连接，并关闭删除
* Input: *
* Output: *
* Others: *
*************************************************/
void Sloong::CNetworkCenter::CheckTimeoutWorkLoop(SMARTER param)
{
	int tout = m_pConfig->m_nConnectTimeout * 60;
	int tinterval = m_pConfig->m_nTimeoutInterval * 60;

	m_pLog->Verbos("Check connect timeout thread is running.");
	while (m_bIsRunning)
	{
		m_pLog->Verbos("Check connect timeout start.");
		RecheckTimeout:
		unique_lock<mutex> sockLck(m_oSockListMutex);
		for (map<int, shared_ptr<CSockInfo>>::iterator it = m_SockList.begin(); it != m_SockList.end(); ++it)
		{
			if (it->second != NULL && time(NULL) - it->second->m_ActiveTime > tout)
			{
				sockLck.unlock();
				m_pLog->Info(CUniversal::Format("[Timeout]:[Close connect:%s]", it->second->m_pCon->m_strAddress));
				SendCloseConnectEvent(it->first);
				goto RecheckTimeout;
			}
		}
		sockLck.unlock();
		m_pLog->Verbos(CUniversal::Format("Check connect timeout done. wait [%d] seconds.", tinterval));
		m_oSync.wait_for(tinterval);
	}
	m_pLog->Info("check timeout connect thread is exit ");
}



/// 有新链接到达。
/// 接收链接之后，需要客户端首先发送客户端校验信息。只有校验成功之后才会进行SSL处理
NetworkResult Sloong::CNetworkCenter::OnNewAccept( int conn_sock )
{
	m_pLog->Verbos("Accept function is called.");

		// start client check when acdept
		if (m_bEnableClientCheck)
		{
			char* pCheckBuf = new char[m_nClientCheckKeyLength + 1];
			memset(pCheckBuf, 0, m_nClientCheckKeyLength + 1);
			// In Check function, client need send the check key in 3 second. 
			// 这里仍然使用Universal提供的ReceEx。这里不需要进行SSL接收
			int nLen = CUniversal::RecvEx(conn_sock, pCheckBuf, m_nClientCheckKeyLength, m_pConfig->m_nClientCheckTime);
			if (nLen != m_nClientCheckKeyLength || 0 != strcmp(pCheckBuf, m_pConfig->m_strClientCheckKey.c_str()))
			{
				m_pLog->Warn(CUniversal::Format("Check Key Error.Length[%d]:[%d].Server[%s]:[%s]Client", m_nClientCheckKeyLength, nLen, m_pConfig->m_strClientCheckKey.c_str(), pCheckBuf));
				return NetworkResult::Error;
			}
		}

		auto info = make_shared<CSockInfo>(m_pConfig->m_nPriorityLevel,m_pConfig->m_bEnableMD5Check,m_pConfig->m_bEnableSwiftNumberSup);
		info->Initialize(m_iMsg,m_iData,conn_sock,m_pCTX);
	
		unique_lock<mutex> sockLck(m_oSockListMutex);
		m_SockList[conn_sock] = info;
		sockLck.unlock();

		m_pLog->Info(CUniversal::Format("Accept client:[%s:%d].", info->m_pCon->m_strAddress, info->m_pCon->m_nPort));
		
		return NetworkResult::Succeed;
}

NetworkResult Sloong::CNetworkCenter::OnDataCanReceive( int nSocket )
{
    shared_ptr<CSockInfo> info = m_SockList[nSocket];

	if (info == NULL)
	{
		m_pLog->Error("OnDataCanReceive called, but SockInfo is null.");
		return NetworkResult::Error;
	}
		
 
	auto res = info->OnDataCanReceive();
	if( res  == NetworkResult::Error)
		SendCloseConnectEvent(nSocket);
	return res;
}

NetworkResult Sloong::CNetworkCenter::OnCanWriteData( int nSocket )
{
    // 可以写入事件
	shared_ptr<CSockInfo> info = m_SockList[nSocket];

	if (info == NULL)
	{
		m_pLog->Error("OnCanWriteData called, but SockInfo is null.");
		return NetworkResult::Error;
	}

	auto res = info->OnDataCanSend();
	if( res  == NetworkResult::Error)
		SendCloseConnectEvent(nSocket);
	return res;
}


NetworkResult Sloong::CNetworkCenter::OnOtherEventHappened( int nSocket )
{
	SendCloseConnectEvent(nSocket);
	return NetworkResult::Succeed;
}