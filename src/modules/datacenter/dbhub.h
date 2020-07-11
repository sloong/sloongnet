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
        CResult Initialize(IControl *ic);

        CResult RequestPackageProcesser(CDataTransPackage *);

        CResult ConnectDatabaseHandler(const string &req_obj, CDataTransPackage *pack);
        CResult QuerySQLCmdHandler(const string &req_obj, CDataTransPackage *pack);
        CResult InsertSQLCmdHandler(const string &req_obj, CDataTransPackage *pack);
        CResult UpdateSQLCmdHandler(const string &req_obj, CDataTransPackage *pack);
        CResult DeleteSQLCmdHandler(const string &req_obj, CDataTransPackage *pack);

    protected:
        map_ex<DataCenter::Functions, FunctionHandler> m_mapFuncToHandler;
        map_ex<int, unique_ptr<MySqlEx>> m_mapSessionIDToDBConnection;
        map_ex<string, int> m_mapDBNameToSessioinID;
    };
} // namespace Sloong

#endif