/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2018-01-12 15:25:16
 * @LastEditTime: 2020-08-05 17:46:32
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/EasyConnect.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 封装Socket连接的细节信息。并将Socket进行隔离，上层使用特征码而不需要关心实际的连接Socket句柄。
 * 支持TCP 和 TCP over SSL。提供主动连接模式下的自动重连。
 * 将数据流的收发过程进行数据包的封装，上层只需要关心数据包的收发结果而不需要处理数据流可能导致的各种沾包拆包等复杂问题。
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#ifndef SLOONGNET_EASY_CONNECT_H
#define SLOONGNET_EASY_CONNECT_H

#include "core.h"

#include "SSLHelper.h"

namespace Sloong
{
	// 有两种使用方式：
	//  1：作为接受方，此时需要提供一个已经建立连接的SOCKET。
	//  2：作为发起方，此时需要提供一个目标地址，之后会自动处理连接过程
	class EasyConnect
	{
	public:
		~EasyConnect(){	m_pSSL = nullptr; }
		// 以接受方/服务端的方式初始化。链接断开后不进行任何操作
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		CResult InitializeAsServer(SOCKET, LPVOID p = nullptr);

		// 以发起方/客户端的方式初始化。链接断开后会根据信息尝试重连
		// 如果需要启用SSL支持，那么需要送入指定的ctx变量。否则保持送空即可。
		CResult InitializeAsClient(const string &, int, LPVOID p = nullptr);


		/**
         * @Description: Receive and create data package. 
         * @Params: 
		 * 		1 > Block receive. until succeed or error.
         * @Return: if package receive succeed, return Succed.
         *          if other error happened else return Error 
         *          if md5 check failed, return Invalied.
         */
		PackageResult RecvPackage(bool = false);

		/**
         * @Description: send this package
         * @Params: 
		 * 		1 > The DataPackage object.
         * @Return: if send fialed, return Error.
         */
		CResult SendPackage(UniquePackage);

		void Close();

		inline SOCKET GetSocketID() { return m_nSocket; }

		inline int GetErrno() { return m_nErrno; }

		inline int64_t GetHashCode() { return m_nHashCode; }

		inline bool IsReceiving() { return !m_strReceiving.empty(); }

		inline bool IsSending() { return !m_strSending.empty(); }

		inline void SetOnReconnectCallback(std::function<void(int64_t, int, int)> func)
		{
			m_pOnReconnect = func;
		}

	protected:	
		CResult Connect();
		
		int Read(char *, int, bool, bool);

		decltype(auto) RecvLengthData(bool);

		int Write(const char *, int, int);

		inline int Write(const string &sendData, int index) { return Write(sendData.c_str(), (int)sendData.length(), index); }

		string GetLengthData(int64_t);

		

	public:
		string m_strAddress;
		int m_nPort;
		int64_t m_nHashCode;

		string m_strReceiving;
		int m_RecvPackageSize;
		int m_ReceivedSize;

		string m_strSending;
		int m_SendPackageSize;
		int m_SentSize;

		SOCKET m_nInvalidSocket = INVALID_SOCKET;

	private:
		int m_nErrno;
		unique_ptr<SSLHelper> m_pSSL = nullptr;
		SOCKET m_nSocket = INVALID_SOCKET;
		
		bool m_bSupportReconnect = false;
		std::function<void(int64_t, int, int)> m_pOnReconnect = nullptr;
	};

	typedef unique_ptr<EasyConnect> UniqueConnection;

} // namespace Sloong

#endif // !SLOONGNET_EASY_CONNECT_H
