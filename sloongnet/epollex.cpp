// load system file
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <queue>
#include <sys/types.h> 
#include <sys/times.h> 
#include <sys/select.h> 
#include "epollex.h"
// load open ssl file
#include <openssl/ssl.h>
#include <openssl/err.h>
// load univ file
#include <univ/log.h>
#include <univ/MD5.h>
// load model file
#include "utility.h"
#include "lconnect.h"
#include "sockinfo.h"
#include "serverconfig.h"
#include "progressbar.h"
#include "NetworkEvent.h"
#include "SendMessageEvent.h"

#define MAXRECVBUF 4096
#define MAXBUF MAXRECVBUF+10

using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;

const int s_llLen = 8;

Sloong::CEpollEx::CEpollEx()
{
	m_bIsRunning = false;
	m_pCTX = NULL;
}

Sloong::CEpollEx::~CEpollEx()
{
	if (m_pCTX)
	{
		SSL_CTX_free(m_pCTX);
	}
}


// Initialize the epoll and the thread pool.
int Sloong::CEpollEx::Initialize(IMessage* iM,IData* iData)
{
	m_iData = iData;
	m_iMsg = iM;
	m_iMsg->RegisterEvent(ReveivePackage);
	m_iMsg->RegisterEvent(SocketClose);
	m_iMsg->RegisterEvent(MSG_TYPE::SendMessage);
	m_iMsg->RegisterEventHandler(MSG_TYPE::SendMessage, this, EventHandler);

	m_pConfig = (CServerConfig*)m_iData->Get(Configuation);
	m_pLog = (CLog*)m_iData->Get(Logger);
	if (m_pConfig->m_nClientCheckTime > 0 && m_pConfig->m_strClientCheckKey.length() > 0)
	{
		m_bEnableClientCheck = true;
		m_nClientCheckKeyLength = m_pConfig->m_strClientCheckKey.length();
	}
	m_pLog->Info(CUniversal::Format("epollex is initialize.license port is %d", m_pConfig->m_nPort ));

	// 初始化socket
	m_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
	int sock_op = 1;
	// SOL_SOCKET:在socket层面设置
	// SO_REUSEADDR:允许套接字和一个已在使用中的地址捆绑
	setsockopt(m_ListenSock, SOL_SOCKET, SO_REUSEADDR, &sock_op, sizeof(sock_op));

	// 初始化地址结构
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(m_pConfig->m_nPort);

	// 绑定端口
	errno = bind(m_ListenSock, (struct sockaddr*)&address, sizeof(address));
	if (errno == -1)
		throw normal_except(CUniversal::Format("bind to %d field. errno = %d", m_pConfig->m_nPort, errno));

	return true;
}

void Sloong::CEpollEx::Run()
{
	// 监听端口,监听队列大小为1024.可修改为SOMAXCONN
	errno = listen(m_ListenSock, 1024);
	// 设置socket为非阻塞模式
	SetSocketNonblocking(m_ListenSock);
	// 创建epoll
	m_EpollHandle = epoll_create(65535);
	// 创建epoll事件对象
	CtlEpollEvent(EPOLL_CTL_ADD, m_ListenSock, EPOLLIN | EPOLLOUT);
	m_bIsRunning = true;
	// Init the thread pool
	CThreadPool::AddWorkThread(WorkLoop, this, m_pConfig->m_nEPoolThreadQuantity);
	CThreadPool::AddWorkThread(CheckTimeoutConnect, this);
}

void Sloong::CEpollEx::EnableSSL(string certFile, string keyFile, string passwd)
{
	int ret = lConnect::G_InitializeSSL(m_pCTX,certFile, keyFile, passwd);
	if (ret != S_OK)
	{
		m_pLog->Error("Initialize SSL environment error.");
		m_pLog->Error(lConnect::G_FormatSSLErrorMsg(ret));
	}
}


void Sloong::CEpollEx::CtlEpollEvent(int opt, int sock, int events)
{
	struct epoll_event ent;
	memset(&ent, 0, sizeof(ent));
	ent.data.fd = sock;
	ent.events = events | EPOLLET;

	// 设置事件到epoll对象
	epoll_ctl(m_EpollHandle, opt, sock, &ent);
}


