#include "EasyConnect.h"
#include <sys/socket.h>
#include "utility.h"
#include "defines.h"

// openssl head file
#include <openssl/ssl.h>
#include <openssl/err.h>


bool support_ssl_reconnect = false;

// If want use the int64 to send the length data, define this.
//#define USE_INT64_LENGTH_SIZE	1
constexpr int s_llLen = 8;
constexpr int s_lLen = 4;

void Sloong::EasyConnect::Initialize(SOCKET sock, LPVOID ctx)
{
	m_nSocket = sock;
	m_strAddress = CUtility::GetSocketIP(m_nSocket);
	m_nPort = CUtility::GetSocketPort(m_nSocket);

	if (ctx)
	{
		auto pCtx = TYPE_TRANS<SSL_CTX*>(ctx);
		auto pSSL = SSL_new(pCtx);
		SSL_set_fd(pSSL, sock);
		SSL_set_accept_state(pSSL);
		m_pSSL = pSSL;
		do_handshake();
	}
}

void Sloong::EasyConnect::Initialize(const string &address, int port, LPVOID ctx)
{
	m_strAddress = address;
	m_nPort = port;
	if (ctx)
	{
		auto pCtx = TYPE_TRANS<SSL_CTX*>(ctx);
		m_pSSL = SSL_new(pCtx);
	}
}

