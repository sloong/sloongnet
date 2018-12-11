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

		// ��ʼ�����Ӷ���
		// �����Ҫ����SSL֧�֣���ô��Ҫ����ָ����ctx���������򱣳��Ϳռ��ɡ�
		void Initialize( int sock, SSL_CTX* ctx);

		// ��ȡ�Զ˷��͵����ݣ�����ʵ�ʶ�ȡ���ĳ���
		// ����������֤���ݻ�ȫ����ȡ��ɲŷ��ء�ֻ�᳢�Ծ����ܶ�Ķ�ȡ��ֱ������������߶�ȥ��ɡ�������Ҫ���������ȡ�ĳ��Ⱥ�ʵ�ʶ�ȡ�ĳ���
		// Return��
		//	  -1 - ��ȡ�����������޷��ָ�����Ҫ�ر����ӡ�
		//    >= 0 - ��ȡ�������ݳ���
		int Read(char* data, int len, int timeout, bool bagain = false);

		// ��Զ˷������ݣ�����ʵ�ʷ��͵ĳ���
		// ����������֤���ݻ�ȫ��������ɲŷ��ء�ֻ�ܳ��Ծ����ܶ�ķ��ͣ�ֱ�������������ȫ��������ɡ���Ҫ����������͵ĳ��Ⱥ�ʵ�ʷ��͵ĳ���
		// Return��
		//    -1 - ��ȡ�����������޷��ָ�����Ҫ�ر����ӡ�
		//    >=0 - �ɹ����͵����ݳ���
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


