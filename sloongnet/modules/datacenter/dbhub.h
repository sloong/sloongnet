#ifndef SLOONGNET_MODULE_DATACENTER_DBHUB_H
#define SLOONGNET_MODULE_DATACENTER_DBHUB_H

#include "mysqlex.h"
#include "DataTransPackage.h"

#include "IData.h"
#include "IObject.h"

#include "protocol/datacenter.pb.h"
using namespace DataCenter;

namespace Sloong
{


    class DBHub : public IObject
    {
        public:
        CResult Initialize( IControl* ic );

        CResult RequestPackageProcesser(CDataTransPackage *);

        CResult RunSQLHandler(const string &req_obj, CDataTransPackage *pack);

        protected:
        map_ex<DataCenter::Functions, FunctionHandler> m_mapFuncToHandler;
        unique_ptr<MySqlEx> m_pMySql = nullptr;
    };
}


#endif