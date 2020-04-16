/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-16 16:35:02
 * @Description: file content
 */
#ifndef DATA_TRANS_PACKAGE_H
#define DATA_TRANS_PACKAGE_H

#include "core.h"
#include "utility.h"
#include "EasyConnect.h"
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
        void ResponsePackage(const string& msg, const string& exdata);
		void ResponsePackage(ResultType result, const string& message);
        void ResponsePackage(const CResult& result);

        void RequestPackage( shared_ptr<DataPackage> pack );
        void ResponsePackage( shared_ptr<DataPackage> pack );

	protected:
		void PrepareSendPackageData();

    public:
        /**
         * @Remarks: Receive and create data package. 
         * @Params: 
         * @Return: if package receive succeed, return Succed.
         *          if other error happened else return Error 
         *          if md5 check failed, return Invalied.
         */
        ResultType RecvPackage(int,int=1);

        /**
         * @Remarks: send this package
         * @Params: 
         * @Return: if send fialed, return Error.
         */
        ResultType SendPackage();

        shared_ptr<DataPackage> GetRecvPackage(){ return m_pTransPackage;}

        inline string GetRecvMessage(){ return m_pTransPackage->content(); }

        string GetExtendData() { return m_pTransPackage->extend(); }

        inline int GetSocketID(){return m_pCon->GetSocketID(); }

        inline string GetSocketIP() { return CUtility::GetSocketIP(m_pCon->GetSocketID()); }

        inline int GetSocketPort() { return CUtility::GetSocketPort(m_pCon->GetSocketID()); }

        /**
         * @Remarks: If have ex data send, this package is big package. need add to send list.
         * @Params: 
         * @Return: 
         */
        inline bool IsBigPackage(){ return m_pTransPackage->extend().length() > 0 ? true : false; }

        int GetPriority(){
            return m_pTransPackage->prioritylevel();
        }

        void SetPriority(int value){
            m_pTransPackage->set_prioritylevel(value);
        }

        u_int64_t GetSerialNumber(){
            return m_pTransPackage->serialnumber();
        }

        void SetSerialNumber(u_int64_t value){
            m_pTransPackage->set_serialnumber(value);
        }

        void AddSerialNumber( u_int64_t& value ){
            m_pTransPackage->set_serialnumber(value);
            value++;
        }
    protected:
        // Send data info
        string m_strPackageData;
        int m_nSent=0;
		int m_nPackageSize=0;
        shared_ptr<DataPackage> m_pTransPackage;
    protected:
        SmartConnect    m_pCon;
        CLog*           m_pLog = nullptr;
    };

    typedef shared_ptr<CDataTransPackage> SmartPackage;
}


#endif