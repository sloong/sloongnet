#pragma once

#include <memory>
using namespace std;

#include "IObject.h"
using namespace Sloong::Interface;

namespace Sloong
{
    class CSmartSync;
    class CSockInfo;
    class CEpollEx;
    class CServerConfig;
    class CNetworkCenter : IObject
    {
    public:
        CNetworkCenter();
        ~CNetworkCenter();

        void Initialize(IMessage* iMsg, IData* iData );

        // event handler
        void Run(SmartEvent event);
		void Exit(SmartEvent event);
        void SendMessageEventHandler(SmartEvent event);
        void CloseConnectEventHandler(SmartEvent event);

        // Work thread.
		void CheckTimeoutWorkLoop(SMARTER params);

        // Callback function
        NetworkResult OnNewAccept( int nSocket );
		NetworkResult OnDataCanReceive( int nSocket );
		NetworkResult OnCanWriteData( int nSocket );
        NetworkResult OnOtherEventHappened( int nSocket );

    protected:
        void SendCloseConnectEvent(int socket);
        void EnableSSL(string certFile, string keyFile, string passwd);

        void SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData = NULL, int nExSize = 0 );

        /// 将响应消息加入到epoll发送列表中
		void AddToSendList(int socket, int nPriority, const char* pBuf, int nSize, int nStart, const char* pExBuf, int nExSize);
        
    protected:
        map<int, shared_ptr<CSockInfo>> m_SockList;
        mutex                   m_oSockListMutex;
        bool                    m_bIsRunning;
        unique_ptr<CEpollEx>    m_pEpoll;
        CServerConfig*          m_pConfig;
        CSmartSync              m_oSync;
        SSL_CTX*                m_pCTX = nullptr;
		bool m_bEnableClientCheck;
		int m_nClientCheckKeyLength;
    };
}

