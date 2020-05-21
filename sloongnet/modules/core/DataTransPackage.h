/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 19:10:52
 * @Description: file content
 */
#ifndef DATA_TRANS_PACKAGE_H
#define DATA_TRANS_PACKAGE_H

#include "core.h"
#include "utility.h"
#include "EasyConnect.h"
#include <sys/time.h>
namespace Sloong
{
    class CDataTransPackage
    {
	public:
        CDataTransPackage(EasyConnect* conn):m_pCon(conn){}

		void ResponsePackage(ResultType result, const string& message, const string* exdata =nullptr);
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
        ResultType RecvPackage(int);

        /**
         * @Remarks: send this package
         * @Params: 
         * @Return: if send fialed, return Error.
         */
        ResultType SendPackage();

        inline int GetFunction() { return m_pTransPackage->function();}

        inline shared_ptr<DataPackage> GetRecvPackage(){ return m_pTransPackage;}

        inline string GetRecvMessage(){ return m_pTransPackage->content(); }

        inline string GetExtendData() { return m_pTransPackage->extend(); }

        inline void SetConnection(EasyConnect* conn){ m_pCon = conn; }

        inline EasyConnect* GetConnection() { return m_pCon; }

        inline int GetSocketID(){return m_pCon->GetSocketID(); }

        inline string GetSocketIP() { return CUtility::GetSocketIP(m_pCon->GetSocketID()); }

        inline int GetSocketPort() { return CUtility::GetSocketPort(m_pCon->GetSocketID()); }

        /**
         * @Remarks: If have ex data send, this package is big package. need add to send list.
         * @Params: 
         * @Return: 
         */
        inline bool IsBigPackage(){ return m_pTransPackage->extend().length() > 0 ? true : false; }

        inline int GetPriority(){ return m_pTransPackage->prioritylevel(); }

        inline void SetPriority(int value){ m_pTransPackage->set_prioritylevel(value); }

        inline u_int64_t GetSerialNumber(){ return m_pTransPackage->serialnumber(); }

        inline void SetSerialNumber(u_int64_t value){ m_pTransPackage->set_serialnumber(value); }

        inline void AddSerialNumber( u_int64_t& value ){
            m_pTransPackage->set_serialnumber(value);
            value++;
        }
        inline int GetSentSize(){ return m_nSent; }
        inline int GetPackageSize(){return m_nPackageSize;}

        inline void Record(){ struct  timeval  start;gettimeofday(&start,NULL); m_listClock.push_back(start); }
        string FormatRecord();
    protected:
        // Send data info
        string m_strPackageData;
        int m_nSent=0;
		int m_nPackageSize=0;
        shared_ptr<DataPackage> m_pTransPackage;
        list<timeval> m_listClock;
    protected:
        EasyConnect*    m_pCon;
    public:
        static inline void InitializeLog(CLog* log){ g_pLog = log; }
        static CLog*    g_pLog;
    };

    typedef shared_ptr<CDataTransPackage> SmartPackage;
}


#endif