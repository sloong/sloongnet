#include "dbhub.h"

CResult Sloong::DBHub::Initialize(IControl *ic)
{
    IObject::Initialize(ic);
    auto config = IData::GetModuleConfig();
    MySqlEx mysql;
    auto res = mysql.Connect((*config)["Address"].asString().c_str(), (*config)["Port"].asInt(), (*config)["User"].asString().c_str(),
                             (*config)["Password"].asString().c_str(), "");

    if (res.IsFialed())
        return res;

    m_mapFuncToHandler[DataCenter::Functions::ConnectDatabase] = std::bind(&DBHub::ConnectDatabaseHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[DataCenter::Functions::QuerySQLCmd] = std::bind(&DBHub::QuerySQLCmdHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[DataCenter::Functions::InsertSQLCmd] = std::bind(&DBHub::InsertSQLCmdHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[DataCenter::Functions::UpdateSQLCmd] = std::bind(&DBHub::UpdateSQLCmdHandler, this, std::placeholders::_1, std::placeholders::_2);
    m_mapFuncToHandler[DataCenter::Functions::DeleteSQLCmd] = std::bind(&DBHub::DeleteSQLCmdHandler, this, std::placeholders::_1, std::placeholders::_2);

    return CResult::Succeed;
}

PackageResult Sloong::DBHub::RequestPackageProcesser(DataPackage *pack)
{
    auto function = (Functions)pack->function();
    if (!DataCenter::Functions_IsValid(function))
    {
        return PackageResult::Make_Error(Helper::Format("Parser request package function[%s] error.", pack->content().c_str()));
    }

    auto req_str = pack->content();
    auto func_name = Functions_Name(function);
    m_pLog->Debug(Helper::Format("Request [%d][%s]:[%s]", function, func_name.c_str(), CBase64::Encode(req_str).c_str()));
    if (!m_mapFuncToHandler.exist(function))
    {
        return PackageResult::Make_Error(Helper::Format("Function [%s] no handler.", func_name.c_str()));
    }

    auto res = m_mapFuncToHandler[function](req_str, pack);
    m_pLog->Debug(Helper::Format("Response [%s]:[%s][%s].", func_name.c_str(), ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str()));
    return PackageResult::Make_OK(Package::MakeResponse(pack,res));
}

CResult Sloong::DBHub::ConnectDatabaseHandler(const string &req_obj, DataPackage *pack)
{
    auto req = ConvertStrToObj<ConnectDatabaseRequest>(req_obj);

    if (!m_mapDBNameToSessioinID.exist(req->database()))
    {
        auto config = IData::GetModuleConfig();
        auto connection = make_unique<MySqlEx>();
        auto res = connection->Connect((*config)["Address"].asString().c_str(), (*config)["Port"].asInt(), (*config)["User"].asString().c_str(),
                                       (*config)["Password"].asString().c_str(), req->database());
        if (res.IsFialed())
            return res;

        connection->SetLog(m_pLog);
        auto id = m_mapSessionIDToDBConnection.size() + 1;
        m_mapSessionIDToDBConnection[id] = std::move(connection);
        m_mapDBNameToSessioinID[req->database()] = id;
    }

    auto id = m_mapDBNameToSessioinID.try_get(req->database());
    if (id == nullptr)
        return CResult::Make_Error("Get session id fialed.");

    ConnectDatabaseResponse response;
    response.set_sessionid(*id);
    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::DBHub::QuerySQLCmdHandler(const string &req_obj, DataPackage *pack)
{
    auto req = ConvertStrToObj<QuerySQLCmdRequest>(req_obj);

    auto session = m_mapSessionIDToDBConnection.try_get(req->sessionid());
    if (session == nullptr)
        return CResult::Make_Error("SessionID is invaild.");

    auto res = (*session)->Query(req->sqlcmd());
    if (res.IsFialed())
        return res;

    auto sql_res = res.GetResultObject();
    QuerySQLCmdResponse response;
    for( int i = 0; i< sql_res->GetLinesNum(); i++ )
    {
        auto line = response.add_lines();
        for( int j = 0; j < sql_res->GetColumnsNum(); j++ )
        {
            line->add_rawdataitem(sql_res->GetData(i,j));
        }
    }
    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::DBHub::InsertSQLCmdHandler(const string &req_obj, DataPackage *pack)
{
    auto req = ConvertStrToObj<InsertSQLCmdRequest>(req_obj);

    auto session = m_mapSessionIDToDBConnection.try_get(req->sessionid());
    if (session == nullptr)
        return CResult::Make_Error("SessionID is invaild.");

    auto res = (*session)->Insert(req->sqlcmd());
    if (res.IsFialed())
        return res;

    InsertSQLCmdResponse response;
    response.set_affectedrows(res.GetResultObject());

    if ( req->getidentity() )
    {
        auto id_res = (*session)->Query("SELECT @@IDENTITY");
        if( id_res.IsFialed() )
            response.set_identity(-1);
        else
        {
            auto obj = id_res.GetResultObject();
            int id;
            if( !ConvertStrToInt(obj->GetData(1,1), &id))
                response.set_identity(-1);
            else
                response.set_identity(id);
        }
    }

    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::DBHub::DeleteSQLCmdHandler(const string &req_obj, DataPackage *pack)
{
    auto req = ConvertStrToObj<DeleteSQLCmdRequest>(req_obj);

    auto session = m_mapSessionIDToDBConnection.try_get(req->sessionid());
    if (session == nullptr)
        return CResult::Make_Error("SessionID is invaild.");

    auto res = (*session)->Delete(req->sqlcmd());
    if (res.IsFialed())
        return res;

    DeleteSQLCmdResponse response;
    response.set_affectedrows(res.GetResultObject());

    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::DBHub::UpdateSQLCmdHandler(const string &req_obj, DataPackage *pack)
{
    auto req = ConvertStrToObj<UpdateSQLCmdRequest>(req_obj);

    auto session = m_mapSessionIDToDBConnection.try_get(req->sessionid());
    if (session == nullptr)
        return CResult::Make_Error("SessionID is invaild.");
        
    auto res = (*session)->Update(req->sqlcmd());
    if (res.IsFialed())
        return res;

    UpdateSQLCmdResponse response;
    response.set_affectedrows(res.GetResultObject());

    return CResult::Make_OK(ConvertObjToStr(&response));
}
