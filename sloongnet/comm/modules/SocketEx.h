#ifndef SLOONGNET_SOCKETEX_H
#define SLOONGNET_SOCKETEX_H

#include "main.h"
namespace Sloong
{
    class CSocketEx
    {
    public:
        // format addr:port
        CSocketEx(string addr);
        ~CSocketEx();

        bool Connect();
        bool Send(string sendData, bool appendLength = true);
        string Recv(int length);
        string RecvPackage();

        int GetSocket();
    protected:
        string m_strAddress;
        int m_nPort;
        int m_nSocket;
    };

}

#endif