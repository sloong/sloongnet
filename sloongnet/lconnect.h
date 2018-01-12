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
		void Initialize( int sock, SSL_CTX* ctx = nullptr);

		// 读取对端发送的数据
		// 在读取发生错误需要关闭连接时，将直接抛出异常而不是返回-1.
		// Return：
		//    0 - 读取发生问题，但是没有错误，需要等待下次可读写状态进行再次重试。
		//    >0 - 读取到的数据长度
		int Read(char* data, int len, int timeout, bool bagain = false);

		// 向对端发送数据
		// 在发送发生错误需要关闭连接时，将直接抛出异常而不是返回-1.
		// Return：
		//    0 - 发送数据时出现问题，但是没有错误，需要等待下次可读写状态进行再次重试。
		//    >0 - 成功发送的数据长度
		int Write(const char* data, int len, int index);

		void Close();

		int GetSocket();

	public:
		static int G_InitializeSSL(SSL_CTX** ctx, string certFile, string keyFile, string passwd);
		static string G_FormatSSLErrorMsg(int code);

	private:
		bool CheckSSLStatus(bool bRead);
		bool do_handshake();

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


