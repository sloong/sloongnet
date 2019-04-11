#ifndef SOCKINFO_H
#define SOCKINFO_H


#include "lconnect.h"

#include "DataTransPackage.h"

#include "IObject.h"

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
		NetworkResult OnDataCanReceive();

		/**
		 * @Remarks: When data can send, should call this function to send the package.
		 * @Params: 
		 * @Return: if send done, return Succeed.
		 * 			if happened erros, return Error.
		 * 			if happened EAGAIN signal, return Retry. 
		 */
		NetworkResult OnDataCanSend();

		/**
		 * @Remarks: When need response data package,call this function. 
		 * @Params: 
		 * @Return: if send data succeed, return Succeed.
		 * 			if happened erros, return Error.
		 * 			if have extend data or all data is no send and have EAGAIN sinal , return Retry.
		 */
		NetworkResult ResponseDataPackage(SmartPackage pack);

		void SetProperty( DataTransPackageProperty value ){
            m_emPackageProperty = value;
        }

	protected:
		void ProcessPrepareSendList();
		NetworkResult ProcessSendList();
		int GetSendInfoList(queue<shared_ptr<CDataTransPackage>>*& list );
		shared_ptr<CDataTransPackage> GetSendInfo(queue<shared_ptr<CDataTransPackage>>* list);
		void AddToSendList(SmartPackage pack);

	public:
        queue<shared_ptr<CDataTransPackage>>* m_pSendList; // the send list of the bytes.
        queue<shared_ptr<CDataTransPackage>> m_oPrepareSendList;

		time_t m_ActiveTime;
		shared_ptr<lConnect> m_pCon;

		unique_ptr<CLuaPacket> m_pUserInfo;
        mutex m_oSockReadMutex;
        mutex m_oSockSendMutex; 
		mutex m_oSendListMutex;
        mutex m_oPreSendMutex;
		int m_nLastSentTags = -1;
        bool m_bIsSendListEmpty = true;
		int m_ReceiveTimeout = 0;
		// 指示这个链接在接收数据包的时候，数据包的接收类型
		DataTransPackageProperty m_emPackageProperty = DataTransPackageProperty::EnableAll;
	};

}

#endif // SOCKINFO_H
