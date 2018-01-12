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
#include "sockinfo.h"
#include "structs.h"
using namespace std; //std 命名空间
typedef unsigned char byte;

namespace Sloong
{
	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;

	struct EpollExConfig
	{
		// 优先级分级数量，最高为255
		int m_nPriorityLevel;
		// 在日志记录中显示发送消息
		bool m_bShowSendMessage;
		// 在日志记录中显示接收消息
		bool m_bShowReceiveMessage;
		// 是否启用流水号支持
		bool m_bSwiftNumberSupport;
		// 是否启用MD5支持
		bool m_bMD5Support;
		// 是否启用客户端连接检查
		bool m_bEnableClientCheck;
		// 客户端检查密匙
		string m_strClientCheckKey;
		// 客户单密匙长度
		int m_nCheckKeyLength;
		// 客户端检查超时
		int m_nClientCheckTime;
		// 连接超时
		int m_nConnectTimeout;
		// 接收超时
		int m_nReceiveTimeout;
		// 主动进行超时检查的间隔。
		int m_nTimeoutInterval;
		// 是否启用SSL
		bool m_bEnableSSL;
		// 运行的线程数
		int m_nWorkThreadNum;
		EpollExConfig()
		{
			m_bEnableClientCheck = false;
		}
	};

	class CEpollEx
	{
	public:
        CEpollEx();
		virtual ~CEpollEx();
        int Initialize(CLog* pLog,int listenPort, int nThreadNum, int nPriorityLevel, bool bSwiftNumSupprot, bool bMD5Support, 
				int nTimeout, int nTimeoutInterval, int nRecvTimeout, int nCheckTimeout,string strCheckKey);
		void SetConfig(EpollExConfig& conf);
		void Run();
		void EnableSSL(string certFile, string keyFile, string passwd);
		void SetLogConfiguration(bool bShowSendMessage, bool bShowReceiveMessage);
		void Exit();
        void SendMessage(int sock, int nPriority, long long nSwift, string msg, const char* pExData = NULL, int nSize = 0 );
        void SetEvent( condition_variable* pCV );
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
		static void* WorkLoop(void* params);
		static void* CheckTimeoutConnect(void* params);
	protected:
		int     m_ListenSock;
		int 	m_EpollHandle;
		//struct epoll_event m_Event;
		epoll_event m_Events[1024];
		CLog*		m_pLog;
	public:
		map<int, CSockInfo*> m_SockList;
		queue<EventListItem> m_EventSockList;
        mutex m_oEventListMutex;
        mutex m_oSockListMutex;
		mutex m_oExitMutex;
		condition_variable m_oExitCV;
        condition_variable* m_pEventCV;
		SSL_CTX* m_pCTX;
		bool m_bIsRunning;
		EpollExConfig m_oConfig;
	};
}


#endif // CEPOLLEX_H