// 设置套接字为非阻塞模式
int Sloong::CEpollEx::SetSocketNonblocking(int socket)
{
	int op;

	op = fcntl(socket, F_GETFL, 0);
	fcntl(socket, F_SETFL, op | O_NONBLOCK);

	return op;
}

void* Sloong::CEpollEx::CALLBACK_SocketClose(void* params, void* object)
{
	CNetworkEvent* evt = TYPE_TRANS<CNetworkEvent*>(params);
	(TYPE_TRANS<CEpollEx*>(object))->CloseSocket(evt->GetSocketID());
}

/*************************************************
* Function: * epoll_loop
* Description: * epoll检测循环
* Input: *
* Output: *
* Others: *
*************************************************/
void* Sloong::CEpollEx::WorkLoop(void* pParam)
{
	CEpollEx* pThis = (CEpollEx*)pParam;
	auto pid = this_thread::get_id();
	string spid = CUniversal::ntos(pid);
	pThis->m_pLog->Info("epoll work thread is running." + spid);
	int sockListen = pThis->m_ListenSock;
	int n, i;
	while (pThis->m_bIsRunning)
	{
		// 返回需要处理的事件数
		n = epoll_wait(pThis->m_EpollHandle, pThis->m_Events, 1024, 500);

		if (n <= 0)
			continue;

		for (i = 0; i < n; ++i)
		{
			int fd = pThis->m_Events[i].data.fd;
			if (fd == sockListen)
			{
				pThis->OnNewAccept();
			}
			else if (pThis->m_Events[i].events&EPOLLIN)
			{
				pThis->OnDataCanReceive(fd);
			}
			else if (pThis->m_Events[i].events&EPOLLOUT)
			{
				pThis->OnCanWriteData(fd);
			}
			else
			{
				pThis->CloseConnect(fd);
			}
		}
	}
	pThis->m_pLog->Info("epoll work thread is exit " + spid);
	return 0;
}
/*************************************************
* Function: * check_connect_timeout
* Description: * 检测长时间没反应的网络连接，并关闭删除
* Input: *
* Output: *
* Others: *
*************************************************/
void* Sloong::CEpollEx::CheckTimeoutConnect(void* pParam)
{
	CEpollEx* pThis = (CEpollEx*)pParam;
	int tout = pThis->m_pConfig->m_nConnectTimeout * 60;
	int tinterval = pThis->m_pConfig->m_nTimeoutInterval * 60;
	unique_lock<mutex> lck(pThis->m_oExitMutex);
	while (pThis->m_bIsRunning)
	{
		for (map<int, CSockInfo*>::iterator it = pThis->m_SockList.begin(); it != pThis->m_SockList.end(); ++it)
		{
			if (it->second != NULL && time(NULL) - it->second->m_ActiveTime > tout)
			{
				pThis->m_pLog->Info(CUniversal::Format("[Timeout]:[Close Timeout connect:%s]", it->second->m_Address));
				pThis->CloseConnect(it->first);
			}
		}
		pThis->m_oExitCV.wait_for(lck, chrono::seconds(tinterval));
	}
	pThis->m_pLog->Info("check timeout connect thread is exit ");
	return 0;
}

void * Sloong::CEpollEx::EventHandler(void * params, void * object)
{
	IEvent* evt = TYPE_TRANS<IEvent*>(params);
	CEpollEx * pThis = TYPE_TRANS<CEpollEx*>(object);
	switch (evt->GetEvent())
	{
	case MSG_TYPE::SendMessage: {
		auto send_evt = EVENT_TRANS<CSendMessageEvent*>(evt);
		pThis->SendMessage(send_evt->GetSocketID(), send_evt->GetPriority(), send_evt->GetSwift(), send_evt->GetMessage(), send_evt->GetSendExData(), send_evt->GetSendExDataSize());
		break;
	}
	default:
		break;
	}
	SAFE_RELEASE_EVENT(evt);
	return nullptr;
}