bool Sloong::EasyConnect::Connect()
{
	struct sockaddr_in remote_addr;
	memset(&remote_addr, 0, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr(m_strAddress.c_str());
	remote_addr.sin_port = htons((uint16_t)m_nPort);
	if ((m_nSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		return false;
	}
	if (connect(m_nSocket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0)
	{
		perror("connect error");
		return false;
	}
	if (m_pSSL)
	{
		auto pSSL = TYPE_TRANS<SSL*>(m_pSSL);
		SSL_set_fd(pSSL, m_nSocket);
		SSL_set_accept_state(pSSL);
		do_handshake();
	}
	return true;
}

int Sloong::EasyConnect::SSL_Read_Ex( char *buf, int nSize, int nTimeout, bool bAgagin)
{
	if (nSize <= 0)
		return 0;

	auto pSSL = TYPE_TRANS<SSL*>(m_pSSL);
	int nIsRecv = 0;
	int nNoRecv = nSize;
	int nRecv = 0;
	char *pBuf = buf;
	while (nIsRecv < nSize)
	{
		nRecv = SSL_read(pSSL, pBuf + nSize - nNoRecv, nNoRecv);
		if (nRecv <= 0)
		{
			return nIsRecv;
		}
		nNoRecv -= nRecv;
		nIsRecv += nRecv;
	}
	return nIsRecv;
}

int Sloong::EasyConnect::SSL_Write_Ex(const char *buf, int len)
{
	auto pSSL = TYPE_TRANS<SSL*>(m_pSSL);
	return SSL_write(pSSL,buf,len);
}

string Sloong::EasyConnect::GetLengthData(int64_t lengthData)
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

ResultType Sloong::EasyConnect::SendPackage(const string &data, int &nSentSize)
{
	auto nCurSent = 0;
	if (nSentSize == 0)
	{
		auto lendata = GetLengthData(data.size());
		int nLen = lendata.size();
		lendata.append(data);
		nCurSent = Write(lendata, 0);
		// In first time send. the length data must send succeed. 
		if (nCurSent > nLen)
			nSentSize = nCurSent - nLen;
		else
			return ResultType::Error;
	}
	else
	{
		nCurSent = Write(data, nSentSize);
		if (nCurSent > 0)
			nSentSize += nCurSent;
	}
	if (nCurSent > 0)
	{
		if (nSentSize == (int)data.length())
			return ResultType::Succeed;
		else
			return ResultType::Retry;
	}
	else if (nCurSent == -11)
		return ResultType::Retry;
	else
		return ResultType::Error;
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

ResultType Sloong::EasyConnect::RecvPackage(string &res, int &nPackage, int &nReceivedSize, bool block)
{
	if (nPackage == 0)
	{
		nPackage = RecvLengthData(block);

		if (nPackage == -11)
			return ResultType::Warning;
		else if (nPackage <= 0)
			return ResultType::Error;
	}
	
	string buf;
	buf.resize(nPackage - nReceivedSize);
	auto len = Read(buf.data(), (int)buf.size(), block, false);
	if (len > 0)
	{
		nReceivedSize += len;
		res.append(buf);
		if (nReceivedSize == nPackage)
			return ResultType::Succeed;
		else
			return ResultType::Retry;
	}
	else if (len == -11)
		return ResultType::Retry;
	else
		return ResultType::Error;
}

int Sloong::EasyConnect::Read(char *data, int len, bool block, bool bagain)
{
	// 未启用SSL时直接发送数据
	if (!m_pSSL)
	{
		int recvSize = 0;
		if (block)
			recvSize = CUniversal::RecvTimeout(m_nSocket, data, len, 0, bagain);
		else
			recvSize = CUniversal::RecvEx(m_nSocket, data, len, bagain);

		if (recvSize < 0)
			m_nErrno = 0 - recvSize;
		return recvSize;
	}

	if (!CheckSSLStatus(true))
	{
		return 0;
	}

	// SSL发送数据
	// 这里可能会有以下几种情况。
	// 1.正常全部读取完成。
	// 2.读取后发生错误，错误信息为SSL_ERROR_WANT_READ，需等待下次可读事件，并根据已读的部分进行组合。
	// 3.读取后发生错误，错误信息为其他，认为连接发生问题需要重连。
	int ret = SSL_Read_Ex( data, len, 0, true);
	if (ret != len)
	{
		auto pSSL = TYPE_TRANS<SSL*>(m_pSSL);
		int err = SSL_get_error(pSSL, ret);
		switch (err)
		{
		case SSL_ERROR_WANT_READ:
			m_stStatus = ConnectStatus::WaitRead;
			break;
		case SSL_ERROR_WANT_WRITE:
			if (support_ssl_reconnect)
			{
				m_stStatus = ConnectStatus::WaitWrite;
			}
			else
			{
				m_stStatus = ConnectStatus::ConnectError;
			}
			break;
		default:
			m_stStatus = ConnectStatus::ConnectError;
			break;
		}
	}
	if (m_stStatus == ConnectStatus::ConnectError)
		return -1;
	else
		return ret;
}

int Sloong::EasyConnect::Write(const char *data, int len, int index)
{
	// 未启用SSL时直接发送数据
	if (!m_pSSL)
	{
		auto sendSize = CUniversal::SendEx(m_nSocket, data, len, index);
		if (sendSize < 0)
			m_nErrno = 0 - sendSize;
		return sendSize;
	}

	if (!CheckSSLStatus(false))
	{
		return 0;
	}

	// SSL发送数据
	int ret = SSL_Write_Ex(data + index, len);
	if (ret > 0)
	{
		if (ret != len)
			m_stStatus = ConnectStatus::WaitWrite;

		return ret;
	}
	else if (ret < 0)
	{
		auto pSSL = TYPE_TRANS<SSL*>(m_pSSL);
		int err = SSL_get_error(pSSL, ret);

		switch (err)
		{
		case SSL_ERROR_WANT_WRITE:
			m_stStatus = ConnectStatus::WaitWrite;
			return 0;
			break;
		case SSL_ERROR_WANT_READ:
			if (support_ssl_reconnect)
			{
				m_stStatus = ConnectStatus::WaitRead;
				return 0;
			}
			else
			{
				m_stStatus = ConnectStatus::ConnectError;
				return -1;
			}
			break;
		default:
			m_stStatus = ConnectStatus::ConnectError;
			return -1;
			break;
		}
	}
	else
	{
		m_stStatus = ConnectStatus::ConnectError;
		return -1;
	}
}

void Sloong::EasyConnect::Close()
{
	if (m_pSSL)
	{
		auto pSSL = TYPE_TRANS<SSL*>(m_pSSL);
		SSL_shutdown(pSSL);
		SSL_free(pSSL);
	}
	shutdown(m_nSocket, SHUT_RDWR);
	close(m_nSocket);
}

unsigned long Sloong::EasyConnect::G_InitializeSSL(LPVOID* out_ctx, const string &certFile, const string &keyFile, const string &passwd)
{
	SSL_library_init();
	OpenSSL_add_all_algorithms();			   /* load & register all cryptos, etc. */
	SSL_load_error_strings();				   /* load all error messages */
	auto ctx = SSL_CTX_new(SSLv23_server_method()); /* create new server-method instance */
	if (ctx == NULL)
	{
		return ERR_get_error();
	}

	if (!passwd.empty())
		SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *)passwd.c_str());

	//New lines
	if (SSL_CTX_load_verify_locations(ctx, certFile.c_str(), keyFile.c_str()) != 1)
		return ERR_get_error();
	if (SSL_CTX_set_default_verify_paths(ctx) != 1)
		return ERR_get_error();
	//End new lines
	/* set the local certificate from CertFile */
	if (SSL_CTX_use_certificate_file(ctx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		return ERR_get_error();
	}
	/* set the private key from KeyFile (may be the same as CertFile) */
	if (SSL_CTX_use_PrivateKey_file(ctx, keyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		return ERR_get_error();
	}
	/* verify private key */
	if (!SSL_CTX_check_private_key(ctx))
	{
		return ERR_get_error();
	}
	*out_ctx = ctx;
	return S_OK;
}

void Sloong::EasyConnect::G_FreeSSL(LPVOID ctx)
{
	if( ctx )
	{
		SSL_CTX_free(TYPE_TRANS<SSL_CTX*>(ctx));
	}
}

string Sloong::EasyConnect::G_FormatSSLErrorMsg(int code)
{
	return string(ERR_error_string(code, NULL));
}

bool Sloong::EasyConnect::do_handshake()
{
	auto pSSL = TYPE_TRANS<SSL*>(m_pSSL);
	int res = SSL_do_handshake(pSSL);
	if (1 == res)
	{
		m_stStatus = ConnectStatus::Ready;
		return true;
	}
	switch (SSL_get_error(pSSL, res))
	{
	case SSL_ERROR_WANT_WRITE:
		//m_stStatus = ConnectStatus::WaitWrite;
		break;
	case SSL_ERROR_WANT_READ:
		//m_stStatus = ConnectStatus::WaitRead;
		break;
	default:
		m_stStatus = ConnectStatus::ConnectError;
		break;
	}
	return false;
}

bool Sloong::EasyConnect::CheckSSLStatus(bool bRead)
{
	if (!m_pSSL)
		return true;

	switch (m_stStatus)
	{
	case ConnectStatus::Disconnect:
		return do_handshake();
		break;
	case ConnectStatus::WaitRead:
		if (bRead)
			return true;
		else
			return false;
		break;
	case ConnectStatus::WaitWrite:
		if (!bRead)
			return true;
		else
			return false;
		break;
	default:
		return true;
		break;
	}
}
