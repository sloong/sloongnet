#ifndef SLOONGNET_EASY_CONNECT_H
#define SLOONGNET_EASY_CONNECT_H

#include "core.h"
namespace Sloong
{
	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;

	enum ConnectStatus
	{
		Disconnect,
		Ready,
		WaitRead,
		WaitWrite,
		ConnectError,
	};

	// 有两种使用方式：
	//  1：作为接受方，此时需要提供一个已经建立连接的SOCKET。
	//  2：作为发起方，此时需要提供一个目标地址，之后会自动处理连接过程
	class EasyConnect
	{
	public:
		EasyConnect();
		~EasyConnect();

		// 以接受方的方式初始化
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		void Initialize( SOCKET sock, SSL_CTX* ctx= nullptr, bool useLongLongSize = false);

		// 以发起方的方式初始化
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		void Initialize( string address, int port, SSL_CTX* ctx= nullptr, bool useLongLongSize = false);

		bool Connect();

		// 读取对端发送的数据，返回实际读取到的长度
		// 函数并不保证数据会全部读取完成才返回。只会尝试尽可能多的读取，直到发生错误或者都去完成。所以需要检查期望读取的长度和实际读取的长度
		// Return：
		//	  -1 - 读取发生错误。且无法恢复。需要关闭连接。
		//    >= 0 - 读取到的数据长度
		int Read( char* data, int len, bool block, int timeout, bool bagain);
		
		// 接收一个数据包
		// 如果接收到了所有的数据，那么返回succeed.
		// 如果接收到了部分数据之后发生错误，直接返回Error
		// 如果接收到了部分数据，并且超时设置为0，那么将会重复尝试，直至全部接收完毕或者发生其他错误，
		// 如果接收到了部分数据，并且超时时间大于0，那么将会在发生超时之后，返回Timeout
		// 如果没有接收到数据，并且发生了错误，返回Error
		// 没有接收到数据，发生了超时，返回Timeout
		ResultType RecvPackage(string& res, int timeOut = 0);

		long long RecvLengthData(bool block);

		// 向对端发送数据，返回实际发送的长度
		// 函数并不保证数据会全部发送完成才返回。只能尝试尽可能多的发送，直到发生错误或者全部发送完成。需要检查期望发送的长度和实际发送的长度
		// Return：
		//    -1 - 读取发生错误。且无法恢复。需要关闭连接。
		//    >=0 - 成功发送的数据长度
		int Write(const char* data, int len, int index);

		int Write(string sendData, int index);

		// 首先根据在Initialize函数中的参数发送Package长度（4Bit/8Bit）。然后在发送数据包本身
		int SendPackage(string sendData, int index);

		bool SendLengthData(long long lengthData);

		void Close();

		SOCKET GetSocketID(){
			return m_nSocket;
		}

		int GetErrno(){
			return m_nErrno;
		}
	public:
		static unsigned long G_InitializeSSL(SSL_CTX*& ctx, string certFile, string keyFile, string passwd);
		static string G_FormatSSLErrorMsg(int code);

	private:
		bool CheckSSLStatus(bool bRead);
		bool do_handshake();

		int SSL_Read_Ex(SSL* ssl, char* buf, int nSize, int nTimeout, bool bAgagin);
		int SSL_Write_Ex(SSL* ssl, char* buf, int len);
	public:
		string m_strAddress;
		int m_nPort;
	private:
		int m_nErrno;
		SOCKET m_nSocket=INVALID_SOCKET;
		SSL* m_pSSL = nullptr;
		bool m_bSupportReconnect = false;
		string m_strErrorMsg;
		int m_nErrorCode;
		bool m_bUseLongLongSize = false;
		ConnectStatus m_stStatus;
	};

	typedef shared_ptr<EasyConnect> SmartConnect;

}

#endif // !SLOONGNET_EASY_CONNECT_H


