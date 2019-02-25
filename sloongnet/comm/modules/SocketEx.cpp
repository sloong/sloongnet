#include "SocketEx.h"

Sloong::CSocketEx::CSocketEx(string addr)
{
    auto params = CUniversal::split(m_strAddress, ":");
    m_strAddress = params[0];
    m_nPort = atoi(params[1].c_str());
}

Sloong::CSocketEx::~CSocketEx()
{
}

bool Sloong::CSocketEx::Connect()
{
    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(m_strAddress.c_str());
    remote_addr.sin_port = htons(m_nPort);
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
    return true;
}

bool Sloong::CSocketEx::Send(string sendData, bool appednLength)
{
    
    if( appednLength )
    {
        long long nMsgLen = sendData.size();
        
        char m_pMsgBuffer[s_llLen] = {0};
        char *pCpyPoint = m_pMsgBuffer;

        CUniversal::LongToBytes(nMsgLen, pCpyPoint);
        send( m_nSocket, pCpyPoint, s_llLen, 0);
    }
    send(m_nSocket, sendData.c_str(), sendData.length(), 0);
    return true;
}

string Sloong::CSocketEx::Recv(int length)
{
    string buf;
    buf.resize(length);
    CUniversal::RecvEx(m_nSocket,buf.data(),length, 5);
    return buf;
}

string Sloong::CSocketEx::RecvPackage()
{
    char nLen[4] = {0};
    CUniversal::RecvEx(m_nSocket, nLen, 4, 5);
    auto len = CUniversal::BytesToLong(nLen);

    string buf;
    buf.resize(len);
    CUniversal::RecvEx(m_nSocket, buf.data(), len, 5);

}

int Sloong::CSocketEx::GetSocket()
{
    return m_nSocket;
}