#include "DataTransPackage.h"

#include "main.h"
#include "NetworkEvent.h"
using namespace Sloong::Events;

void Sloong::CDataTransPackage::Initialize(IControl* iMsg,SmartConnect conn)
{
	IObject::Initialize(iMsg);
	m_pCon = conn;	
}

void Sloong::CDataTransPackage::ResponsePackage(const string &msg, const char *pExData, int nExSize)
{
	long long nBufLen = s_llLen + msg.size();
	string md5("");
	if (g_pConfig->m_bEnableSwiftNumberSup)
	{
		nBufLen += s_llLen;
	}
	if (g_pConfig->m_bEnableMD5Check)
	{
		md5 = CMD5::Encode(msg);
		nBufLen += md5.length();
	}

	if (g_pConfig->m_oLogInfo.ShowSendMessage)
	{
		m_pLog->Verbos(CUniversal::Format("SEND<<<[%d][%s]<<<%s",m_nSerialNumber,md5,msg));
		if( pExData != nullptr )
			m_pLog->Verbos(CUniversal::Format("SEND_EXDATA<<<[%d]<<<DATALEN[%d]",m_nSerialNumber,nExSize));
	}


	// in here, the exdata size no should include the buffer length,
	// nBufLen不应该包含exdata的长度,所以如果有附加数据,那么这里应该只增加Buff空间,但是前8位的长度中不包含buff长度的8位指示符.
	long long nMsgLen = nBufLen - s_llLen;
	if (pExData != NULL && nExSize > 0)
	{
		nBufLen += s_llLen;
	}

	m_pMsgBuffer = new char[nBufLen];
	memset(m_pMsgBuffer, 0, nBufLen);
	char *pCpyPoint = m_pMsgBuffer;

	CUniversal::LongToBytes(nMsgLen, pCpyPoint);
	pCpyPoint += 8;
	if (g_pConfig->m_bEnableSwiftNumberSup)
	{
		CUniversal::LongToBytes(m_nSerialNumber, pCpyPoint);
		pCpyPoint += s_llLen;
	}
	if (g_pConfig->m_bEnableMD5Check)
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

	m_pExBuffer = pExData;
	m_nExSize = nExSize;
}

/**
 * @Remarks: 
 * @Params: 
 * @Return: if send succeed, return true. need listen read message. 
 *          else return false, need listen write message.
 */
NetworkResult Sloong::CDataTransPackage::SendPackage()
{
	// 首先检查是不是已经发送过部分的数据了
	if (m_nSent > 0)
	{
		// 先检查普通数据发送状态
		if (m_nSent < m_nMsgSize)
		{
			int nSentSize = m_pCon->Write(m_pMsgBuffer, m_nMsgSize, m_nSent);
			if (nSentSize < 0)
			{
				return NetworkResult::Error;
			}
			else
			{
				m_nSent = nSentSize;
			}
		}
		// 已经发送完普通数据了，需要继续发送扩展数据
		if (m_nSent >= m_nMsgSize && m_nExSize > 0)
		{
			int nSentSize = m_pCon->Write(m_pExBuffer, m_nExSize, m_nSent - m_nMsgSize);
			if (nSentSize < 0)
			{
				return NetworkResult::Error;
			}
			else
			{
				m_nSent = m_nMsgSize + nSentSize;
			}
		}
	}
	else
	{
		// send normal data.
		m_nSent = m_pCon->Write(m_pMsgBuffer, m_nMsgSize, m_nSent);
		// when send nurmal data succeeded, try send exdata in one time.
		if (m_nSent != -1 && m_nSent == m_nMsgSize && m_nExSize > 0)
		{
			int nSentSize = m_pCon->Write(m_pExBuffer, m_nExSize, 0);
			if (nSentSize < 0)
			{
				return NetworkResult::Error;
			}
			else
			{
				m_nSent = m_nMsgSize + nSentSize;
			}
		}
	}
	m_pLog->Verbos(CUniversal::Format("Send Info : AllSize[%d],ExSize[%d],Sent[%d]", m_nPackSize, m_nExSize, m_nSent));

	// check send result.
	// send done, remove the is sent data and try send next package.
	if (m_nSent < m_nPackSize)
	{
		return NetworkResult::Retry;
	}
	else
	{
		m_pLog->Verbos(CUniversal::Format("Message package send succeed, remove from send list. All size[%d]", m_nSent));
		return NetworkResult::Succeed;
	}
}

