#ifndef DATA_TRANS_PACKAGE_H
#define DATA_TRANS_PACKAGE_H

#include <string> 
using std::string;

#include "defines.h"
#include "lconnect.h"
namespace Sloong
{
    class CDataTransPackage
    {
    private:
        CDataTransPackage(){}
	public:
		CDataTransPackage(SmartConnect);
		~CDataTransPackage(){
			SAFE_DELETE_ARR(pSendBuffer);
			SAFE_DELETE_ARR(pExBuffer);
		}

        /**
         * @Remarks: When process done, should call this function to response this package.
         * @Params: 
         * @Return: 
         */
        bool ResponsePackage(const string& msg, const char* exData, int exSize);

    public:
        /**
         * @Remarks: Receive and create data package. 
         * @Params: 
         * @Return: if package receive succeed, return true.
         *          else return false if other error happened.
         */
        bool RecvPackage(ULONG);

        bool SendPackage();

        string GetRecvMessage();
        void SetSendMessage(const string& msg, const char* exData, int exSize );

    public:
        // Send data info
        const char* pSendBuffer=nullptr;
        int nSize=0;
		const char* pExBuffer=nullptr;
		int nExSize=0;
        int nSent=0;  // is send
		int nPackSize=0;
        // Recv data info
        long long nSwiftNumber = -1;
        string strMD5 = "";
        string strMessage = "";

        int nPriority = 0;

    protected:
        SmartConnect    m_pConn;

        bool        m_bEnableSwiftNumberSup;
        bool        m_bEnableMD5Check;
        int         m_nPriorityLevel;        
    };

    typedef shared_ptr<CDataTransPackage> SmartPackage;
}


#endif