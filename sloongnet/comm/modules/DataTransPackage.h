#ifndef DATA_TRANS_PACKAGE_H
#define DATA_TRANS_PACKAGE_H

#include "main.h"

#include "lconnect.h"
namespace Sloong
{
    class CDataTransPackage
    {
	public:
		~CDataTransPackage(){
			SAFE_DELETE_ARR(m_pExBuffer);
		}

        void Initialize(SmartConnect conn,CLog* log= nullptr);

        /**
         * @Remarks: When process done, should call this function to response this package.
         * @Params: 
         * @Return: 
         */
        void ResponsePackage(const string& msg, const char* exData = nullptr, int exSize = 0);

        void RequestPackage( int SerialNumber, int priorityLevel,const string& msg);

        void PrepareSendPackageData( const string& msg, int priorityLevel, const char* exData = nullptr, int exSize = 0);

    public:
        /**
         * @Remarks: Receive and create data package. 
         * @Params: 
         * @Return: if package receive succeed, return Succed.
         *          if other error happened else return Error 
         *          if md5 check failed, return Invalied.
         */
        NetworkResult RecvPackage();

        /**
         * @Remarks: send this package
         * @Params: 
         * @Return: if send fialed, return Error.
         */
        NetworkResult SendPackage();

        inline string GetRecvMessage(){ return m_strMessage; }

        inline int GetSocketID(){return m_pCon->GetSocketID(); }

        /**
         * @Remarks: If have ex data send, this package is big package. need add to send list.
         * @Params: 
         * @Return: 
         */
        inline bool IsBigPackage(){ return m_nExSize > 0 ? true : false; }

    public:
        // priority of this package
        int nPriority = 0;

    protected:
        // Send data info
        string m_szMsgBuffer;
        int m_nMsgSize=0;
		const char* m_pExBuffer=nullptr;
		int m_nExSize=0;
        int m_nSent=0;
		int m_nPackSize=0;

        // serial number of this package
        long long m_nSerialNumber = -1;
        
        // received MD5, used to check the validity of message 
        string m_strMD5 = "";
        
        // request message of this package.
        string m_strMessage = "";

    protected:
        SmartConnect    m_pCon;
        CLog*           m_pLog = nullptr;
    };

    typedef shared_ptr<CDataTransPackage> SmartPackage;
}


#endif