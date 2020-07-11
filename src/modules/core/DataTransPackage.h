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
        CDataTransPackage(EasyConnect *conn) : m_pCon(conn) { m_nSocket = m_pCon->GetSocketID(); }

        void RequestPackage();
        void RequestPackage(const DataPackage &);

        /**
         * @Summary: 
         *      Response datapackage only with an result code. 
         * @Params: 
         *      Result type. 
         * @Remarks: 
         *      Only used it when you just need return an result code. 
         *      This function will !!! auto clear the Content and Extend !!!.
         */
        void ResponsePackage(ResultType);

        /**
         * @Summary: 
         *      Response datapackage with content (Recommend)
         * @Params: 
         *      1 > Result object
         * @Remarks: 
         *      Only used it when you don't need return the extend data. 
         *      This function will !!! auto clear the Extend !!!.
         */
        void ResponsePackage(const CResult &);

        /**
         * @Summary: 
         *      Response datapackage with content.
         * @Params: 
         *      1 > Result type. 
         *      2 > Response content.
         * @Remarks: 
         *      Only used it when you don't need return the extend data. 
         *      This function will !!! auto clear the Extend !!!.
         */
        void ResponsePackage(ResultType, const string &);
        
        /**
         * @Summary: 
         *      Response datapackage with content and extend data.
         * @Params: 
         *      1 > Result type. 
         *      2 > Response content.
         *      3 > Response extend data.
         * @Remarks: 
         *      Only used it when you need return and extend data. 
         *      This package will be see big package, and add to send list.
         */
        void ResponsePackage(ResultType, const string &, const string& extend);

        /**
         * @Summary: 
         *      Response datapackage with content and extend data.
         * @Params: 
         *      1 > Result type. 
         *      2 > Response content.
         *      3 > Response extend data pointer.
         *      4 > Response extend data size.
         * @Remarks: 
         *      Only used it when you need return and extend data. 
         *      This package will be see big package, and add to send list.
         */
        void ResponsePackage(ResultType, const string &, const char* exnted, int size);

        /**
         * @Summary: 
         *      Response datapackage with an exist DataPackage object.
         * @Params: 
         *      1 > The DataPackage object.
         * @Remarks: 
         *      Only used it when you need control everything. 
         *      This function just set the status to response.
         */
        void ResponsePackage(DataPackage*);


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
        ResultType RecvPackage(bool=false);

        /**
         * @Remarks: send this package
         * @Params: 
         * @Return: if send fialed, return Error.
         */
        ResultType SendPackage();

        inline int GetFunction() { return m_oTransPackage.function(); }

        inline uint64_t GetSender() { return m_oTransPackage.sender(); }

        inline DataPackage *GetDataPackage() { return &m_oTransPackage; }

        inline string GetRecvMessage() { return m_oTransPackage.content(); }

        inline string GetExtendData() { return m_oTransPackage.extend(); }

        inline void SetConnection(EasyConnect *conn)
        {
            m_pCon = conn;
            m_nSocket = m_pCon->GetSocketID();
        }
        inline void ClearConnection() { m_pCon = nullptr; }
        inline void SetSocket(SOCKET sock) { m_nSocket = sock; }

        inline EasyConnect *GetConnection() { return m_pCon; }

        inline int GetSocketID() { return m_nSocket; }

        inline string GetSocketIP() { return CUtility::GetSocketIP(m_nSocket); }

        inline int GetSocketPort() { return CUtility::GetSocketPort(m_nSocket); }

        /**
         * @Remarks: If have ex data send, this package is big package. need add to send list.
         * @Params: 
         * @Return: 
         */
        inline bool IsBigPackage() { return m_oTransPackage.extend().length() > 0 ? true : false; }

        inline int GetPriority() { return m_oTransPackage.priority(); }

        inline void SetPriority(int value) { m_oTransPackage.set_priority(value); }

        inline uint64_t GetSerialNumber() { return m_oTransPackage.id(); }

        inline void SetSerialNumber(uint64_t value) { m_oTransPackage.set_id(value); }

        inline int GetSentSize() { return m_nSent; }
        inline int GetPackageSize() { return m_nPackageSize; }

        inline timeval GetTimeval()
        {
            struct timeval start;
            gettimeofday(&start, NULL);
            return start;
        }
        inline void Record() { m_listClock.push_back(GetTimeval()); }
        inline list<timeval> *GetRecord() { return &m_listClock; }

        string FormatRecord();

    protected:
        ResultType RecvPackageSucceedProcess(const string&);

    protected:
        // Send data info
        string m_strPackageData;
        int m_nSent = 0;
        int m_nReceived = 0;
        int m_nPackageSize = 0;
        DataPackage m_oTransPackage;
        list<timeval> m_listClock;

    protected:
        EasyConnect *m_pCon = nullptr;
        SOCKET m_nSocket = INVALID_SOCKET;

    public:
        static CLog *g_pLog;
        static inline void InitializeLog(CLog *log) { g_pLog = log; }
        static size_t g_max_package_size; // In default it;s 5MB
        static inline void SetPackageSizeLimit( int size_of_byte ){  g_max_package_size = size_of_byte; }
    };
    typedef unique_ptr<CDataTransPackage> UniqueTransPackage;
} // namespace Sloong

#endif