NetworkResult Sloong::CDataTransPackage::RecvPackage(ULONG dtlen)
{

	char *data = new char[dtlen + 1];
	memset(data, 0, dtlen + 1);

	// TODO: 对于超时时间不需要这样频繁设置，可以直接在连接建立之后由该连接自己保存
	//nRecvSize = m_pCon->Read(data, dtlen, m_pConfig->m_nReceiveTimeout, true);//一次性接受所有消息
	int nRecvSize = m_pCon->Read(data, dtlen, 5, true); //一次性接受所有消息
	if (nRecvSize < 0)
	{
		return NetworkResult::Error;
	}
	else if (nRecvSize != (int)dtlen)
	{
		m_pLog->Warn(CUniversal::Format("Receive all data is timeout. recved lenght %d, data length %d", nRecvSize, dtlen));
		return NetworkResult::Error;
	}

	// TODO: 这里对于优先级的设置以及使用还是有一些问题的。目前优先级只是用在了发送，处理并未使用到优先级
	const char *pMsg = NULL;
	// check the priority level
	if (g_pConfig->m_nPriorityLevel != 0)
	{
		char pLevel[2] = {0};
		pLevel[0] = data[0];
		int level = pLevel[0];
		if (level > g_pConfig->m_nPriorityLevel || level < 0)
		{
			m_pLog->Error(CUniversal::Format("Receive priority level error. the data is %d, the config level is %d. add this message to last list", level, g_pConfig->m_nPriorityLevel));
			return NetworkResult::Error;
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

	// TODO: 这里对于优先级，以及流水号和MD5这些的配置项有些不太合理，这里先暂时直接在构造函数中传进来，后面考虑怎么优化处理
	if (g_pConfig->m_bEnableSwiftNumberSup)
	{
		// TODO: 接收长度信息这个可以直接在lConnect这个类里直接集成
		char pLongBuffer[s_llLen + 1] = {0};
		memcpy(pLongBuffer, pMsg, s_llLen);
		m_nSerialNumber = CUniversal::BytesToLong(pLongBuffer);
		pMsg += s_llLen;
	}

	if (g_pConfig->m_bEnableMD5Check)
	{
		strMD5 = string(pMsg,32);
		pMsg += 32;
	}

	strMessage = string(pMsg);
	SAFE_DELETE_ARR(data);

	if (g_pConfig->m_bEnableMD5Check)
	{
		string rmd5 = CMD5::Encode(strMessage);
		CUniversal::touper(strMD5);
		CUniversal::touper(rmd5);
		if (strMD5 != rmd5)
		{
			// handle error.
			string strSend = CUniversal::Format("{\"errno\": \"-1\",\"errmsg\" : \"package check error\",\"server_md5\":\"%s\",\"client_md5\":\"%s\",\"check_string\":\"%s\"}", rmd5, strMD5, strMessage);
			ResponsePackage(strSend);
			auto event = make_shared<CNetworkEvent>(MSG_TYPE::MonitorSendStatus);
			event->SetSocketID(m_pCon->GetSocketID());
			m_iMsg->SendMessage(event);
			return NetworkResult::Invalid;
		}
	}

	if (g_pConfig->m_oLogInfo.ShowReceiveMessage)
		m_pLog->Verbos(CUniversal::Format("RECV<<<[%d][%s]<<<%s",m_nSerialNumber,strMD5, strMessage));

	return NetworkResult::Succeed;
}
