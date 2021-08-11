/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2018-01-12 15:25:16
 * @LastEditTime: 2021-03-25 17:43:26
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/EasyConnect.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 自动重连运行逻辑：发送/接收后检查操作结果，如果是无法恢复的错误（非超时，重试），
 * 那么直接视为链接发生异常，需要进行重新连接。考虑到链接断开后，双方的读写队列都已经清空，
 * 所以这里不增加自动重试（自动尝试重新进行发送/接收数据）。只是将标记设置为链接断开。然后返回错误信息。
 * 待到下次重新进行操作的时候，首先检查标记，如果不是已连接，那么直接开始进行连接操作。连接成功之后，调用注册的回调函数
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#include "EasyConnect.h"
#include <sys/socket.h>
#include "utility.h"
#include "Helper.h"

// If want use the int64 to send the length data, define this.
//#define USE_INT64_LENGTH_SIZE	1
#ifdef USE_INT64_LENGTH_SIZE
constexpr int s_llLen = 8;
#else
constexpr int s_lLen = 4;
#endif

CResult Sloong::EasyConnect::InitializeAsServer(CLog *log, SOCKET sock, LPVOID ctx)
{
	m_pLog = log;
	m_bSupportReconnect = false;
	m_nSocket = sock;
	m_strAddress = CUtility::GetSocketIP(m_nSocket);
	m_nPort = CUtility::GetSocketPort(m_nSocket);
	m_nHashCode = std::hash<string>{}(Helper::Format("%s:%d", m_strAddress.c_str(), m_nPort));

	if (ctx)
	{
		m_pSSL = make_unique<SSLHelper>(ctx);
		m_pSSL->Initialize(sock);
	}
	return CResult::Succeed;
}

