#ifndef CEPOLLEX_H
#define CEPOLLEX_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <openssl/ssl.h>
#include "structs.h"

#include "IData.h"
#include "IMessage.h"
using namespace std; //std 命名空间
typedef unsigned char byte;

namespace Sloong
{
	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;
	using namespace Interface;
	
	class CSockInfo;
	class CServerConfig;
	class CEpollEx
	{
	public:
        CEpollEx();
		virtual ~CEpollEx();
        int Initialize(IMessage* iM,IData* iData);
		void Run();
		void EnableSSL(string certFile, string keyFile, string passwd);
		void SetLogConfiguration(bool bShowSendMessage, bool bShowReceiveMessage);
		void Exit();
        void SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData = NULL, int nSize = 0 );
		void ProcessPrepareSendList( CSockInfo* info );
		/************************************************************************/
		/* If need listen write event, return false, else return true
		*/
		/************************************************************************/
		bool ProcessSendList(CSockInfo* pInfo);
		//////////////////////////////////////////////////////////////////////////
		// Add for #20 [https://git.sloong.com/public/sloongnet/issues/20] 
		// 在下面的CloseConnect函数中，将只关闭socket连接和发送event，并不删除相对应的信息
		// 直到外层监听该事件的处理者处理之后调用该函数才移除对应的信息
		//////////////////////////////////////////////////////////////////////////
		void CloseSocket(int socket);
	protected:
		/// 设置socket到非阻塞模式
		int SetSocketNonblocking(int socket);
		/// 修改socket的epoll监听事件
		void CtlEpollEvent(int opt, int sock, int events);
		/// close the connected socket and remove the resources.
		void CloseConnect(int socket);
		/// 将响应消息加入到epoll发送列表中
		void AddToSendList(int socket, int nPriority, const char* pBuf, int nSize, int nStart, const char* pExBuf, int nExSize);
		
		// event function
		void OnNewAccept();
		void OnDataCanReceive( int nSocket );
		void OnCanWriteData( int nSocket );

	public:
		static void* CALLBACK_SocketClose(void* params, void* object);
		static void* WorkLoop(void* params);
		static void* CheckTimeoutConnect(void* params);
		static void* EventHandler(void* params, void* object);
	protected:
		int     m_ListenSock;
		int 	m_EpollHandle;
		//struct epoll_event m_Event;
		epoll_event m_Events[1024];
		CLog*		m_pLog;
	public:
		map<int, CSockInfo*> m_SockList;
        mutex m_oSockListMutex;
		mutex m_oExitMutex;
		condition_variable m_oExitCV;
		SSL_CTX* m_pCTX;
		bool m_bIsRunning;
		bool m_bEnableClientCheck;
		int m_nClientCheckKeyLength;
		CServerConfig* m_pConfig;
		IData* m_iData;
		IMessage* m_iMsg;
	};
}


#endif // CEPOLLEX_H
