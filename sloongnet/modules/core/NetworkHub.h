/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 16:58:54
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
        void EnableSSL(const string& certFile, const string& keyFile, const string& passwd);

        // event handler
        void Run(IEvent* event);
		void Exit(IEvent* event);
        void SendPackageEventHandler(IEvent* event);
        void CloseConnectEventHandler(IEvent* event);
		void MonitorSendStatusEventHandler(IEvent* evt);
        void RegisteConnectionEventHandler(IEvent* evt);


        inline void RegisterProcesser(RequestPackageProcessFunction req,ResponsePackageProcessFunction res,EventPackageProcessFunction event){
            m_pRequestFunc = req;
            m_pResponseFunc = res;
            m_pEventFunc = event;
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
        void RegisterAccpetConnectProcesser(NewConnectAcceptProcessFunction value){
            m_pAcceptFunc = value;
        }

        // Work thread.
		void CheckTimeoutWorkLoop();
        void MessageProcessWorkLoop();

        // Callback function
        ResultType OnNewAccept( int nSocket );
		ResultType OnDataCanReceive( int nSocket );
		ResultType OnCanWriteData( int nSocket );
        ResultType OnOtherEventHappened( int nSocket );

    protected:
        void SendCloseConnectEvent(int socket);
        void SendMessage(int sock, int nPriority, int64_t nSwift, string msg, const char* pExData = NULL, int nExSize = 0 );
        void AddMessageToSendList(UniqueTransPackage& pack);
        /// 将响应消息加入到epoll发送列表中
		void AddToSendList(int socket, int nPriority, const char* pBuf, int nSize, int nStart, const char* pExBuf, int nExSize);
        
    protected:
        map_ex<int, unique_ptr<CSockInfo>> m_SockList;
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
        RequestPackageProcessFunction          m_pRequestFunc = nullptr;
        ResponsePackageProcessFunction          m_pResponseFunc = nullptr;
        EventPackageProcessFunction            m_pEventFunc = nullptr;
        NewConnectAcceptProcessFunction        m_pAcceptFunc = nullptr;
        queue_ex<UniqueTransPackage>*    m_pWaitProcessList;
    };
}

