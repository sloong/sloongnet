// load system file

#include "epollex.h"
#include "defines.h"
// load open ssl file
#include <openssl/ssl.h>
#include <openssl/err.h>
// load model file

#include "utility.h"
#include "lconnect.h"
#include "sockinfo.h"
#include "serverconfig.h"
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
	m_iMsg->RegisterEventHandler(MSG_TYPE::SendMessage, std::bind(&CEpollEx::SendMessageEventHandler,this,std::placeholders::_1));

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
	CThreadPool::AddWorkThread( std::bind(&CEpollEx::MainWorkLoop, this, std::placeholders::_1), nullptr, m_pConfig->m_nEPoolThreadQuantity);
	CThreadPool::AddWorkThread( std::bind(&CEpollEx::CheckTimeoutWorkLoop, this, std::placeholders::_1), nullptr);
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
	// LT模式时，事件就绪时，假设对事件没做处理，内核会反复通知事件就绪	  	EPOLLLT
	// ET模式时，事件就绪时，假设对事件没做处理，内核不会反复通知事件就绪  	EPOLLET
	ent.events = events | EPOLLERR | EPOLLHUP | EPOLLET;

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

/*************************************************
* Function: * epoll_loop
* Description: * epoll检测循环
* Input: *
* Output: *
* Others: *
*************************************************/
void Sloong::CEpollEx::MainWorkLoop(SMARTER param)
{
	auto pid = this_thread::get_id();
	string spid = CUniversal::ntos(pid);
	m_pLog->Info("epoll work thread is running." + spid);
	int n, i;
	while (m_bIsRunning)
	{
		// 返回需要处理的事件数
		n = epoll_wait(m_EpollHandle, m_Events, 1024, 500);

		if (n <= 0)
			continue;

		for (i = 0; i < n; ++i)
		{
			int fd = m_Events[i].data.fd;
			if (fd == m_ListenSock)
			{
				m_pLog->Verbos("EPoll Accept event happened.");
				OnNewAccept();
			}
			else if (m_Events[i].events&EPOLLIN)
			{
				m_pLog->Verbos("EPoll EPOLLIN event happened.Data Can Receive.");
				OnDataCanReceive(fd);
			}
			else if (m_Events[i].events&EPOLLOUT)
			{
				m_pLog->Verbos("EPoll EPOLLOUT event happened.Can Write Data.");
				OnCanWriteData(fd);
			}
			else
			{
				m_pLog->Verbos("EPoll unkuown event happened.close this connnect.");
				CloseConnect(fd);
			}
		}
	}
	m_pLog->Info("epoll work thread is exit " + spid);
}
/*************************************************
* Function: * check_connect_timeout
* Description: * 检测长时间没反应的网络连接，并关闭删除
* Input: *
* Output: *
* Others: *
*************************************************/
void Sloong::CEpollEx::CheckTimeoutWorkLoop(SMARTER param)
{
	int tout = m_pConfig->m_nConnectTimeout * 60;
	int tinterval = m_pConfig->m_nTimeoutInterval * 60;
	unique_lock<mutex> lck(m_oExitMutex);
	m_pLog->Verbos("Check connect timeout thread is running.");
	while (m_bIsRunning)
	{
		m_pLog->Verbos("Check connect timeout start.");
		RecheckTimeout:
		unique_lock<mutex> sockLck(m_oSockListMutex);
		for (map<int, CSockInfo*>::iterator it = m_SockList.begin(); it != m_SockList.end(); ++it)
		{
			if (it->second != NULL && time(NULL) - it->second->m_ActiveTime > tout)
			{
				sockLck.unlock();
				m_pLog->Info(CUniversal::Format("[Timeout]:[Close connect:%s]", it->second->m_Address));
				CloseConnect(it->first);
				goto RecheckTimeout;
			}
		}
		sockLck.unlock();
		m_pLog->Verbos(CUniversal::Format("Check connect timeout done. wait [%d] seconds.", tinterval));
		m_oExitCV.wait_for(lck, chrono::seconds(tinterval));
	}
	m_pLog->Info("check timeout connect thread is exit ");
}

void Sloong::CEpollEx::SendMessageEventHandler(SmartEvent event)
{
	auto send_evt = dynamic_pointer_cast<CSendMessageEvent>(event);
	SendMessage(send_evt->GetSocketID(), send_evt->GetPriority(), send_evt->GetSwift(), send_evt->GetMessage(), send_evt->GetSendExData(), send_evt->GetSendExDataSize());
}

