#include "DataTransPackage.h"

#include "main.h"

const int g_nMD5Length = 32;

void Sloong::CDataTransPackage::Initialize(SmartConnect conn, CLog* pLog)
{
	m_pCon = conn;	
	m_pLog = pLog;
}

void Sloong::CDataTransPackage::PrepareSendPackageData( const string& msg, const char* pExData , int nExSize)
{
	// calculate the send buffer length
	long long nBufLen = msg.size();
	if( m_emProperty & DataTransPackageProperty::EnablePriorityLevel )
	{
		nBufLen += 1;
	}
	if( m_emProperty & DataTransPackageProperty::EnableSerialNumber ){
		nBufLen += s_llLen;
	}
	if( m_emProperty & DataTransPackageProperty::EnableMD5Check )
	{
		nBufLen += g_nMD5Length;
	}
	if (pExData != NULL && nExSize > 0)
	{
		nBufLen += s_llLen;
	}

	string logString = "SEND<<<";

	m_szMsgBuffer.resize(nBufLen);
	char *pCpyPoint = m_szMsgBuffer.data();

	// copy the data to buffer. the order is Priority -> SerialNum -> MD5
	if( m_emProperty & DataTransPackageProperty::EnablePriorityLevel ){
		*pCpyPoint = (char)m_nPriority;
		logString += CUniversal::Format("[%d]",m_nPriority);
		pCpyPoint += 1;
	}
	if( m_emProperty & DataTransPackageProperty::EnableSerialNumber ){
		CUniversal::Int64ToBytes(m_nSerialNumber, pCpyPoint);
		logString += CUniversal::Format("[%d]",m_nSerialNumber);
		pCpyPoint += s_llLen;
	}
	if( m_emProperty & DataTransPackageProperty::EnableMD5Check ){
		string md5 = CMD5::Encode(msg);
		memcpy(pCpyPoint, md5.c_str(), md5.length());
		logString += CUniversal::Format("[%s]",md5);
		pCpyPoint += md5.length();
	}
	
	memcpy(pCpyPoint, msg.c_str(), msg.length());
	pCpyPoint += msg.length();

	if (pExData != NULL && nExSize > 0)
	{
		long long Exlen = nExSize;
		CUniversal::Int64ToBytes(Exlen, pCpyPoint);
		logString += CUniversal::Format("&&&EXDATA<<<[%d]",nExSize);
		pCpyPoint += 8;
	}

	logString += "<<<" + msg;
	if (m_pLog!= nullptr)
	{
		m_pLog->Verbos(logString);
	}

	m_pExBuffer = pExData;
	m_nExSize = nExSize;
}


void Sloong::CDataTransPackage::RequestPackage(  const string& msg )
{
	PrepareSendPackageData(msg,nullptr,0);
}

void Sloong::CDataTransPackage::ResponsePackage(const string &msg, const char *pExData, int nExSize)
{
	PrepareSendPackageData(msg,pExData,nExSize);
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
	
	const char* pMsg = result.data();
	int msgLength = result.length();

	if( m_emProperty & DataTransPackageProperty::EnablePriorityLevel ){
		int m_nPriority = pMsg[0];
		if (m_nPriority > s_PriorityLevel || m_nPriority < 0)
		{
			if( m_pLog )
				m_pLog->Error(CUniversal::Format("Receive priority level error. the data is %d, the config level is %d. add this message to last list", m_nPriority, s_PriorityLevel));
			return NetworkResult::Error;
		}
		pMsg += 1;
		msgLength -= 1;
	}
	if( m_emProperty & DataTransPackageProperty::EnableSerialNumber ){
		// TODO： 这里直接把pMsg传进去应该也是可以的
		char pLongBuffer[s_llLen + 1] = {0};
		memcpy(pLongBuffer, pMsg, s_llLen);
		m_nSerialNumber = CUniversal::BytesToInt64(pLongBuffer);
		pMsg += s_llLen;
		msgLength -= s_llLen;
	}
	if( m_emProperty & DataTransPackageProperty::EnableMD5Check ){
		m_strMD5 = string(pMsg,g_nMD5Length);
		pMsg += g_nMD5Length;
		msgLength -= g_nMD5Length;
	}

	m_strMessage = string(pMsg,msgLength);

	if( m_emProperty & DataTransPackageProperty::EnableMD5Check ){
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
	}

	if( m_pLog )
		m_pLog->Verbos(CUniversal::Format("RECV<<<[%d][%s]<<<%s",m_nSerialNumber,m_strMD5, m_strMessage));

	return NetworkResult::Succeed;
}
