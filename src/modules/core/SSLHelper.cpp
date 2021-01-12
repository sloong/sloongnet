/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-08-03 14:38:46
 * @LastEditTime: 2020-08-10 16:24:43
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/SSLHelper.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#include "SSLHelper.h"
#include "Helper.h"

// openssl head file
#include <openssl/ssl.h>
#include <openssl/err.h>

bool support_ssl_reconnect = false;

Sloong::SSLHelper::~SSLHelper()
{
    auto pSSL = STATIC_TRANS<SSL *>(m_pSSL);
    SSL_shutdown(pSSL);
    SSL_free(pSSL);
}

CResult Sloong::SSLHelper::Initialize(SOCKET sock)
{
    auto pCtx = STATIC_TRANS<SSL_CTX *>(m_pSSL_CTX);
    auto pSSL = SSL_new(pCtx);
    SSL_set_fd(pSSL, sock);
    SSL_set_accept_state(pSSL);
    m_pSSL = pSSL;
    do_handshake();
    return CResult::Succeed;
}

U64Result Sloong::SSLHelper::Read(char *data, int len, bool block, bool bagain)
{
    if (!CheckStatus(true))
    {
        return U64Result::Retry();
    }

    // SSL发送数据
    // 这里可能会有以下几种情况。
    // 1.正常全部读取完成。
    // 2.读取后发生错误，错误信息为SSL_ERROR_WANT_READ，需等待下次可读事件，并根据已读的部分进行组合。
    // 3.读取后发生错误，错误信息为其他，认为连接发生问题需要重连。
    int ret = SSL_Read_Ex(data, len, 0, true);
    if (ret == len)
    {
        return U64Result::Make_OKResult(ret);
    }
    else
    {
        return ResultCheck(ret);
    }
}

U64Result Sloong::SSLHelper::Write(const char *data, int len, int index)
{
    if (!CheckStatus(false))
    {
        return U64Result::Retry();
    }

    // SSL发送数据
    int ret = SSL_Write_Ex(data + index, len);
    if (ret == len - index)
    {
        return U64Result::Make_OKResult(ret);
    }
    else
    {
        return ResultCheck(ret);
    }
}

U64Result Sloong::SSLHelper::ResultCheck(int ret)
{
    if (ret > 0)
    {
        m_stStatus = ConnectStatus::WaitWrite;
        return U64Result::Retry();
    }
    else if (ret < 0)
    {
        auto pSSL = STATIC_TRANS<SSL *>(m_pSSL);
        int err = SSL_get_error(pSSL, ret);

        switch (err)
        {
        case SSL_ERROR_WANT_WRITE:
            m_stStatus = ConnectStatus::WaitWrite;
            return U64Result::Retry();
            break;
        case SSL_ERROR_WANT_READ:
            if (support_ssl_reconnect)
            {
                m_stStatus = ConnectStatus::WaitRead;
                return U64Result::Retry();
            }
            else
            {
                m_stStatus = ConnectStatus::ConnectError;
                return U64Result::Make_Error("SSL_ERROR_WANT_READ error, and support_ssl_reconnect is false.");
            }
            break;
        default:
            m_stStatus = ConnectStatus::ConnectError;
            return U64Result::Make_Error("SSL_ERROR_WANT_READ error");
            break;
        }
    }
    else
    {
        m_stStatus = ConnectStatus::ConnectError;
        return U64Result::Make_Error("SSL_ERROR_WANT_READ error");
    }
}

bool Sloong::SSLHelper::CheckStatus(bool bRead)
{
    switch (m_stStatus)
    {
    case ConnectStatus::Handshake:
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

int Sloong::SSLHelper::SSL_Read_Ex(char *buf, int nSize, int nTimeout, bool bAgagin)
{
    if (nSize <= 0)
        return 0;

    auto pSSL = STATIC_TRANS<SSL *>(m_pSSL);
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

int Sloong::SSLHelper::SSL_Write_Ex(const char *buf, int len)
{
    auto pSSL = STATIC_TRANS<SSL *>(m_pSSL);
    return SSL_write(pSSL, buf, len);
}

unsigned long Sloong::SSLHelper::G_InitializeSSL(LPVOID *out_ctx, const string &certFile, const string &keyFile, const string &passwd)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();                   /* load & register all cryptos, etc. */
    SSL_load_error_strings();                       /* load all error messages */
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

void Sloong::SSLHelper::G_FreeSSL(LPVOID ctx)
{
    if (ctx)
    {
        SSL_CTX_free(STATIC_TRANS<SSL_CTX *>(ctx));
    }
}

string Sloong::SSLHelper::G_FormatSSLErrorMsg(int code)
{
    return string(ERR_error_string(code, NULL));
}

bool Sloong::SSLHelper::do_handshake()
{
    auto pSSL = STATIC_TRANS<SSL *>(m_pSSL);
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
