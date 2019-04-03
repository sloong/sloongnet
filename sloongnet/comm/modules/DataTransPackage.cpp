#include "DataTransPackage.h"

#include "main.h"

void Sloong::CDataTransPackage::Initialize(SmartConnect conn, CLog* pLog)
{
	m_pCon = conn;	
	m_pLog = pLog;
}

void Sloong::CDataTransPackage::PrepareSendPackageData( const string& msg, int priorityLevel, const char* pExData , int nExSize)
{
	// get the message md5
	string md5 = CMD5::Encode(msg);

	// calculate the send buffer length
	long long nBufLen = s_llLen + md5.length() + msg.size();
	if (pExData != NULL && nExSize > 0)
	{
		nBufLen += s_llLen;
	}
	if( priorityLevel != -1 )
	{
		nBufLen += 1;
	}

	if (m_pLog!= nullptr)
	{
		m_pLog->Verbos(CUniversal::Format("SEND<<<[%d][%s]<<<%s",m_nSerialNumber,md5,msg));
		if( pExData != nullptr )
			m_pLog->Verbos(CUniversal::Format("SEND_EXDATA<<<[%d]<<<DATALEN[%d]",m_nSerialNumber,nExSize));
	}

	m_szMsgBuffer.resize(nBufLen);
	char *pCpyPoint = m_szMsgBuffer.data();

	/*long long nMsgLen = nBufLen - s_llLen;
	CUniversal::Int64ToBytes(nMsgLen, pCpyPoint);
	pCpyPoint += 8;*/
	if( priorityLevel != -1 )
	{
		*pCpyPoint = (char)priorityLevel;
		pCpyPoint += 1;
	}
	
	CUniversal::Int64ToBytes(m_nSerialNumber, pCpyPoint);
	pCpyPoint += s_llLen;
	
	memcpy(pCpyPoint, md5.c_str(), md5.length());
	pCpyPoint += md5.length();
	
	memcpy(pCpyPoint, msg.c_str(), msg.length());
	pCpyPoint += msg.length();

	if (pExData != NULL && nExSize > 0)
	{
		long long Exlen = nExSize;
		CUniversal::Int64ToBytes(Exlen, pCpyPoint);
		pCpyPoint += 8;
	}

	m_pExBuffer = pExData;
	m_nExSize = nExSize;
}


void Sloong::CDataTransPackage::RequestPackage( int SerialNumber, int priorityLevel, const string& msg)
{
	m_nSerialNumber = SerialNumber;
	PrepareSendPackageData(msg,priorityLevel,nullptr,0);
}

void Sloong::CDataTransPackage::ResponsePackage(const string &msg, const char *pExData, int nExSize)
{
	PrepareSendPackageData(msg,0,pExData,nExSize);
}




/**
 * @Remarks: 
 * @Params: 
 * @Return: if send succeed, return true. need listen read message. 
 *          else return false, need listen write message.
 */
NetworkResult Sloong::CDataTransPackage::SendPackage()
{
	int nMsgSize = m_szMsgBuffer.length();
	// 首先检查是不是已经发送过部分的数据了
	if (m_nSent > 0)
	{
		// 先检查普通数据发送状态
		if (m_nSent < nMsgSize)
		{
			int nSentSize = m_pCon->SendPackage(m_szMsgBuffer,m_nSent);
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
		if (m_nSent >= nMsgSize && m_nExSize > 0)
		{
			int nSentSize = m_pCon->Write(m_pExBuffer, m_nExSize, m_nSent - nMsgSize);
			if (nSentSize < 0)
			{
				return NetworkResult::Error;
			}
			else
			{
				m_nSent = nMsgSize + nSentSize;
			}
		}
	}
	else
	{
		// send normal data.
		m_nSent = m_pCon->SendPackage(m_szMsgBuffer, m_nSent);
		// when send nurmal data succeeded, try send exdata in one time.
		if (m_nSent != -1 && m_nSent == nMsgSize && m_nExSize > 0)
		{
			int nSentSize = m_pCon->Write(m_pExBuffer, m_nExSize, 0);
			if (nSentSize < 0)
			{
				return NetworkResult::Error;
			}
			else
			{
				m_nSent = nMsgSize + nSentSize;
			}
		}
	}
	if( m_pLog )
		m_pLog->Verbos(CUniversal::Format("Send Info : AllSize[%d],ExSize[%d],Sent[%d]", m_nPackSize, m_nExSize, m_nSent));

	// check send result.
	// send done, remove the is sent data and try send next package.
	if (m_nSent < m_nPackSize)
	{
		return NetworkResult::Retry;
	}
	else
	{
		if( m_pLog )
			m_pLog->Verbos(CUniversal::Format("Message package send succeed, remove from send list. All size[%d]", m_nSent));
		return NetworkResult::Succeed;
	}
}

NetworkResult Sloong::CDataTransPackage::RecvPackage()
{
	string result;
	if( !m_pCon->RecvPackage(result))
		return NetworkResult::Error;
	
	const char* data = result.data();


	// TODO: 这里对于优先级的设置以及使用还是有一些问题的。目前优先级只是用在了发送，处理并未使用到优先级
	const char *pMsg = NULL;
	
		char pLevel[2] = {0};
		pLevel[0] = data[0];
		int level = pLevel[0];
		if (level > s_PriorityLevel || level < 0)
		{
			if( m_pLog )
				m_pLog->Error(CUniversal::Format("Receive priority level error. the data is %d, the config level is %d. add this message to last list", level, s_PriorityLevel));
			return NetworkResult::Error;
		}
		else
		{
			nPriority = level;
		}
		pMsg = &data[1];
	

	// TODO: 这里对于优先级，以及流水号和MD5这些的配置项有些不太合理，这里先暂时直接在构造函数中传进来，后面考虑怎么优化处理
	
		// TODO: 接收长度信息这个可以直接在lConnect这个类里直接集成
		char pLongBuffer[s_llLen + 1] = {0};
		memcpy(pLongBuffer, pMsg, s_llLen);
		m_nSerialNumber = CUniversal::BytesToInt64(pLongBuffer);
		pMsg += s_llLen;
	

	
		m_strMD5 = string(pMsg,32);
		pMsg += 32;
	

	m_strMessage = string(pMsg,result.length()-1-s_llLen-32);

	
		string rmd5 = CMD5::Encode(m_strMessage);
		CUniversal::touper(m_strMD5);
		CUniversal::touper(rmd5);
		if (m_strMD5 != rmd5)
		{
			// handle error.
			string strSend = CUniversal::Format("{\"errno\": \"-1\",\"errmsg\" : \"package check error\",\"server_md5\":\"%s\",\"client_md5\":\"%s\",\"check_string\":\"%s\"}", rmd5, m_strMD5, m_strMessage);
			ResponsePackage(strSend);
			return NetworkResult::Invalid;
		}

	if( m_pLog )
		m_pLog->Verbos(CUniversal::Format("RECV<<<[%d][%s]<<<%s",m_nSerialNumber,m_strMD5, m_strMessage));

	return NetworkResult::Succeed;
}