void Sloong::CEpollEx::SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData, int nSize)
{
	if (m_pConfig->m_oLogInfo.ShowSendMessage)
		m_pLog->Verbos(string("SEND>>>") + msg);

	// process msg
	char* pBuf = NULL;
	long long nBufLen = s_llLen + msg.size();
	string md5;
	if (m_pConfig->m_bEnableSwiftNumberSup)
	{
		nBufLen += s_llLen;
	}
	if (m_pConfig->m_bEnableMD5Check)
	{
		md5 = CMD5::Encoding(msg);
		nBufLen += md5.length();
	}
	// in here, the exdata size no should include the buffer length,
	// nBufLen不应该包含exdata的长度,所以如果有附加数据,那么这里应该只增加Buff空间,但是前8位的长度中不包含buff长度的8位指示符.
	long long nMsgLen = nBufLen - s_llLen;
	if (pExData != NULL && nSize > 0)
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
	if (pExData != NULL && nSize > 0)
	{
		long long Exlen = nSize;
		CUniversal::LongToBytes(Exlen, pCpyPoint);
		pCpyPoint += 8;
	}

	CSockInfo* pInfo = NULL;

	// if have exdata, directly add to epoll list.
	if (pExData != NULL && nSize > 0)
	{
		AddToSendList(sock, nPriority, pBuf, nBufLen, 0, pExData, nSize);
		return;
	}
	else
	{
		pInfo = m_SockList[sock];
		if (!pInfo)
		{
			m_pLog->Warn(CUniversal::Format("Get socket[%d] info from socket list failed. Close the socket.", sock));
			CloseConnect(sock);
			return;
		}
		// check the send list size. if all empty, try send message directly.
		if ((pInfo->m_bIsSendListEmpty == false && !pInfo->m_pPrepareSendList->empty()) || pInfo->m_oSockSendMutex.try_lock() == false)
		{
			AddToSendList(sock, nPriority, pBuf, nBufLen, 0, pExData, nSize);
			return;
		}
	}

	unique_lock<mutex> lck(pInfo->m_oSockSendMutex, std::adopt_lock);
	// if code run here. the all list is empty. and no have exdata. try send message
	int nMsgSend = pInfo->m_pCon->Write(pBuf, nSize, 0);
	if (nMsgSend < 0)
	{
		m_pLog->Warn(CUniversal::Format("Send data failed.[%s][%s]", pInfo->m_Address, pInfo->m_pCon->G_FormatSSLErrorMsg(nMsgSend)));
	}
	if (nMsgSend != nSize)
	{
		m_pLog->Verbos(CUniversal::Format("Add to send list with Priority[%d],Size[%d/%d].", nPriority, nMsgSend,nSize));
		AddToSendList(sock, nPriority, pBuf, nSize, nMsgSend, NULL, 0);
	}
	SAFE_DELETE_ARR(pBuf);
	lck.unlock();
}


void Sloong::CEpollEx::AddToSendList(int socket, int nPriority, const char *pBuf, int nSize, int nStart, const char* pExBuf, int nExSize)
{
	if (pBuf == NULL || nSize <= 0)
		return;

	CSockInfo* info = m_SockList[socket];
	unique_lock<mutex> lck(info->m_oPreSendMutex);
	SENDINFO *si = new SENDINFO();
	si->nSent = nStart;
	si->nSize = nSize;
	si->pSendBuffer = pBuf;
	si->pExBuffer = pExBuf;
	si->nExSize = nExSize;
	PRESENDINFO psi;
	psi.pSendInfo = si;
	psi.nPriorityLevel = nPriority;
	info->m_pPrepareSendList->push(psi);
	info->m_bIsSendListEmpty = false;
	SetSocketNonblocking(socket);
	CtlEpollEvent(EPOLL_CTL_MOD, socket, EPOLLOUT | EPOLLIN);
}


void Sloong::CEpollEx::CloseConnect(int socket)
{
	CtlEpollEvent(EPOLL_CTL_DEL, socket, EPOLLIN | EPOLLOUT);
	CSockInfo* info = m_SockList[socket];
	if (!info)
		return;

	info->m_pCon->Close();
	m_pLog->Info(CUniversal::Format("close connect:%s:%d.", info->m_Address, info->m_nPort));

	CNetworkEvent* event = new CNetworkEvent(SocketClose);
	event->SetSocketID(socket);
	event->SetSocketInfo(info);
	event->SetHandler(this);
	event->SetCallbackFunc(CALLBACK_SocketClose);
	m_iMsg->SendMessage(event);
}


