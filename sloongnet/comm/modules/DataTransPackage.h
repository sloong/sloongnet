#ifndef DATA_TRANS_PACKAGE_H
#define DATA_TRANS_PACKAGE_H

#include "main.h"
#include "config.pb.h"
#include "EasyConnect.h"

using namespace ProtobufMessage;

namespace Sloong
{
    class CDataTransPackage
    {
	public:
        void Initialize(SmartConnect conn, CLog* log= nullptr);


        /**
         * @Remarks: When process done, should call this function to response this package.
         * @Params: 
         * @Return: 
         */
        void ResponsePackage(const string& msg, const char* exData = nullptr, int exSize = 0);

        void RequestPackage( shared_ptr<MessagePackage> pack );
        void ResponsePackage( shared_ptr<MessagePackage> pack );

        void PrepareSendPackageData( const string& msg, const char* exData = nullptr, int exSize = 0);

    public:
        /**
         * @Remarks: Receive and create data package. 
         * @Params: 
         * @Return: if package receive succeed, return Succed.
         *          if other error happened else return Error 
         *          if md5 check failed, return Invalied.
         */
        NetworkResult RecvPackage(int );

        /**
         * @Remarks: send this package
         * @Params: 
         * @Return: if send fialed, return Error.
         */
        NetworkResult SendPackage();

        shared_ptr<MessagePackage> GetRecvPackage(){ return m_pReceivedPackage;}

        inline string GetRecvMessage(){ return m_pReceivedPackage->context(); }

        inline int GetSocketID(){return m_pCon->GetSocketID(); }

        /**
         * @Remarks: If have ex data send, this package is big package. need add to send list.
         * @Params: 
         * @Return: 
         */
        inline bool IsBigPackage(){ return m_pReceivedPackage->extenddata().length() > 0 ? true : false; }

        int GetPriority(){
            return m_pReceivedPackage->prioritylevel();
        }

        void SetPriority(int value){
            m_pReceivedPackage->set_prioritylevel(value);
        }

        u_int64_t GetSerialNumber(){
            return m_pReceivedPackage->serialnumber();
        }

        void SetSerialNumber(u_int64_t value){
            m_pReceivedPackage->set_serialnumber(value);
        }

        void AddSerialNumber( u_int64_t& value ){
            m_pReceivedPackage->set_serialnumber(value);
            value++;
        }
    protected:
        // Send data info
        string m_strPackageData;
        int m_nSent=0;
		int m_nPackageSize=0;
        shared_ptr<MessagePackage> m_pReceivedPackage;
    protected:
        SmartConnect    m_pCon;
        CLog*           m_pLog = nullptr;
    };

    typedef shared_ptr<CDataTransPackage> SmartPackage;
}


#endif