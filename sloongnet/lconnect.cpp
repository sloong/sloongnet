#include "lconnect.h"
#include <univ/log.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

bool support_ssl_reconnect = false;

Sloong::lConnect::lConnect(CLog* log)
{
	m_pLog = log;
}


Sloong::lConnect::~lConnect()
{
	Close();
}

void Sloong::lConnect::Initialize(int sock, SSL_CTX* ctx /*= nullptr*/)
{
	m_nSocket = sock;

	if (ctx)
	{
		m_pSSL = SSL_new(ctx);
		SSL_set_fd(m_pSSL, sock);

		SSL_set_accept_state(m_pSSL);
		int ret = SSL_do_handshake(m_pSSL);
		if (ret == 1)
		{
			m_stStatus = ConnectStatus::Ready;
		}
		else
		{
			int err = SSL_get_error(m_pSSL, ret);
			string str = ERR_error_string(err, NULL);
			m_pLog->Error(str);
			m_stStatus = ConnectStatus::Disconnect;
		}
	}
	
}



int Sloong::lConnect::Read(char * data, int len, int timeout, bool bagain)
{
	// 未启用SSL时直接发送数据
	if (!m_pSSL)
		return CUniversal::RecvEx(m_nSocket, data, len, timeout, bagain);

	if (!CheckSSLStatus(true))
	{
		return 0;
	}

	// SSL发送数据
	int ret = SSL_read(m_pSSL, data, len);
	if (ret > 0)
	{
		if (ret != len)
			m_stStatus = ConnectStatus::WaitRead;

		return ret;
	}
	else if (ret < 0)
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
				m_stStatus = ConnectStatus::Error;
			}
			break;
		default:
			m_stStatus = ConnectStatus::Error;
			break;
		}
		return 0;
	}
	else
	{
		m_stStatus = ConnectStatus::Error;
	}
}

int Sloong::lConnect::Write(const char* data, int len, int index)
{
	// 未启用SSL时直接发送数据
	if (!m_pSSL)	
		return CUniversal::SendEx(m_nSocket, data, len, index);

	
	if (!CheckSSLStatus(false))
	{
		return 0;
	}

	// SSL发送数据
	int ret = SSL_write(m_pSSL, data, len);
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
			break;
		case SSL_ERROR_WANT_READ:
			if (support_ssl_reconnect)
			{
				m_stStatus = ConnectStatus::WaitRead;
			}
			else
			{
				m_stStatus = ConnectStatus::Error;
			}
			break;
		default:
			m_stStatus = ConnectStatus::Error;
			break;
		}
		
	}
	else
	{
		m_stStatus = ConnectStatus::Error;
	}
	
}

void Sloong::lConnect::Close()
{
	if (m_pSSL)
	{
		SSL_shutdown(m_pSSL);
		SSL_free(m_pSSL);
	}
	close(m_nSocket);
}

int Sloong::lConnect::GetSocket()
{
	return m_nSocket;
}

int Sloong::lConnect::G_InitializeSSL(SSL_CTX** ctx, string certFile, string keyFile, string passwd)
{
	SSL_CTX* p_ctx = (*ctx);
	SSL_library_init();
	OpenSSL_add_all_algorithms();/* load & register all cryptos, etc. */
	SSL_load_error_strings();/* load all error messages */
	p_ctx = SSL_CTX_new(TLSv1_server_method());/* create new server-method instance */
	if (p_ctx == NULL)
	{
		return ERR_get_error();
	}

	SSL_CTX_set_default_passwd_cb_userdata(p_ctx, (void*)passwd.c_str());
	//New lines
	if (SSL_CTX_load_verify_locations(p_ctx, certFile.c_str(), keyFile.c_str()) != 1)
		return ERR_get_error();
	if (SSL_CTX_set_default_verify_paths(p_ctx) != 1)
		return ERR_get_error();
	//End new lines
	/* set the local certificate from CertFile */
	if (SSL_CTX_use_certificate_file(p_ctx, certFile.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		return ERR_get_error();
	}
	/* set the private key from KeyFile (may be the same as CertFile) */
	if (SSL_CTX_use_PrivateKey_file(p_ctx, keyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		return ERR_get_error();
	}
	/* verify private key */
	if (!SSL_CTX_check_private_key(p_ctx))
	{
		return ERR_get_error();
	}
	return S_OK;
}

string Sloong::lConnect::G_FormatSSLErrorMsg(int code)
{
	return string(ERR_error_string(code, NULL));
}

bool Sloong::lConnect::do_handshake()
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
		m_stStatus = ConnectStatus::WaitWrite;
		break;
	case SSL_ERROR_WANT_READ:
		m_stStatus = ConnectStatus::WaitRead;
		break;
	default:
		m_stStatus = ConnectStatus::Error;
		break;
	}
	return false;
}

bool Sloong::lConnect::CheckSSLStatus(bool bRead)
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