void Sloong::CEpollEx::SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData, int nExSize)
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

	CSockInfo* pInfo = NULL;
	
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
			CloseConnect(sock);
			return;
		}
		// check the send list size. if all empty, try send message directly.
		if ((pInfo->m_bIsSendListEmpty == false && !pInfo->m_pPrepareSendList->empty()) || pInfo->m_oSockSendMutex.try_lock() == false)
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
		m_pLog->Warn(CUniversal::Format("Send data failed.[%s][%s]", pInfo->m_Address, pInfo->m_pCon->G_FormatSSLErrorMsg(nMsgSend)));
	}
	if (nMsgSend != nBufLen)
	{
		m_pLog->Verbos(CUniversal::Format("Add to send list with Priority[%d],Size[%d/%d].", nPriority, nMsgSend, nBufLen));
		AddToSendList(sock, nPriority, pBuf, nBufLen, nMsgSend, NULL, 0);
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
	CSendInfo *si = new CSendInfo();
	si->nSent = nStart;
	si->nSize = nSize;
	si->pSendBuffer = pBuf;
	si->pExBuffer = pExBuf;
	si->nExSize = nExSize;
	si->nPackSize = nSize + nExSize;
	PRESENDINFO psi;
	psi.pSendInfo = si;
	psi.nPriorityLevel = nPriority;
	info->m_pPrepareSendList->push(psi);
	m_pLog->Debug(CUniversal::Format("Add send package to prepare send list. list size:[%d]",info->m_pPrepareSendList->size()));
	info->m_bIsSendListEmpty = false;
	SetSocketNonblocking(socket);
	// 只有在需要使用EPoll来发送数据的时候才去添加EPOLLOUT标志
	CtlEpollEvent(EPOLL_CTL_MOD, socket, EPOLLOUT | EPOLLIN);
}


/// 有新链接到达。
/// 接收链接之后，需要客户端首先发送客户端校验信息。只有校验成功之后才会进行SSL处理
void Sloong::CEpollEx::OnNewAccept()
{
	// accept the connect and add it to the list
	int conn_sock = -1;
	do
	{
		m_pLog->Verbos("Accept function is called.");
		conn_sock = accept(m_ListenSock, NULL, NULL);
		if (conn_sock == -1)
		{
			if (errno == EAGAIN)
			{
				m_pLog->Verbos("Accept end. But the result is -1 and the errno is EAGAIN.");
			}
			else
			{
				m_pLog->Warn("Accept error.");
			}
			return;
		}

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
				shutdown(conn_sock,SHUT_RDWR);
				close(conn_sock);
				return;
			}
		}

		CSockInfo* info = new CSockInfo(m_pConfig->m_nPriorityLevel);
		info->m_Address = CUtility::GetSocketIP(conn_sock);
		info->m_nPort = CUtility::GetSocketPort(conn_sock);
		info->m_ActiveTime = time(NULL);
		info->m_pCon->Initialize(conn_sock,m_pCTX);
		info->m_pUserInfo->SetData("ip", info->m_Address);
		info->m_pUserInfo->SetData("port", CUniversal::ntos(info->m_nPort));

		unique_lock<mutex> sockLck(m_oSockListMutex);
		m_SockList[conn_sock] = info;
		sockLck.unlock();

		m_pLog->Info(CUniversal::Format("Accept client:[%s:%d].", info->m_Address, info->m_nPort));
		//将接受的连接添加到Epoll的事件中.
		// Add the recv event to epoll;
		SetSocketNonblocking(conn_sock);
		// 刚接收连接，所以只关心可读状态。
		CtlEpollEvent(EPOLL_CTL_ADD, conn_sock, EPOLLIN);
	}while(conn_sock > 0);
}

