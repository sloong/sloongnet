/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-11 18:39:54
 * @Description: file content
 */
#pragma once

#include "IObject.h"
#include "export.h"
namespace Sloong
{ 
    class CSockInfo;
    class CEpollEx;
    class CNetworkHub : IObject
    {
    public:
        CNetworkHub();
        ~CNetworkHub();

        CResult Initialize(IControl* iMsg);

        void EnableClientCheck(const string& clientCheckKey, int clientCheckTime);
        void EnableTimeoutCheck(int timeoutTime, int checkInterval);
        void EnableSSL(string certFile, string keyFile, string passwd);

        // event handler
        void Run(SmartEvent event);
		void Exit(SmartEvent event);
        void SendMessageEventHandler(SmartEvent event);
        void CloseConnectEventHandler(SmartEvent event);
		void MonitorSendStatusEventHandler(SmartEvent evt);


        inline void RegisterMessageProcesser(MessagePackageProcesserFunction value){
            m_pProcessFunc = value;
        }
        inline void RegisterEventProcesser(EventPackageProcesserFunction value){
            m_pEventFunc = value;
        }
        inline void RegisterEnvCreateProcesser(CreateProcessEnvironmentFunction value){
            m_pCreateEnvFunc = value;
        }


        /**
         * @Remarks: In default case, the connect just accept and add to epoll watch list. 
         *       if want do other operation, call this function and set the process, when accpet ent, will call this function .
         * @Params: the function bind for processer
         * @Return: NO
         */
        void RegisterAccpetConnectProcesser(NewConnectAcceptProcesserFunction value){
            m_pAcceptFunc = value;
        }

        // Work thread.
		void CheckTimeoutWorkLoop(SMARTER params=nullptr);
        void MessageProcessWorkLoop(SMARTER params=nullptr);

        void RegisteConnection(SOCKET);

        // Callback function
        ResultType OnNewAccept( int nSocket );
		ResultType OnDataCanReceive( int nSocket );
		ResultType OnCanWriteData( int nSocket );
        ResultType OnOtherEventHappened( int nSocket );

    protected:
        void SendCloseConnectEvent(int socket);
        void SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData = NULL, int nExSize = 0 );
        void AddMessageToSendList(SmartPackage pack);
        /// 将响应消息加入到epoll发送列表中
		void AddToSendList(int socket, int nPriority, const char* pBuf, int nSize, int nStart, const char* pExBuf, int nExSize);
        
    protected:
        map<int, shared_ptr<CSockInfo>> m_SockList;
        mutex                   m_oSockListMutex;
        bool                    m_bIsRunning;
        unique_ptr<CEpollEx>    m_pEpoll;
        CEasySync               m_oCheckTimeoutThreadSync;
        
        SSL_CTX*                m_pCTX = nullptr;
        GLOBAL_CONFIG*          m_pConfig = nullptr;

        // Timeout check
		int m_nConnectTimeoutTime=0;
        int m_nCheckTimeoutInterval=0;
        // Client check 
        string  m_strClientCheckKey="";
		int m_nClientCheckKeyLength=0;
        int m_nClientCheckTime=0;
        // For message process 
        CEasySync               m_oProcessThreadSync;
        CreateProcessEnvironmentFunction         m_pCreateEnvFunc = nullptr;
        MessagePackageProcesserFunction          m_pProcessFunc = nullptr;
        EventPackageProcesserFunction            m_pEventFunc = nullptr;
        NewConnectAcceptProcesserFunction        m_pAcceptFunc = nullptr;
        queue_ex<SmartPackage>*    m_pWaitProcessList;
    };
}

