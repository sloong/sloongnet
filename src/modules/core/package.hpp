/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-08-23 19:27:02
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/package.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: The extend class for DataPackage.
 */
#pragma once

#include "protocol/base.pb.h"
using namespace Base;

#include <list>
using namespace std;

#include "result.h"

namespace Sloong
{
    class Package
    {
    public:
        uint64_t sessionid() { return SessionID; }
        void set_sessionid(uint64_t sessionid) { SessionID = sessionid; }
        list<uint64_t> clocks() { return Clocks; }
        void add_clocks(uint64_t clock) { Clocks.push_back(clock); }

        /*** 
         * @description: 这个函数只针对需要提前计算正确的包大小的情况，因为hash字段是在发送之前由Connect对象设置的，所以会先检查Hash字段是否设置过，如果没有则会设置空进去，然后计算大小之后再清空hash。
         */
        size_t ByteSizeLongEx()
        {
            auto len = data.ByteSizeLong();
            if (data.hash().length() == 0)
            {
                data.set_hash(string(32, 0x00));
                len = data.ByteSizeLong();
                data.clear_hash();
            }
            return len;
        }

        inline bool SerializeToString(string *str) { return data.SerializeToString(str); }

        inline bool ParseFromString(const string &str) { return data.ParseFromString(str); }

        inline size_t ByteSizeLong() { return data.ByteSizeLong(); }

        inline string ShortDebugString() { return data.ShortDebugString(); }

        inline uint64_t sender() { return data.sender(); }
        inline void set_sender(uint64_t s) { data.set_sender(s); }

        inline string hash() { return data.hash(); }
        inline void set_hash(const string &str) { data.set_hash(str); }
        inline void set_hash(const void *str, int len) { data.set_hash(str, len); }
        inline void clear_hash() { data.clear_hash(); }

        inline const string &extend() { return data.extend(); }
        inline void set_extend(const string &str)
        {
            data.set_extend(str);
            // pack->mutable_extend()->set_hash( CRCEncode32(message));
            // pack->mutable_extend()->set_data( message );
        }
        inline void set_extend(string &&str)
        {
            data.set_extend(str);
            // pack->mutable_extend()->set_hash( CRCEncode32(message));
            // pack->mutable_extend()->set_data( message );
        }
        inline void set_extend(const void *str, int len)
        {
            data.set_extend(str, len);
            // pack->mutable_extend()->set_hash( CRCEncode32(message));
            // pack->mutable_extend()->set_data( message );
        }
        inline void clear_extend() { data.clear_extend(); }

        inline const string &content() { return data.content(); }
        inline void set_content(const string &str)
        {
            data.set_content(str);
            // mutable_content()->set_hash( CRCEncode32(message));
            //pack->mutable_content()->set_data( message );
        }
        inline void set_content(string &&str)
        {
            data.set_content(move(str));
            // mutable_content()->set_hash( CRCEncode32(message));
            //pack->mutable_content()->set_data( message );
        }
        inline void clear_content() { data.clear_content(); }

        inline int32_t function() { return data.function(); }
        inline void set_function(int32_t f) { data.set_function(f); }

        inline int32_t priority() { return data.priority(); }
        inline void set_priority(int32_t p) { data.set_priority(p); }

        inline uint64_t id() { return data.id(); }
        inline void set_id(uint64_t id) { data.set_id(id); }

        inline DataPackage_PackageType type() { return data.type(); }
        inline void set_type(DataPackage_PackageType value) { data.set_type(value); }

        inline DataPackage_StatusType status() { return data.status(); }
        inline void set_status(DataPackage_StatusType s) { data.set_status(s); }

        inline ResultType result() { return data.result(); }
        inline void set_result(ResultType r) { data.set_result(r); }

        static inline unique_ptr<Package> new_unique() { return make_unique<Package>(); }

        static unique_ptr<Package> GetRequestPackage(bool EventPackage = false)
        {
            auto package = Package::new_unique();

            package->data.set_status(DataPackage_StatusType::DataPackage_StatusType_Request);
            if (EventPackage)
                package->data.set_type(DataPackage_PackageType::DataPackage_PackageType_EventPackage);
            else
                package->data.set_type(DataPackage_PackageType::DataPackage_PackageType_NormalPackage);
            return package;
        }

        /**
         * @Description: 
         *      Response datapackage with an exist Package object.
         * @Params: 
         *      1 > The Package object.
         * @Remarks: 
         *      Only used it when you need control everything. 
         *      This function just set the status to response.
         */
        static unique_ptr<Package> MakeResponse(Package *request_pack)
        {
            auto response_pack = Package::new_unique();
            response_pack->set_sessionid(request_pack->sessionid());
            response_pack->set_type(request_pack->data.type());
            response_pack->set_priority(request_pack->priority());
            response_pack->set_function(request_pack->function());
            response_pack->set_id(request_pack->id());
            response_pack->set_status(DataPackage_StatusType::DataPackage_StatusType_Response);
            return response_pack;
        }

        /**
         * @Description: 
         *      Response datapackage only with an result code. 
         * @Params: 
         *      Result type. 
         * @Remarks: 
         *      Only used it when you just need return an result code. 
         *      This function will !!! auto clear the Content and Extend !!!.
         */
        static unique_ptr<Package> MakeOKResponse(Package *request_pack)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->data.set_result(ResultType::Succeed);
            return response_pack;
        }

        /**
         * @Description: 
         *      Response datapackage with content.
         * @Params: 
         *      1 > Result type. 
         *      2 > Response content.
         * @Remarks: 
         *      Only used it when you don't need return the extend data. 
         *      This function will !!! auto clear the Extend !!!.
         */
        static unique_ptr<Package> MakeErrorResponse(Package *request_pack, const string &message)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(ResultType::Error);
            response_pack->set_content(message);
            return response_pack;
        }

        /**
         * @Description: 
         *      Response datapackage with content and extend data.
         * @Params: 
         *      1 > Result type. 
         *      2 > Response content.
         *      3 > Response extend data.
         * @Remarks: 
         *      Only used it when you need return and extend data. 
         *      This package will be see big package, and add to send list.
         */
        static unique_ptr<Package> MakeResponse(Package *request_pack, ResultType result, const string &message, const string &extend)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(result);
            response_pack->set_content(message);
            response_pack->set_extend(extend);
            return response_pack;
        }

        /**
         * @Description: 
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
        static unique_ptr<Package> MakeResponse(Package *request_pack, ResultType result, const string &message, const char *extend, int size)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(result);
            response_pack->set_content(message);
            response_pack->set_extend(extend, size);
            return response_pack;
        }

        /**
         * @Description: 
         *      Response datapackage with content (Recommend)
         * @Params: 
         *      1 > Result object
         * @Remarks: 
         *      Only used it when you don't need return the extend data. 
         *      This function will !!! auto clear the Extend !!!.
         */
        static unique_ptr<Package> MakeResponse(Package *request_pack, const CResult &result)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(result.GetResult());
            response_pack->set_content(result.GetMessage());
            return response_pack;
        }

        DataPackage data;

    private:
        uint64_t SessionID = 0;
        list<uint64_t> Clocks;
    };

    typedef shared_ptr<Package> SmartPackage;
    typedef unique_ptr<Package> UniquePackage;

    typedef TResult<unique_ptr<Package>> PackageResult;

} // namespace Sloong