/// 有新链接到达。
/// 接收链接之后，需要客户端首先发送客户端校验信息。只有校验成功之后才会进行SSL处理
void Sloong::CEpollEx::OnNewAccept()
{
	// accept the connect and add it to the list
	int conn_sock = -1;
	while ((conn_sock = accept(m_ListenSock, NULL, NULL)) > 0)
	{
		struct sockaddr_in add;
		int nSize = sizeof(add);
		memset(&add, 0, sizeof(add));
		getpeername(conn_sock, (sockaddr*)&add, (socklen_t*)&nSize);

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
				close(conn_sock);
				return;
			}
		}

		

		CSockInfo* info = new CSockInfo(m_pConfig->m_nPriorityLevel);
		info->m_Address = string(inet_ntoa(add.sin_addr));
		info->m_nPort = add.sin_port;
		info->m_ActiveTime = time(NULL);
		info->m_pCon->Initialize(conn_sock,m_pCTX);
		info->m_pUserInfo->SetData("ip", info->m_Address);
		info->m_pUserInfo->SetData("port", CUniversal::ntos(info->m_nPort));

		m_SockList[conn_sock] = info;
		m_pLog->Info(CUniversal::Format("accept client:%s.", info->m_Address));
		//将接受的连接添加到Epoll的事件中.
		// Add the recv event to epoll;
		SetSocketNonblocking(conn_sock);
		CtlEpollEvent(EPOLL_CTL_ADD, conn_sock, EPOLLIN | EPOLLOUT);
	}
	if (conn_sock == -1)
	{
		if (errno == EAGAIN)
			return;
		else
			m_pLog->Warn("accept error.");
	}
}

void Sloong::CEpollEx::OnDataCanReceive(int nSocket)
{
	CSockInfo* info = m_SockList[nSocket];
	// The app is used ET mode, so should wait the mutex. 
	unique_lock<mutex> srlck(info->m_oSockReadMutex);

		auto pid = this_thread::get_id();
	string spid = CUniversal::ntos(pid);

	// 已经连接的用户,收到数据,可以开始读入
	char* pLongBuffer = new char[s_llLen + 1]();//dataLeng;
	bool bLoop = true;
	while (bLoop)
	{
		// 先读取消息长度
		memset(pLongBuffer, 0, s_llLen + 1);
		int nRecvSize = info->m_pCon->Read( pLongBuffer, s_llLen, m_pConfig->m_nReceiveTimeout);
		if (nRecvSize < 0)
		{
			// 读取错误,将这个连接从监听中移除并关闭连接
			CloseConnect(nSocket);
			break;
		}
		else if (nRecvSize == 0)
		{
			//由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读在这里就当作是该次事件已处理过。
			break;
		}
		else
		{
			long long dtlen = CUniversal::BytesToLong(pLongBuffer);
			// package length cannot big than 2147483648. this is max value for int.
			if (dtlen <= 0 || dtlen > 2147483648 || nRecvSize != s_llLen)
			{
				m_pLog->Error("Receive data length error.");
				CloseConnect(nSocket);
				break;
			}
			char* data = new char[dtlen + 1];
			memset(data, 0, dtlen + 1);

			nRecvSize = info->m_pCon->Read(data, dtlen, m_pConfig->m_nReceiveTimeout, true);//一次性接受所有消息
			if (nRecvSize < 0)
			{
				CloseConnect(nSocket);
				break;
			}

			int nPriority = 0;
			RECVINFO recvInfo;

			const char* pMsg = NULL;
			// check the priority level
			if (m_pConfig->m_nPriorityLevel != 0)
			{
				char pLevel[2] = { 0 };
				pLevel[0] = data[0];
				int level = pLevel[0];
				if (level > m_pConfig->m_nPriorityLevel || level < 0)
				{
					m_pLog->Error(CUniversal::Format("Receive priority level error. the data is %d, the config level is %d. add this message to last list", level, m_pConfig->m_nPriorityLevel));
					nPriority = m_pConfig->m_nPriorityLevel - 1;
				}
				else
				{
					nPriority = level;
				}
				pMsg = &data[1];
			}
			else
			{
				nPriority = 0;
				pMsg = data;
			}

			if (m_pConfig->m_bEnableSwiftNumberSup)
			{
				memset(pLongBuffer, 0, s_llLen);
				memcpy(pLongBuffer, pMsg, s_llLen);
				recvInfo.nSwiftNumber = CUniversal::BytesToLong(pLongBuffer);
				pMsg += s_llLen;
			}

			if (m_pConfig->m_bEnableMD5Check)
			{
				char tmd5[33] = { 0 };
				memcpy(tmd5, pMsg, 32);
				recvInfo.strMD5 = tmd5;
				pMsg += 32;
			}

			recvInfo.strMessage.clear();
			recvInfo.strMessage = string(pMsg);

			// Add the msg to the sock info list
			delete[] data;
			if (m_pConfig->m_oLogInfo.ShowReceiveMessage)
				m_pLog->Verbos(string("RECV<<<") + recvInfo.strMessage);

			// update the socket time
			info->m_ActiveTime = time(NULL);
			// Add the sock event to list
			CNetworkEvent* event = new CNetworkEvent(ReveivePackage);
			event->SetSocketID(nSocket);
			event->SetSocketInfo(info);
			event->SetPriority(nPriority);
			event->SetRecvPackage(recvInfo);
			m_iMsg->SendMessage(event);
		}
	}
	srlck.unlock();
	SAFE_DELETE_ARR(pLongBuffer);
}

