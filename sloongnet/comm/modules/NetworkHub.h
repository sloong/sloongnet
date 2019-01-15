#pragma once

#include "IObject.h"

namespace Sloong
{
    class CEasySync;
    class CSockInfo;
    class CEpollEx;
    class CServerConfig;
    class CNetworkCenter : IObject
    {
    public:
        CNetworkCenter();
        ~CNetworkCenter();

        void Initialize(IControl* iMsg);

        // event handler
        void Run(SmartEvent event);
		void Exit(SmartEvent event);
        void SendMessageEventHandler(SmartEvent event);
        void CloseConnectEventHandler(SmartEvent event);
		void MonitorSendStatusEventHandler(SmartEvent evt);

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
        CEasySync              m_oSync;
        SSL_CTX*                m_pCTX = nullptr;
		bool m_bEnableClientCheck;
		int m_nClientCheckKeyLength;
    };
}

