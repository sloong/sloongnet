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
		// 以接受方的方式初始化
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		void Initialize(SOCKET, LPVOID p = nullptr);

		// 以发起方的方式初始化
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		void Initialize(const string &, int, LPVOID p = nullptr);

		bool Connect();

		/* Receive a data package.
		   Params:
				1 -> The buffer to save received data.
				2 -> Data package length.If it's first call, set to 0, When the function returns ti will be set to the length of this package. 
						And in next calls, reset to this length.
				3 -> Received length. If it's first call, set to 0. When the function returns it will be set to the actual received length. 
						And in next calls, reset to this actual received length.
				4 -> Block receive. until succeed or error.
		   Returns:
		    Succeed	-> All data received.
		    Error	-> Error happened. need close socket.
		    Retry	-> Part of the package was received, and it's happened EAGAIN error. need call this function again when socket is can received.
		*/
		ResultType RecvPackage(string&, int&, int&, bool=false);

		/* Send the data package.
		   Params:
		  		1 -> the send data.
		  		2 -> Sent length. If it's first call, set to 0. When the function returns it will be set to the actual sent length. 
		  				And in next calls, reset to this actual sent length.
		   Returns:
		  	Succeed	-> All data is sent.
		  	Error	-> Error happened. need close socket.
		  	Retry	-> Part of the package was sent. and it's happened EAGAIN error. need call this function again when socket is can sent.
		*/
		ResultType SendPackage(const string &, int&);

		void Close();

		inline SOCKET GetSocketID() { return m_nSocket; }

		inline int GetErrno() { return m_nErrno; }

	protected:
		int Read(char *, int, bool, bool);

		decltype(auto) RecvLengthData(bool);

		int Write(const char *, int, int);

		inline int Write(const string &sendData, int index) { return Write(sendData.c_str(), (int)sendData.length(), index); }

		string GetLengthData(int64_t);

	public:
		static unsigned long G_InitializeSSL(LPVOID*, const string &, const string &, const string &);
		static void G_FreeSSL(LPVOID);
		static string G_FormatSSLErrorMsg(int);

	private:
		bool CheckSSLStatus(bool);
		bool do_handshake();

		int SSL_Read_Ex(char *, int, int, bool);
		int SSL_Write_Ex(const char *, int);

	public:
		string m_strAddress;
		int m_nPort;

	private:
		int m_nErrno;
		SOCKET m_nSocket = INVALID_SOCKET;
		LPVOID m_pSSL = nullptr;
		bool m_bSupportReconnect = false;
		string m_strErrorMsg;
		int m_nErrorCode;
		ConnectStatus m_stStatus = ConnectStatus::Disconnect;
	};

	typedef unique_ptr<EasyConnect> SmartConnect;

} // namespace Sloong

#endif // !SLOONGNET_EASY_CONNECT_H
