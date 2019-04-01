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
        // if set the long long size, the sendpackage and recvPackage will use the length package with 8 bit.
        // else the size is 4 bit for long.
        CSocketEx(string addr, bool useLongLongSize);
        ~CSocketEx();

        bool Connect();
        bool Send(string sendData);
        string Recv(int length);

        bool SendPackage(string sendData);
        string RecvPackage();

        int GetSocket(){
            return m_nSocket;
        }
    protected:
        string m_strAddress;
        int m_nPort;
        int m_nSocket;
        bool m_bLongLongSize;
        int m_nTimeout = 5;
    };

}

#endif