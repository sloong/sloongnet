#include "SocketEx.h"

Sloong::CSocketEx::CSocketEx(string addr)
{
    CSocketEx(addr, false);
}

Sloong::CSocketEx::CSocketEx( string addr, bool useLongLongSize )
{
    auto params = CUniversal::split(m_strAddress, ":");
    m_strAddress = params[0];
    m_nPort = atoi(params[1].c_str());
    m_bLongLongSize = useLongLongSize;
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

bool Sloong::CSocketEx::Send(string sendData)
{
    CUniversal::SendEx(m_nSocket, sendData.c_str(), sendData.length());
    return true;
}


string Sloong::CSocketEx::Recv(int length)
{
    string buf;
    buf.resize(length);
    CUniversal::RecvEx(m_nSocket,buf.data(),length, 5);
    return buf;
}



long long Sloong::CSocketEx::RecvLengthData()
{
    if( m_bLongLongSize ) {
        char nLen[s_llLen] = {0};
        CUniversal::RecvEx(m_nSocket, nLen, s_llLen, m_nTimeout);
        auto len = CUniversal::BytesToInt64(nLen);
        return len;
    }else{
        char nLen[s_lLen] = {0};
        CUniversal::RecvEx(m_nSocket, nLen, s_lLen, m_nTimeout);
        auto len = CUniversal::BytesToInt32(nLen);
        return len;
    }
}

string Sloong::CSocketEx::GetSendLengthData(long long lengthData)
{
    if( m_bLongLongSize ) {
        char m_pMsgBuffer[s_llLen] = {0};
        char *pCpyPoint = m_pMsgBuffer;
        CUniversal::Int64ToBytes(lengthData, pCpyPoint);
        return string(m_pMsgBuffer,s_llLen);
    }else{
        char m_pMsgBuffer[s_lLen] = {0};
        char *pCpyPoint = m_pMsgBuffer;
        CUniversal::Int32ToBytes(lengthData, pCpyPoint);
        return string(m_pMsgBuffer,s_lLen);
    }
}



bool Sloong::CSocketEx::SendPackage(string sendData)
{
    auto length = GetSendLengthData(sendData.size());
    
    Send(length);
    return Send(sendData);
}




string Sloong::CSocketEx::RecvPackage()
{
    auto len = RecvLengthData();

    string buf;
    buf.resize(len);
    CUniversal::RecvEx(m_nSocket, buf.data(), len, m_nTimeout);

}
