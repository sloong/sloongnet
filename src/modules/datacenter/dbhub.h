/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-08-06 18:50:09
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/datacenter/dbhub.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
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

        PackageResult RequestPackageProcesser(Package *);

        CResult ConnectDatabaseHandler(const string &req_obj, Package *pack);
        CResult QuerySQLCmdHandler(const string &req_obj, Package *pack);
        CResult InsertSQLCmdHandler(const string &req_obj, Package *pack);
        CResult UpdateSQLCmdHandler(const string &req_obj, Package *pack);
        CResult DeleteSQLCmdHandler(const string &req_obj, Package *pack);

    protected:
        map_ex<DataCenter::Functions, FunctionHandler> m_mapFuncToHandler;
        map_ex<string, int>* m_pMapDBNameToSessioinID = nullptr;
        map_ex<int, map<thread::id,UniqueMySQLEx>>* m_pMapSessionIDToConnections = nullptr;
    };
} // namespace Sloong

#endif