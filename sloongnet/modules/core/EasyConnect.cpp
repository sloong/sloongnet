#include "EasyConnect.h"
#include <sys/socket.h>
#include "utility.h"
#include "defines.h"

bool support_ssl_reconnect = false;

void Sloong::EasyConnect::Initialize(SOCKET sock, SSL_CTX* ctx, bool useLongLongSize)
{
	m_nSocket = sock;
	m_strAddress = CUtility::GetSocketIP(m_nSocket);
	m_nPort = CUtility::GetSocketPort(m_nSocket);

	if (ctx)
	{
		m_pSSL = SSL_new(ctx);
		SSL_set_fd(m_pSSL, sock);

		SSL_set_accept_state(m_pSSL);
		do_handshake();
	}
	
    m_bUseLongLongSize = useLongLongSize;
}

void Sloong::EasyConnect::Initialize( string address, int port, SSL_CTX* ctx, bool useLongLongSize)
{
    m_strAddress = address;
    m_nPort = port;
	if (ctx)
	{
		m_pSSL = SSL_new(ctx);
	}
    m_bUseLongLongSize = useLongLongSize;
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
		SSL_set_fd(m_pSSL, m_nSocket);
		SSL_set_accept_state(m_pSSL);
		do_handshake();
	}
    return true;
}

int Sloong::EasyConnect::SSL_Read_Ex(SSL* ssl, char* buf, int nSize, int nTimeout, bool bAgagin)
{
	if (nSize <= 0)
		return 0;

	int nIsRecv = 0;
	int nNoRecv = nSize;
	int nRecv = 0;
	char* pBuf = buf;
	while (nIsRecv < nSize)
	{
		nRecv = SSL_read(ssl, pBuf + nSize - nNoRecv, nNoRecv);
		if (nRecv <= 0)
		{
			return nIsRecv;
		}
		nNoRecv -= nRecv;
		nIsRecv += nRecv;
	}
	return nIsRecv;
}


int Sloong::EasyConnect::SSL_Write_Ex(SSL * ssl, char * buf, int len)
{
	return 0;
}


// 成功返回数据包长度
// 超时返回0
// 错误返回-1
long long Sloong::EasyConnect::RecvLengthData(bool block)
{
	int bRes=0;
    if( m_bUseLongLongSize ) {
        char nLen[s_llLen] = {0};
		bRes = Read(nLen, s_llLen, block,0 ,false);
        if(bRes>1){
			auto len = CUniversal::BytesToInt64(nLen);
        	return len;
		}
    }else{
        char nLen[s_lLen] = {0};
        bRes = Read(nLen, s_lLen, block,0, false);
		if (bRes>1){
			auto len = CUniversal::BytesToInt32(nLen);
        	return len;
		}
    }
	return bRes;
}

bool Sloong::EasyConnect::SendLengthData(long long lengthData)
{
	string szLengthData;
    if( m_bUseLongLongSize ) {
        char m_pMsgBuffer[s_llLen] = {0};
        char *pCpyPoint = m_pMsgBuffer;
        CUniversal::Int64ToBytes(lengthData, pCpyPoint);
        szLengthData = string(m_pMsgBuffer,s_llLen);
    }else{
        char m_pMsgBuffer[s_lLen] = {0};
        char *pCpyPoint = m_pMsgBuffer;
        CUniversal::Int32ToBytes((uint32_t)lengthData, pCpyPoint);
        szLengthData = string(m_pMsgBuffer,s_lLen);
    }
	if( Write(szLengthData,0) < 1 )
		return false;
	return true;
}


int Sloong::EasyConnect::Write(string sendData, int index)
{
	return Write(sendData.c_str(),(int)sendData.length(),index);
}

int Sloong::EasyConnect::SendPackage(string sendData, int index)
{
	if( index == 0 )
	{
		if( !SendLengthData(sendData.size()))
		return -1;
	}

	return Write(sendData, index);
}


ResultType Sloong::EasyConnect::RecvPackage(string& res,int timeOut)
{
    auto len = RecvLengthData(timeOut==0?true:false);
	// If no data can read. the res is -11. ( 0-EAGEAIN -> -11 )
	if( len == -11 )
		return ResultType::Retry;
	else if( len <= 0 )
		return ResultType::Error;
	
    res.resize(len);

	if( Read( res.data(), (int)len, false, timeOut, true) < 1)
		return ResultType::Error;

	return ResultType::Succeed;
}

int Sloong::EasyConnect::Read(char * data, int len, bool block, int timeout, bool bagain)
{
	// 未启用SSL时直接发送数据
	if (!m_pSSL)
	{
		int recvSize=0;
		if( block )
			recvSize = CUniversal::RecvTimeout(m_nSocket, data, len, 0, bagain);
		else if( timeout > 0 )
			recvSize = CUniversal::RecvTimeout(m_nSocket, data, len, timeout, bagain);
		else
			recvSize = CUniversal::RecvEx(m_nSocket, data, len, bagain);
		
		if( recvSize < 0 )
			m_nErrno = recvSize;
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
	int ret = SSL_Read_Ex(m_pSSL, data, len, 0, true);
	if (ret != len)
	{
		int err = SSL_get_error(m_pSSL, ret);
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

int Sloong::EasyConnect::Write(const char* data, int len, int index)
{
	// 未启用SSL时直接发送数据
	if (!m_pSSL)	
	{
		auto sendSize =  CUniversal::SendEx(m_nSocket, data, len, index);
		if( sendSize < 0 )
			m_nErrno = sendSize;
		return sendSize;
	}
	
	if (!CheckSSLStatus(false))
	{
		return 0;
	}

	// SSL发送数据
	int ret = SSL_write(m_pSSL, data+index, len);
	if (ret > 0)
	{
		if( ret != len )
			m_stStatus = ConnectStatus::WaitWrite;

		return ret;
	}
	else if (ret < 0)
	{
		int err = SSL_get_error(m_pSSL, ret);
		
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
		SSL_shutdown(m_pSSL);
		SSL_free(m_pSSL);
	}
	shutdown(m_nSocket, SHUT_RDWR);
	close(m_nSocket);
}

unsigned long Sloong::EasyConnect::G_InitializeSSL(SSL_CTX*& ctx, string certFile, string keyFile, string passwd)
{
	SSL_library_init();
	OpenSSL_add_all_algorithms();/* load & register all cryptos, etc. */
	SSL_load_error_strings();/* load all error messages */
	ctx = SSL_CTX_new(SSLv23_server_method());/* create new server-method instance */
	if (ctx == NULL)
	{
		return ERR_get_error();
	}

	if( !passwd.empty() )
		SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*)passwd.c_str());

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
	return S_OK;
}

string Sloong::EasyConnect::G_FormatSSLErrorMsg(int code)
{
	return string(ERR_error_string(code, NULL));
}

bool Sloong::EasyConnect::do_handshake()
{
	int res = SSL_do_handshake(m_pSSL);
	if (1 == res)
	{
		m_stStatus = ConnectStatus::Ready;
		return true;
	}
	switch (SSL_get_error(m_pSSL, res))
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
	if (!m_pSSL) return true;

	switch (m_stStatus)
	{
	case ConnectStatus::Disconnect:
		return do_handshake();
		break;
	case ConnectStatus::WaitRead:
		if (bRead) return true;
		else return false;
		break;
	case ConnectStatus::WaitWrite:
		if (!bRead) return true;
		else return false;
		break;
	default:
		return true;
		break;
	}
}
