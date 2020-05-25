/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 12:30:12
 * @Description: file content
 */
#ifndef SOCKINFO_H
#define SOCKINFO_H


#include "EasyConnect.h"

#include "DataTransPackage.h"

#include "IObject.h"
#include "IControl.h"
namespace Sloong
{
	class CSockInfo : IObject
	{
	public:
		CSockInfo();
		~CSockInfo();

		void Initialize(IControl* iMsg, int sock, SSL_CTX* ctx);

		/**
		 * @Remarks: When data can receive, should call this function to receive the package.
		 * @Params: 
		 * @Return: if receive done, return Succeed.
		 * 		if happened errors, return Error.
		 * @Note: It always read all data in one time, so no return Retry.
		 */
		ResultType OnDataCanReceive( queue<UniqueTransPackage>& readList );

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
		ResultType SendDataPackage(UniqueTransPackage pack);

		inline bool TrySendLock(){
			return m_oSockSendMutex.try_lock();
		}
		inline bool TryReceiveLock(){
			return m_oSockReadMutex.try_lock();
		}

	protected:
		void ProcessPrepareSendList();
		ResultType ProcessSendList();
		queue_ex<UniqueTransPackage>* GetSendPackage();
		void AddToSendList(UniqueTransPackage pack);

	public:
        queue_ex<UniqueTransPackage>* m_pSendList; // the send list of the bytes.
        queue_ex<UniqueTransPackage>  m_oPrepareSendList;

		time_t m_ActiveTime;
		unique_ptr<EasyConnect> m_pCon;
		
        mutex m_oSockReadMutex;
        mutex m_oSockSendMutex; 
		UniqueTransPackage m_pSendingPackage = nullptr;
		UniqueTransPackage m_pReceiving = nullptr;
        bool m_bIsSendListEmpty = true;
	};

}

#endif // SOCKINFO_H
