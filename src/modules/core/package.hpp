#pragma once

#include "result.h"

namespace Sloong
{
    typedef shared_ptr<DataPackage> SmartPackage;
    typedef unique_ptr<DataPackage> UniquePackage;

    class Package
    {
    public:
        static UniquePackage GetRequestPackage( bool EventPackage = false )
        {
            auto package = make_unique<DataPackage>();
            
            package->set_status(DataPackage_StatusType::DataPackage_StatusType_Request);
            if( EventPackage )
                package->set_type(DataPackage_PackageType::DataPackage_PackageType_EventPackage);
            else
                package->set_type(DataPackage_PackageType::DataPackage_PackageType_NormalPackage);
            return package;
        }

        /**
         * @Description: 
         *      Response datapackage with an exist DataPackage object.
         * @Params: 
         *      1 > The DataPackage object.
         * @Remarks: 
         *      Only used it when you need control everything. 
         *      This function just set the status to response.
         */
        static UniquePackage MakeResponse(DataPackage *request_pack)
        {
            auto response_pack = make_unique<DataPackage>();
            response_pack->set_type(request_pack->type());
            response_pack->set_priority(request_pack->priority());
            response_pack->set_function(request_pack->function());
            response_pack->set_id(request_pack->id());
            response_pack->set_status(DataPackage_StatusType::DataPackage_StatusType_Response);
            
            response_pack->mutable_reserved()->set_sessionid(request_pack->reserved().sessionid());
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
        static UniquePackage MakeOKResponse(DataPackage *request_pack)
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
        static UniquePackage MakeErrorResponse(DataPackage *request_pack, const string &message)
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
        static UniquePackage MakeResponse(DataPackage *request_pack, ResultType result, const string &message, const string &extend)
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
        static UniquePackage MakeResponse(DataPackage *request_pack, ResultType result, const string &message, const char *extend, int size)
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
        static UniquePackage MakeResponse(DataPackage *request_pack, const CResult &result)
        {
            auto response_pack = MakeResponse(request_pack);
            response_pack->set_result(result.GetResult());
            response_pack->set_content(result.GetMessage());
            return response_pack;
        }
    };

} // namespace Sloong