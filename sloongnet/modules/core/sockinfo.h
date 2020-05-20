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
		ResultType OnDataCanReceive( queue<SmartPackage>& readList );

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
		ResultType SendDataPackage(SmartPackage pack);

	protected:
		void ProcessPrepareSendList();
		ResultType ProcessSendList();
		SmartPackage GetSendPackage();
		void AddToSendList(SmartPackage pack);

	public:
        queue_ex<SmartPackage>* m_pSendList; // the send list of the bytes.
        queue_ex<SmartPackage> m_oPrepareSendList;

		time_t m_ActiveTime;
		shared_ptr<EasyConnect> m_pCon;
		
        mutex m_oSockReadMutex;
        mutex m_oSockSendMutex; 
		mutex m_oSendListMutex;
        mutex m_oPreSendMutex;
		SmartPackage m_pSendingPackage = nullptr;
        bool m_bIsSendListEmpty = true;
		int m_ReceiveTimeout = 0;
	};

}

#endif // SOCKINFO_H
