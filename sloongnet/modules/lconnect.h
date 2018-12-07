#ifndef LCONNECT_H_
#define LCONNECT_H_

#include <string>

// include the openssl file before inclue this file
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;
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
		Error,
	};

	class lConnect
	{
	public:
		lConnect();
		~lConnect();

		// 初始化链接对象
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		void Initialize( int sock, SSL_CTX* ctx);

		// 读取对端发送的数据，返回实际读取到的长度
		// 函数并不保证数据会全部读取完成才返回。只会尝试尽可能多的读取，直到发生错误或者都去完成。所以需要检查期望读取的长度和实际读取的长度
		// Return：
		//	  -1 - 读取发生错误。且无法恢复。需要关闭连接。
		//    >= 0 - 读取到的数据长度
		int Read(char* data, int len, int timeout, bool bagain = false);

		// 向对端发送数据，返回实际发送的长度
		// 函数并不保证数据会全部发送完成才返回。只能尝试尽可能多的发送，直到发生错误或者全部发送完成。需要检查期望发送的长度和实际发送的长度
		// Return：
		//    -1 - 读取发生错误。且无法恢复。需要关闭连接。
		//    >=0 - 成功发送的数据长度
		int Write(const char* data, int len, int index);

		void Close();

		int GetSocket();

	public:
		static int G_InitializeSSL(SSL_CTX*& ctx, string certFile, string keyFile, string passwd);
		static string G_FormatSSLErrorMsg(int code);

	private:
		bool CheckSSLStatus(bool bRead);
		bool do_handshake();

		int SSL_Read_Ex(SSL* ssl, char* buf, int nSize, int nTimeout, bool bAgagin);
		int SSL_Write_Ex(SSL* ssl, char* buf, int len);

	private:
		string m_strAddress;
		int m_nPort;
		int m_nSocket;
		SSL* m_pSSL = nullptr;

		string m_strErrorMsg;
		int m_nErrorCode;

		ConnectStatus m_stStatus;
	};

}

#endif // !LCONNECT_H_


