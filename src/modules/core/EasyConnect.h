#ifndef SLOONGNET_EASY_CONNECT_H
#define SLOONGNET_EASY_CONNECT_H

#include "core.h"

namespace Sloong
{
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
		// 以接受方/服务端的方式初始化。链接断开后不进行任何操作
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		void InitializeAsServer(SOCKET, LPVOID p = nullptr);

		// 以发起方/客户端的方式初始化。链接断开后会根据信息尝试重连
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		void InitializeAsClient(const string &, int, LPVOID p = nullptr);

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
		ResultType RecvPackage(string &, int &, int &, bool = false);

		/**
         * @Description: Receive and create data package. 
         * @Params: 
         * @Return: if package receive succeed, return Succed.
         *          if other error happened else return Error 
         *          if md5 check failed, return Invalied.
         */
		PackageResult RecvPackage(bool = false);

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
		ResultType SendPackage(const string &, int &);

		/**
         * @Description: send this package
         * @Params: 
         * @Return: if send fialed, return Error.
         */
		CResult SendPackage(UniquePackage);

		void Close();

		inline SOCKET GetSocketID() { return m_nSocket; }

		inline int GetErrno() { return m_nErrno; }

		inline int64_t GetHashCode() { return m_nHashCode; }

		inline bool IsReceiving() { return !m_strReceiving.empty(); }

		inline bool IsSending() { return !m_strSending.empty(); }

		inline void SetOnReconnectCallback(std::function<void(int)> func)
		{
			m_pOnReconnect = func;
		}

	protected:
		int Read(char *, int, bool, bool);

		decltype(auto) RecvLengthData(bool);

		int Write(const char *, int, int);

		inline int Write(const string &sendData, int index) { return Write(sendData.c_str(), (int)sendData.length(), index); }

		string GetLengthData(int64_t);

	public:
		static unsigned long G_InitializeSSL(LPVOID *, const string &, const string &, const string &);
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
		bool m_bReconnect = false;
		int64_t m_nHashCode;

		string m_strReceiving;
		int m_RecvPackageSize;
		int m_ReceivedSize;

		string m_strSending;
		int m_SendPackageSize;
		int m_SentSize;

	private:
		int m_nErrno;
		SOCKET m_nSocket = INVALID_SOCKET;
		LPVOID m_pSSL = nullptr;
		bool m_bSupportReconnect = false;
		string m_strErrorMsg;
		int m_nErrorCode;
		std::function<void(int)> m_pOnReconnect = nullptr;
		ConnectStatus m_stStatus = ConnectStatus::Disconnect;
	};

	typedef unique_ptr<EasyConnect> UniqueConnection;

} // namespace Sloong

#endif // !SLOONGNET_EASY_CONNECT_H