void Sloong::CEpollEx::OnCanWriteData(int nSocket)
{
	// 可以写入事件
	CSockInfo* info = m_SockList[nSocket];

	ProcessPrepareSendList(info);
	int flag = EPOLLIN;
	if (!ProcessSendList(info))
		flag = EPOLLIN | EPOLLOUT;

	CtlEpollEvent(EPOLL_CTL_MOD, nSocket, flag);
}

void Sloong::CEpollEx::ProcessPrepareSendList(CSockInfo* info)
{
	if (info == NULL)
		return;

	// progress the prepare send list first
	if (!info->m_pPrepareSendList->empty())
	{
		unique_lock<mutex> prelck(info->m_oPreSendMutex);
		if (info->m_pPrepareSendList->empty())
		{
			prelck.unlock();
			return;
		}

		// TODO:: in here i think no need lock the send list. just push data.
		while (!info->m_pPrepareSendList->empty())
		{
			PRESENDINFO* psi = &info->m_pPrepareSendList->front();
			info->m_pPrepareSendList->pop();
			info->m_pSendList[psi->nPriorityLevel].push(psi->pSendInfo);
		}

		prelck.unlock();
	}
}

bool Sloong::CEpollEx::ProcessSendList(CSockInfo* pInfo)
{
	// when prepare list process done, do send operation.
	unique_lock<mutex> lck(pInfo->m_oSockSendMutex);

	bool bTrySend = true;

	while (bTrySend)
	{
		queue<SENDINFO*>* list = NULL;
		// prev package no send end. find and try send it again.
		if (-1 != pInfo->m_nLastSentTags)
		{
			m_pLog->Verbos(CUniversal::Format("Send prev time list, Priority level:%d", pInfo->m_nLastSentTags));
			list = &pInfo->m_pSendList[pInfo->m_nLastSentTags];
		}
		// find next package. 
		else
		{
			for (int i = 0; i < pInfo->m_nPriorityLevel; i++)
			{
				if (pInfo->m_pSendList[i].empty())
					continue;
				else
				{
					list = &pInfo->m_pSendList[i];
					pInfo->m_nLastSentTags = i;
					m_pLog->Verbos(CUniversal::Format("Send list, Priority level:%d", i));
				}
			}
		}
		if (list == NULL)
		{
			m_pLog->Verbos("Send list is null, send function return.");
			return true;
		}


		// if no find send info, is no need send anything , remove this sock from epoll.'
		SENDINFO* si = NULL;
		while (si == NULL)
		{
			if (!list->empty())
			{
				si = list->front();
				if (si == NULL)
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

		if (si == NULL)
		{
			if (pInfo->m_nLastSentTags != -1)
			{
				m_pLog->Verbos("Current list no send message, clear the LastSentTags flag.");
				pInfo->m_nLastSentTags = -1;
			}
			else
			{
				m_pLog->Verbos(CUniversal::Format("No message need send, remove socket[%d] from Epoll", pInfo->m_pCon->GetSocket()));
				CtlEpollEvent(EPOLL_CTL_MOD, pInfo->m_pCon->GetSocket(), EPOLLIN);
				pInfo->m_bIsSendListEmpty = true;
			}
		}

		// try send first.
		if (si->nSent >= si->nSize)
		{
			// Send ex data
			si->nSent = pInfo->m_pCon->Write( si->pExBuffer, si->nExSize, si->nSent - si->nSize) + si->nSize;
		}
		else
		{
			// send normal data.
			si->nSent = pInfo->m_pCon->Write( si->pSendBuffer, si->nSize, si->nSent);
			// when send nurmal data succeeded, try send exdata in one time.
			if (si->nSent != -1 && si->nSent == si->nSize)
			{
				si->nSent = pInfo->m_pCon->Write(si->pExBuffer, si->nExSize, si->nSent - si->nSize) + si->nSize;
			}
		}
		m_pLog->Verbos(CUniversal::Format("Send Info : AllSize[%d],ExSize[%d],Sent[%d]", si->nExSize + si->nSize, si->nExSize, si->nSent));


		if (si->nSent == -1)
		{
			// socket closed
			m_pLog->Warn(CUniversal::Format("Send failed, close socket:[%d]", pInfo->m_pCon->GetSocket()));
			lck.unlock();
			CloseConnect(pInfo->m_pCon->GetSocket());
			break;
		}
		// check send result.
		// send done, remove the is sent data and try send next package.
		if (si->nSent == (si->nSize + si->nExSize))
		{
			m_pLog->Verbos(CUniversal::Format("Message package send succeed, remove from send list. All size[%d]", si->nSent));
			list->pop();
			pInfo->m_nLastSentTags = -1;
			SAFE_DELETE_ARR(si->pSendBuffer);
			SAFE_DELETE_ARR(si->pExBuffer);
			SAFE_DELETE(si);
			bTrySend = true;
		}
		// send falied, wait next event.
		else if (si->nSent >= (si->nSize + si->nExSize))
		{
			m_pLog->Warn(CUniversal::Format("Message package send succeed,but SentSize[%d] is big than AllSize[%d]=nSize[%d] + nExSize[%d]. remove from send list.", si->nSent, si->nExSize + si->nSize, si->nSent, si->nExSize));
			list->pop();
			pInfo->m_nLastSentTags = -1;
			SAFE_DELETE_ARR(si->pSendBuffer);
			SAFE_DELETE_ARR(si->pExBuffer);
			SAFE_DELETE(si);
			bTrySend = true;
		}
		else
		{
			bTrySend = false;
		}
	}

	lck.unlock();
	return false;
}

void Sloong::CEpollEx::CloseSocket(int socket)
{
	CSockInfo* info = m_SockList[socket];
	if (!info)
		return;

	unique_lock<mutex> lsck(info->m_oSendListMutex);
	auto key = m_SockList.find(socket);
	if (key == m_SockList.end())
		return;

	// in here no need delete the send list and read list
	// when delete the SocketInfo object , it will delete the list .

	for (map<int, CSockInfo*>::iterator i = m_SockList.begin(); i != m_SockList.end();)
	{
		if (i->first == socket)
		{
			m_SockList.erase(i);
			break;
		}
		else
		{
			i++;
		}
	}

	lsck.unlock();
	SAFE_DELETE(info);
}



void Sloong::CEpollEx::Exit()
{
	m_bIsRunning = false;
	unique_lock<mutex> lck(m_oExitMutex);
	m_oExitCV.notify_all();
}

void Sloong::CEpollEx::SetLogConfiguration(bool bShowSendMessage, bool bShowReceiveMessage)
{
	m_pConfig->m_oLogInfo.ShowSendMessage = bShowSendMessage;
	m_pConfig->m_oLogInfo.ShowReceiveMessage= bShowReceiveMessage;
}


