#ifndef SLOONGNET_MODULE_DATACENTER_DBHUB_H
#define SLOONGNET_MODULE_DATACENTER_DBHUB_H

#include "mysqlex.h"


#include "IData.h"
#include "IObject.h"

#include "protocol/datacenter.pb.h"
using namespace DataCenter;

namespace Sloong
{

    class DBHub : public IObject
    {
    public:
        CResult Initialize(IControl *ic);

        PackageResult RequestPackageProcesser(DataPackage *);

        CResult ConnectDatabaseHandler(const string &req_obj, DataPackage *pack);
        CResult QuerySQLCmdHandler(const string &req_obj, DataPackage *pack);
        CResult InsertSQLCmdHandler(const string &req_obj, DataPackage *pack);
        CResult UpdateSQLCmdHandler(const string &req_obj, DataPackage *pack);
        CResult DeleteSQLCmdHandler(const string &req_obj, DataPackage *pack);

    protected:
        map_ex<DataCenter::Functions, FunctionHandler> m_mapFuncToHandler;
        map_ex<int, unique_ptr<MySqlEx>> m_mapSessionIDToDBConnection;
        map_ex<string, int> m_mapDBNameToSessioinID;
    };
} // namespace Sloong

#endif