void Sloong::CEpollEx::OnDataCanReceive(int nSocket)
{
	CSockInfo* info = m_SockList[nSocket];
	if( info == nullptr )
	{
		m_pLog->Error("Error in DataCanReceive functions. the socketlist is null");
		CloseConnect(nSocket);
		return;
	}
	// The app is used ET mode, so should wait the mutex. 
	unique_lock<mutex> srlck(info->m_oSockReadMutex);

	auto pid = this_thread::get_id();
	string spid = CUniversal::ntos(pid);

	// 已经连接的用户,收到数据,可以开始读入
	char* pLongBuffer = new char[s_llLen + 1]();//dataLeng;
	bool bLoop = false;
	do 
	{
		// 先读取消息长度
		memset(pLongBuffer, 0, s_llLen + 1);
		int nRecvSize = info->m_pCon->Read( pLongBuffer, s_llLen, 2);
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
			bLoop = true;
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
			else if(nRecvSize != dtlen )
			{
				m_pLog->Warn(CUniversal::Format("Receive all data is timeout. recved lenght %d, data length %d",nRecvSize, dtlen));
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
			SAFE_DELETE_ARR(data);
			if (m_pConfig->m_oLogInfo.ShowReceiveMessage)
				m_pLog->Verbos(CUniversal::Format("RECV<<<[%d][%s]<<<%s",recvInfo.nSwiftNumber,recvInfo.strMD5, recvInfo.strMessage));

			// update the socket time
			info->m_ActiveTime = time(NULL);
			// Add the sock event to list

			auto event = make_shared<CNetworkEvent>(MSG_TYPE::ReveivePackage);
			
			event->SetSocketID(nSocket);
			event->SetSocketInfo(info);
			event->SetPriority(nPriority);
			event->SetRecvPackage(recvInfo);
			m_iMsg->SendMessage(event);
		}
	}while (bLoop);

	srlck.unlock();
	SAFE_DELETE_ARR(pLongBuffer);
}

void Sloong::CEpollEx::OnCanWriteData(int nSocket)
{
	// 可以写入事件
	CSockInfo* info = m_SockList[nSocket];

	if (info == NULL)
		return;

	ProcessPrepareSendList(info);
	ProcessSendList(info);
}

void Sloong::CEpollEx::ProcessPrepareSendList(CSockInfo* info)
{
	// progress the prepare send list first
	if (!info->m_pPrepareSendList->empty())
	{
		unique_lock<mutex> prelck(info->m_oPreSendMutex);
		if (info->m_pPrepareSendList->empty())
		{
			return;
		}
		unique_lock<mutex> sendListlck(info->m_oSendListMutex);
		
		while (!info->m_pPrepareSendList->empty())
		{
			PRESENDINFO* psi = &info->m_pPrepareSendList->front();
			info->m_pPrepareSendList->pop();
			info->m_pSendList[psi->nPriorityLevel].push(psi->pSendInfo);
			m_pLog->Debug(CUniversal::Format("Add send package to send list[%d]. send list size[%d], prepare send list size[%d]",
								psi->nPriorityLevel,info->m_pSendList[psi->nPriorityLevel].size(),info->m_pPrepareSendList->size()));
		}
		prelck.unlock();
		sendListlck.unlock();
	}
}



/// 获取发送信息列表
// 首先判断上次发送标志，如果不为-1，表示上次的发送列表没有发送完成。直接返回指定的列表
// 如果为-1，表示需要发送新的列表。按照优先级逐级的进行寻找。
int Sloong::CEpollEx::GetSendInfoList(CSockInfo* pInfo, queue<CSendInfo*>** list )
{
	*list = nullptr;
	// prev package no send end. find and try send it again.
	if (-1 != pInfo->m_nLastSentTags)
	{
		m_pLog->Verbos(CUniversal::Format("Send prev time list, Priority level:%d", pInfo->m_nLastSentTags));
		*list = &pInfo->m_pSendList[pInfo->m_nLastSentTags];
		if( (*list)->empty() )
			pInfo->m_nLastSentTags = -1;
		else
			return pInfo->m_nLastSentTags;
	}
	
	for (int i = 0; i < pInfo->m_nPriorityLevel; i++)
	{
		if (pInfo->m_pSendList[i].empty())
			continue;
		else
		{
			*list = &pInfo->m_pSendList[i];
			m_pLog->Verbos(CUniversal::Format("Send list, Priority level:%d", i));
			return i;
		}
	}
	return -1;
}


