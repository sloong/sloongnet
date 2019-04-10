#pragma once

#include "IObject.h"

namespace Sloong
{
    class CEasySync;
    class CSockInfo;
    class CEpollEx;
    class CConfiguation;
    class CNetworkHub : IObject
    {
    public:
        CNetworkHub();
        ~CNetworkHub();

        void Initialize(IControl* iMsg);

        void EnableClientCheck(const string& clientCheckKey, int clientCheckTime);
        void EnableTimeoutCheck(int timeoutTime, int checkInterval);
        void EnableSSL(string certFile, string keyFile, string passwd);
        void SetProperty(DataTransPackageProperty value){
            m_emPackageProperty = value;
        }

        void AddMonitorSocket(int socket,DataTransPackageProperty property );

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
        void SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData = NULL, int nExSize = 0 );

        /// 将响应消息加入到epoll发送列表中
		void AddToSendList(int socket, int nPriority, const char* pBuf, int nSize, int nStart, const char* pExBuf, int nExSize);
        
    protected:
        map<int, shared_ptr<CSockInfo>> m_SockList;
        mutex                   m_oSockListMutex;
        bool                    m_bIsRunning;
        unique_ptr<CEpollEx>    m_pEpoll;
        CEasySync              m_oSync;
        SSL_CTX*                m_pCTX = nullptr;
        ProtobufMessage::GLOBAL_CONFIG*          m_pConfig = nullptr;
        // Timeout check
		int m_nConnectTimeoutTime=0;
        int m_nCheckTimeoutInterval=0;
        // Client check 
        string  m_strClientCheckKey="";
		int m_nClientCheckKeyLength=0;
        int m_nClientCheckTime=0;
        DataTransPackageProperty m_emPackageProperty = EnableAll;
    };
}

