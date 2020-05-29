/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 12:30:12
 * @Description: file content
 */
#ifndef SOCKINFO_H
#define SOCKINFO_H

#include "IObject.h"
#include "DataTransPackage.h"

namespace Sloong
{
	class CSockInfo : IObject
	{
	public:
		CSockInfo();
		~CSockInfo();

		void Initialize(IControl *, int, LPVOID);

		/**
		 * @Remarks: When data can receive, should call this function to receive the package.
		 * @Params: 
		 * @Return: if receive done, return Succeed.
		 * 		if happened errors, return Error.
		 * @Note: It always read all data in one time, so no return Retry.
		 */
		ResultType OnDataCanReceive(queue<UniqueTransPackage> &);

		/**
		 * @Remarks: When data can send, should call this function to send the package.
		 * @Params: 
		 * @Return: if send done, return Succeed.
		 * 			if happened erros, return Error.
		 * 			if happened EAGAIN signal, return Retry. 
		 */
		ResultType OnDataCanSend();

		/**
		 * @Remarks: When need response data package,call this function. 
		 * @Params: 
		 * @Return: if send data succeed, return Succeed.
		 * 			if happened erros, return Error.
		 * 			if have extend data or all data is no send and have EAGAIN sinal , return Retry.
		 */
		ResultType SendDataPackage(UniqueTransPackage);

		inline bool TrySendLock()
		{
			return m_oSockSendMutex.try_lock();
		}
		inline bool TryReceiveLock()
		{
			return m_oSockReadMutex.try_lock();
		}

	protected:
		void ProcessPrepareSendList();
		ResultType ProcessSendList();
		queue_ex<UniqueTransPackage> *GetSendPackage();
		void AddToSendList(UniqueTransPackage);

	public:
		queue_ex<UniqueTransPackage> *m_pSendList; // the send list of the bytes.
		queue_ex<UniqueTransPackage> m_oPrepareSendList;

		time_t m_ActiveTime;
		unique_ptr<EasyConnect> m_pCon;

		mutex m_oSockReadMutex;
		mutex m_oSockSendMutex;
		UniqueTransPackage m_pSendingPackage = nullptr;
		UniqueTransPackage m_pReceiving = nullptr;
		bool m_bIsSendListEmpty = true;
	};

} // namespace Sloong

#endif // SOCKINFO_H