CResult Sloong::EasyConnect::InitializeAsClient(CLog *log, const string &address, int port, LPVOID ctx)
{
	m_pLog = log;
	m_bSupportReconnect = true;
	m_strAddress = address;
	m_nPort = port;
	m_nHashCode = std::hash<string>{}(Helper::Format("%s:%d", m_strAddress.c_str(), m_nPort));

	if (ctx)
	{
		m_pSSL = make_unique<SSLHelper>(ctx);
	}
	return Connect();
}
CResult Sloong::EasyConnect::Connect()
{
	if (m_nSocket != INVALID_SOCKET)
		return CResult::Make_Warning("Called connect function, but the socket isn't INVALID_SOCKET.");
	if (!m_bSupportReconnect)
		return CResult::Make_Error("Connection is disconnect. and this connection don't support reconnect.");

	auto dns_res = CUtility::HostnameToIP(m_strAddress);
	if (dns_res.IsFialed())
	{
		return move(dns_res);
	}
	auto list = dns_res.GetResultObject();

	if (m_pLog)
		m_pLog->Debug(Helper::Format("Connect to %s:%d.", list[0].c_str(), m_nPort));

	struct sockaddr_in remote_addr;
	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(list[0].c_str());
	remote_addr.sin_port = htons((uint16_t)m_nPort);
	if ((m_nSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		return CResult::Make_Error("socket error");
	}
	if (connect(m_nSocket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0)
	{
		return CResult::Make_Error("connect error");
	}
	if (m_pSSL)
	{
		auto res = m_pSSL->Initialize(m_nSocket);
		if (res.IsFialed())
			return res;
	}

	if (m_pOnReconnect.size() > 0)
		for_each(m_pOnReconnect.begin(), m_pOnReconnect.end(), [&](auto func)
				 { func(m_nHashCode, m_nInvalidSocket, m_nSocket); });
	return CResult::Succeed;
}

string Sloong::EasyConnect::GetLengthData(uint64_t lengthData)
{
	string szLengthData;
#ifdef USE_INT64_LENGTH_SIZE
	char m_pMsgBuffer[s_llLen] = {0};
	char *pCpyPoint = m_pMsgBuffer;
	Helper::Int64ToBytes(lengthData, pCpyPoint);
	szLengthData = string(m_pMsgBuffer, s_llLen);
#else
	char m_pMsgBuffer[s_lLen] = {0};
	char *pCpyPoint = m_pMsgBuffer;
	Helper::Int32ToBytes((uint32_t)lengthData, pCpyPoint);
	szLengthData = string(m_pMsgBuffer, s_lLen);
#endif
	return szLengthData;
}

/**
 * @Description: 
 * @Params: 
 * @Return: if send succeed, return true. need listen read message. 
 *          else return false, need listen write message.
 */
CResult Sloong::EasyConnect::SendPackage(UniquePackage pack)
{
	if (m_nSocket == INVALID_SOCKET)
	{
		auto res = Connect();
		if (res.IsFialed())
			return res;
	}
	if (m_strSending.empty())
	{
		string tmp;
		if (!pack->SerializeToString(&tmp))
		{
			return CResult::Make_Error("Package serialize fialed.");
		}
		unsigned char buffer[32] = {0};
		CSHA256::Binary_Encoding(tmp, buffer);
		pack->set_hash(buffer, 32);
		if (!pack->SerializeToString(&m_strSending))
		{
			return CResult::Make_Error("Package serialize fialed.");
		}
		m_SendPackageSize = m_strSending.size();
		m_SentSize = 0;
	}
	else if (!m_strSending.empty() && pack != nullptr)
	{
		return CResult::Make_Error("Prev package is no sending done! Make sure the send status is checked before sending new package.");
	}

	if (m_SendPackageSize == 0)
	{
		return CResult(ResultType::Invalid, "Cannot send message. the send data is zero");
	}

	auto nCurSent = 0;
	if (m_SentSize == 0)
	{
		auto lendata = GetLengthData(m_strSending.size());
		int nLen = lendata.size();
		lendata.append(m_strSending);
		nCurSent = Write(lendata, 0);
		// In first time send. the length data must send succeed.
		if (nCurSent >= nLen)
		{
			m_SentSize = nCurSent - nLen;
		}
		else if (nCurSent == -11)
		{
			return CResult(ResultType::Retry);
		}
		else
		{
			auto msg = Helper::Format("Error when send length data. Socket[%d][%s:%d].Errno[%d].", m_nSocket, m_strAddress.c_str(), m_nPort, GetErrno());
			Close();
			return CResult::Make_Error(msg);
		}
	}
	else
	{
		nCurSent = Write(m_strSending, m_SentSize);
		if (nCurSent > 0)
			m_SentSize += nCurSent;
	}
	if (nCurSent > 0)
	{
		if (m_SentSize == (int)m_strSending.length())
		{
			m_strSending.clear();
			m_SendPackageSize = 0;
			m_SentSize = 0;
			return CResult::Succeed;
		}
		else
			return CResult(ResultType::Retry);
	}
	else if (nCurSent == -11)
	{
		return CResult(ResultType::Retry);
	}
	else
	{
		auto msg = Helper::Format("Error when send data. Socket[%d][%s:%d].Errno[%d].", m_nSocket, m_strAddress.c_str(), m_nPort, GetErrno());
		Close();
		return CResult::Make_Error(msg);
	}
}

// 成功返回数据包长度
// 超时返回0
// 错误返回-1
decltype(auto) Sloong::EasyConnect::RecvLengthData(bool block)
{
#ifdef USE_INT64_LENGTH_SIZE
	char nLen[s_llLen] = {0};
	int64_t bRes = Read(nLen, s_llLen, block, false);
	if (bRes > 1)
	{
		bRes = Helper::BytesToInt64(nLen);
	}
	return bRes;
#else
	char nLen[s_lLen] = {0};
	int32_t bRes = Read(nLen, s_lLen, block, false);
	if (bRes > 1)
	{
		bRes = Helper::BytesToInt32(nLen);
	}
	return bRes;
#endif
}

PackageResult Sloong::EasyConnect::RecvPackage(bool block)
{
	if (m_nSocket == INVALID_SOCKET)
	{
		auto res = Connect();
		if (res.IsFialed())
			return res;
	}

	if (m_RecvPackageSize == 0)
	{
		auto len = RecvLengthData(block);

		// If the data length received 11 error, ignore it.
		if (len == -11)
		{
			if (m_pLog)
				m_pLog->Verbos("Receive data package return 11[EAGAIN].");
			return PackageResult(ResultType::Ignore);
		}
		else if (len == 0)
		{
			if (m_pLog)
				m_pLog->Verbos("Receive data package return 0. socket may closed");
			return PackageResult(ResultType::Warning);
		}
		else if (len > 0)
		{
			if (m_pLog)
				m_pLog->Verbos("Receive data package succeed : " + Helper::ntos(len));
			m_RecvPackageSize = len;
		}
		else
		{
			if (m_pLog)
				m_pLog->Debug("Receive data package happened other error. close connection.");
			Close();
			return PackageResult::Make_Error("Error when receive length data.");
		}
	}

	string buf;
	buf.resize(m_RecvPackageSize - m_ReceivedSize);
	auto len = Read(buf.data(), (int)buf.size(), block, false);
	if (len > 0)
	{
		m_ReceivedSize += len;
		m_strReceiving.append(buf, 0, len);
		if (m_ReceivedSize == m_RecvPackageSize)
		{
			auto package = PackageHelper::new_unique();
			if (!package->ParseFromString(m_strReceiving))
			{
				Close();
				return PackageResult::Make_Error("Parser receive data error.");
			}
			m_strReceiving.clear();
			m_RecvPackageSize = 0;
			m_ReceivedSize = 0;

			return PackageResult::Make_OKResult(std::move(package));
		}
		else
		{
			if (m_pLog)
				m_pLog->Debug(Helper::Format("Receive package returned [Retry]. Package size[%d], Received[%d][%d]", m_RecvPackageSize, m_ReceivedSize, m_strReceiving.length()));
			return PackageResult(ResultType::Retry);
		}
	}
	else if (len == -11)
	{
		if (m_pLog)
			m_pLog->Debug(Helper::Format("Receive package returned [Retry]. Package size[%d], Received[%d]", m_RecvPackageSize, m_ReceivedSize));
		return PackageResult(ResultType::Retry);
	}
	else
	{
		Close();
		return PackageResult::Make_Error(Helper::Format("Error when receive data. Socket[%d][%s:%d].Errno[%d].", m_nSocket, m_strAddress.c_str(), m_nPort, GetErrno()));
	}
}

int Sloong::EasyConnect::Read(char *data, int len, bool block, bool bagain)
{
	// 未启用SSL时直接发送数据
	if (!m_pSSL)
	{
		int recvSize = 0;
		if (block)
			recvSize = Helper::RecvTimeout(m_nSocket, data, len, 0, bagain);
		else
			recvSize = Helper::RecvEx(m_nSocket, data, len, bagain);

		if (recvSize < 0)
			m_nErrno = 0 - recvSize;
		return recvSize;
	}
	else
	{
		auto res = m_pSSL->Read(data, len, block, bagain);
		return res.GetResultObject();
	}
}

int Sloong::EasyConnect::Write(const char *data, int len, int index)
{
	// 未启用SSL时直接发送数据
	if (!m_pSSL)
	{
		auto sendSize = Helper::SendEx(m_nSocket, data, len, index, true);
		if (sendSize < 0)
			m_nErrno = 0 - sendSize;
		return sendSize;
	}
	else
	{
		auto res = m_pSSL->Write(data, len, index);
		return res.GetResultObject();
	}
}

void Sloong::EasyConnect::Close()
{
	if (m_pLog)
		m_pLog->Debug(Helper::Format("Socket[%d] is close.", m_nSocket));
	shutdown(m_nSocket, SHUT_RDWR);
	close(m_nSocket);
	m_nInvalidSocket = m_nSocket;
	m_nSocket = INVALID_SOCKET;
}
