/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-11 10:44:18
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/PackageHelper.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#pragma once

#include "package.hpp"
#include "result.h"

namespace Sloong
{
    class PackageHelper
    {
    public:
        static inline UniquePackage new_unique() { return make_unique<Package>(); }
        static inline void SetContent(Package *pack, const string &message)
        {
            pack->set_content(message);
            // mutable_content()->set_hash( CRCEncode32(message));
            //pack->mutable_content()->set_data( message );
        }

        static inline void SetExtend(Package *pack, const string &message)
        {
            pack->set_extend(message);
            // pack->mutable_extend()->set_hash( CRCEncode32(message));
            // pack->mutable_extend()->set_data( message );
        }

        static inline void SetExtend(Package *pack, const char *extend, int size)
        {
            pack->set_extend(extend, size);
            // pack->mutable_extend()->set_hash( CRCEncode32( extend));
            // pack->mutable_extend()->set_data( extend, size );
        }

        static UniquePackage GetRequestPackage(bool EventPackage = false)
        {
            auto package = PackageHelper::new_unique();

            package->set_status(DataPackage_StatusType::DataPackage_StatusType_Request);
            if (EventPackage)
                package->set_type(DataPackage_PackageType::DataPackage_PackageType_EventPackage);
            else
                package->set_type(DataPackage_PackageType::DataPackage_PackageType_NormalPackage);
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
        static UniquePackage MakeResponse(Package *request_pack)
        {
            auto response_pack = PackageHelper::new_unique();
            response_pack->set_type(request_pack->type());
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
        static UniquePackage MakeOKResponse(Package *request_pack)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(ResultType::Succeed);
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
        static UniquePackage MakeErrorResponse(Package *request_pack, const string &message)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(ResultType::Error);
            SetContent(response_pack.get(), message);
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
        static UniquePackage MakeResponse(Package *request_pack, ResultType result, const string &message, const string &extend)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(result);
            SetContent(response_pack.get(), message);
            SetContent(response_pack.get(), extend);
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
        static UniquePackage MakeResponse(Package *request_pack, ResultType result, const string &message, const char *extend, int size)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(result);
            SetContent(response_pack.get(), message);
            SetExtend(response_pack.get(), extend, size);
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
        static UniquePackage MakeResponse(Package *request_pack, const CResult &result)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(result.GetResult());
            SetContent(response_pack.get(), result.GetMessage());
            return response_pack;
        }
    };

} // namespace Sloong