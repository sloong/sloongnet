/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-08-03 14:36:10
 * @LastEditTime: 2021-09-30 14:04:06
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/SSLHelper.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */

#pragma once

#ifndef SOCKET
#define SOCKET int
#endif

#include "result.h"

namespace Sloong
{
enum ConnectStatus
{
    // 刚创建连接，尚未进行连接时的默认状态。在此情况下，连接成功不会触发回调函数
    Created,
    // 链接正常，可以发送/接收数据。
    Ready,
    // 适用于启用SSL的情况，表示Socket连接正常，但是SSL需要进行Handshake。
    Handshake,
    // 适用于启用SSL的情况，
    WaitRead,
    // 适用于启用SSL的情况，
    WaitWrite,
    // 适用于启用SSL的情况，SSL始终无法成功建立正常通讯的情况下会设置此标记
    ConnectError,
};

class SSLHelper
{
  public:
    SSLHelper(void *ctx)
    {
        m_pSSL_CTX = ctx;
    }

    ~SSLHelper();

    CResult Initialize(SOCKET sock);

    U64Result Read(char *, int, bool, bool);
    U64Result Write(const char *, int, int);

    int SSL_Read_Ex(char *, int, int, bool);
    int SSL_Write_Ex(const char *, int);
    bool do_handshake();

    bool CheckStatus(bool);
    U64Result ResultCheck(int);

  public:
    static unsigned long G_InitializeSSL(void **, const string &, const string &, const string &);
    static void G_FreeSSL(void *);
    static string G_FormatSSLErrorMsg(int);

  protected:
    SOCKET m_nSocket = -1;
    void *m_pSSL_CTX = nullptr;
    void *m_pSSL = nullptr;

    ConnectStatus m_stStatus = ConnectStatus::Created;
};
} // namespace Sloong