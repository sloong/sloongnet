/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-09-27 16:17:46
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

#include <sys/time.h>

#include "result.h"

#include "hash.h"

namespace Sloong
{
class Package
{
  public:
    inline uint64_t GetClock()
    {
        struct timespec spec;

        clock_gettime(CLOCK_REALTIME, &spec);

        auto ms = lround(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

        return spec.tv_sec * 1000 + ms;
    }

    // inline string FormatRecord(Package *pack)
    // {
    //     string str;
    //     auto _timeline = pack->_timeline();
    //     auto start = _timeline.begin();
    //     for (auto item = start ++; item != _timeline.end(); item++)
    //     {
    //         str = format("%s[%.2f]", str.c_str(), *item - *start);
    //     }
    //     return str;
    // }

    uint64_t sessionid()
    {
        return SessionID;
    }
    void set_sessionid(uint64_t sessionid)
    {
        SessionID = sessionid;
    }

    auto get_timeline()
    {
        return _timeline;
    }

    void merge_timeline(Package *other)
    {
        for (auto &i : other->get_timeline())
        {
            _timeline.push_back(i);
        }
    }

    /***
     * @description: record current time point to timeline.
     * @param {string&} note: the note message
     */
    void record_point(const string &note)
    {
        _timeline.push_back(std::make_pair(GetClock(), note));
    }
    void record_point(string &&note)
    {
        _timeline.push_back(std::make_pair(GetClock(), move(note)));
    }

    /***
     * @description: This function only for case where the correct package size need to be calculated in advance.
     * because the hash field is set by the Connect obejct before sending So this function will check the hash field and
     * if it is not set, set the empty string into it. and then calculate the package size. and return it. before
     * returned, it will cleanup the hash field if it is set by this function.
     * @return: the package size
     */
    inline size_t ByteSizeLongEx()
    {
        auto len = data.ByteSizeLong();
        if (data.hash().length() == 0)
        {
            // when set the empty to hash, the size for hash maybe different with true hash. because the Protobuf maybe
            // optimize the string storage. especial for empty string. so we calculate the hash and set true value. and
            // then clear it.
            string tmp;
            unsigned char buffer[32] = {0};
            if (data.SerializeToString(&tmp))
                Universal::CSHA256::Binary_Encoding(tmp, buffer);

            data.set_hash(buffer, 32);
            data.clear_hash();
        }
        return len;
    }

    inline bool SerializeToString(string *str)
    {
        return data.SerializeToString(str);
    }

    inline bool ParseFromString(const string &str)
    {
        return data.ParseFromString(str);
    }

    inline size_t ByteSizeLong()
    {
        return data.ByteSizeLong();
    }

    inline string ShortDebugString()
    {
        return data.ShortDebugString();
    }

    inline uint64_t sender()
    {
        return data.sender();
    }
    inline void set_sender(uint64_t s)
    {
        data.set_sender(s);
    }

    inline string hash()
    {
        return data.hash();
    }
    inline void set_hash(const string &str)
    {
        data.set_hash(str);
    }
    inline void set_hash(const void *str, int len)
    {
        data.set_hash(str, len);
    }
    inline void clear_hash()
    {
        data.clear_hash();
    }

    inline const string &extend()
    {
        return data.extend();
    }
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
    inline void clear_extend()
    {
        data.clear_extend();
    }

    inline const string &content()
    {
        return data.content();
    }
    inline void set_content(const string &str)
    {
        data.set_content(str);
        // mutable_content()->set_hash( CRCEncode32(message));
        // pack->mutable_content()->set_data( message );
    }
    inline void set_content(string &&str)
    {
        data.set_content(move(str));
        // mutable_content()->set_hash( CRCEncode32(message));
        // pack->mutable_content()->set_data( message );
    }
    inline void clear_content()
    {
        data.clear_content();
    }

    inline int32_t function()
    {
        return data.function();
    }
    inline void set_function(int32_t f)
    {
        data.set_function(f);
    }

    inline PRIORITY_LEVEL priority()
    {
        return data.priority();
    }
    inline void set_priority(PRIORITY_LEVEL p)
    {
        data.set_priority(p);
    }

    inline uint64_t id()
    {
        return data.id();
    }
    inline void set_id(uint64_t id)
    {
        data.set_id(id);
    }

    inline DataPackage_PackageType type()
    {
        return data.type();
    }
    inline void set_type(DataPackage_PackageType value)
    {
        data.set_type(value);
    }

    inline ResultType result()
    {
        return data.result();
    }
    inline void set_result(ResultType r)
    {
        data.set_result(r);
    }

    static inline unique_ptr<Package> new_unique()
    {
        return make_unique<Package>();
    }

    static unique_ptr<Package> GetRequestPackage()
    {
        auto package = Package::new_unique();
        package->data.set_type(DataPackage_PackageType::DataPackage_PackageType_Request);
        return package;
    }

    static unique_ptr<Package> GetControlEventPackage()
    {
        auto package = Package::new_unique();
        package->data.set_type(DataPackage_PackageType::DataPackage_PackageType_ControlEvent);
        return package;
    }

    static unique_ptr<Package> GetManagerEventPackage()
    {
        auto package = Package::new_unique();
        package->data.set_type(DataPackage_PackageType::DataPackage_PackageType_ManagerEvent);
        return package;
    }

    /**
     * @Description:
     *      Response datapackage with an exist Package object.
     * @Params:
     *      1 > The Package object.
     * @Remarks:
     *      Only used it when you need control everything.
     *      This function just set the type to response.
     */
    static unique_ptr<Package> MakeResponse(Package *request_pack)
    {
        auto response_pack = Package::new_unique();
        response_pack->set_sessionid(request_pack->sessionid());
        response_pack->set_type(DataPackage_PackageType::DataPackage_PackageType_Response);
        response_pack->set_priority(request_pack->priority());
        response_pack->set_function(request_pack->function());
        response_pack->set_id(request_pack->id());
        response_pack->merge_timeline(request_pack);

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
    static unique_ptr<Package> MakeResponse(Package *request_pack, ResultType result, const string &message,
                                            const string &extend)
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
    static unique_ptr<Package> MakeResponse(Package *request_pack, ResultType result, const string &message,
                                            const char *extend, int size)
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

  public:
    DataPackage data;

  private:
    uint64_t SessionID = 0;
    list<std::pair<uint64_t, string>> _timeline;
};

typedef shared_ptr<Package> SmartPackage;
typedef unique_ptr<Package> UniquePackage;

typedef TResult<unique_ptr<Package>> PackageResult;

} // namespace Sloong