CSendInfo* Sloong::CEpollEx::GetSendInfo(CSockInfo* pInfo,queue<CSendInfo*>* list)
{
	CSendInfo* si = NULL;
	while (si == NULL)
	{
		if (!list->empty())
		{
			m_pLog->Verbos(CUniversal::Format("Get send info from list, list size[%d].", list->size()));
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
	return si;
}


/// 发送数据包
// 发送失败返回-1；需要关闭连接
// 需要再次发送返回0；需要监听可写信息
// 发送完成返回1；需要监听可读信息
int Sloong::CEpollEx::SendPackage(CSockInfo* pInfo, CSendInfo* si)
{
	unique_lock<mutex> lck(pInfo->m_oSockSendMutex);

	// 首先检查是不是已经发送过部分的数据了
	if( si->nSent > 0 )
	{
		// 先检查普通数据发送状态
		if( si->nSent < si->nSize)
		{
			int nSentSize = pInfo->m_pCon->Write(si->pSendBuffer, si->nSize, si->nSent);
			if( nSentSize < 0 )
			{
				return -1;
			}
			else
			{
				si->nSent = nSentSize;
			}

		}
		// 已经发送完普通数据了，需要继续发送扩展数据
		if ( si->nSent >= si->nSize && si->nExSize > 0 )
		{
			int nSentSize = pInfo->m_pCon->Write(si->pExBuffer, si->nExSize, si->nSent - si->nSize);
			if( nSentSize < 0 )
			{
				return -1;
			}
			else
			{
				si->nSent = si->nSize + nSentSize;
			}
		}
	}
	else
	{
		// send normal data.
		si->nSent = pInfo->m_pCon->Write( si->pSendBuffer, si->nSize, si->nSent);
		// when send nurmal data succeeded, try send exdata in one time.
		if (si->nSent != -1 && si->nSent == si->nSize && si->nExSize > 0)
		{
			int nSentSize = pInfo->m_pCon->Write(si->pExBuffer, si->nExSize, 0);
			if( nSentSize < 0 )
			{
				return -1;
			}
			else
			{
				si->nSent = si->nSize + nSentSize;
			}
		}
	}
	m_pLog->Verbos(CUniversal::Format("Send Info : AllSize[%d],ExSize[%d],Sent[%d]", si->nPackSize, si->nExSize, si->nSent));

	// check send result.
	// send done, remove the is sent data and try send next package.
	if (si->nSent < si->nPackSize)
	{
		return 0;
	}
	else
	{
		m_pLog->Verbos(CUniversal::Format("Message package send succeed, remove from send list. All size[%d]", si->nSent));
		return 1;
	}
}


void Sloong::CEpollEx::ProcessSendList(CSockInfo* pInfo)
{
	// when prepare list process done, do send operation.
	
	bool bTrySend = true;

	// 这里始终从list开始循环，保证高优先级的信息先被处理
	while (bTrySend)
	{
		unique_lock<mutex> lck(pInfo->m_oSendListMutex);

		queue<CSendInfo*>* list = nullptr;
		int sendTags = GetSendInfoList(pInfo,&list);
		if (list == NULL)
		{
			m_pLog->Error("Send info list empty, no need send.");
			break;
		}

		// if no find send info, is no need send anything , remove this sock from epoll.'
		CSendInfo* si = NULL;
		
		si = GetSendInfo(pInfo,list);
		if ( si != NULL )
		{
			lck.unlock();
			int res = SendPackage(pInfo, si);
		
			if( res < 0)
			{
				m_pLog->Error(CUniversal::Format("Send data package error. close connect:[%s:%d]",pInfo->m_Address,pInfo->m_nPort));
				lck.unlock();
				CloseConnect(pInfo->m_pCon->GetSocket());
				return;
			}
			else if( res == 0)
			{
				m_pLog->Verbos("Send data package done. wait next write sign.");
				//CtlEpollEvent(EPOLL_CTL_MOD, pInfo->m_pCon->GetSocket(), EPOLLIN | EPOLLOUT);
				bTrySend = false;
				pInfo->m_nLastSentTags = sendTags;
				return;
			}
			else
			{
				list->pop();
				pInfo->m_nLastSentTags = -1;
				SAFE_DELETE(si);
				bTrySend = true;
			}		
		}
	}
	CtlEpollEvent(EPOLL_CTL_MOD, pInfo->m_pCon->GetSocket(), EPOLLIN);
}

void Sloong::CEpollEx::CloseConnect(int socket)
{
	CtlEpollEvent(EPOLL_CTL_DEL, socket, EPOLLIN | EPOLLOUT);
	auto item = m_SockList.find(socket);
	if (item == m_SockList.end())
		return;

	CSockInfo* info = m_SockList[socket];
	unique_lock<mutex> sockLck(m_oSockListMutex);
	m_SockList.erase(item);
	sockLck.unlock();
	if (!info)
		return;

	// in here no need delete the send list and read list
	// when delete the SocketInfo object , it will delete the list .
	info->m_pCon->Close();
	m_pLog->Info(CUniversal::Format("close connect:%s:%d.", info->m_Address, info->m_nPort));

	auto event = make_shared<CNetworkEvent>(MSG_TYPE::SocketClose);
	event->SetSocketID(socket);
	event->SetSocketInfo(info);
	event->SetHandler(this);
	m_iMsg->SendMessage(event);
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


