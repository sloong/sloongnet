#include "dbhub.h"

#include "datacenter.h"

CResult Sloong::DBHub::Initialize(IControl *ic)
{
    IObject::Initialize(ic);
    auto config = IData::GetModuleConfig();
    MySqlEx mysql;
    auto res = mysql.Connect((*config)["Address"].asString(), (*config)["Port"].asInt(), (*config)["User"].asString(),
                             (*config)["Password"].asString(), "");

    if (res.IsFialed())
        return res;

    auto m = ic->Get(DATACENTER_DATAITEM::MapSessionIDToConnection);
    m_pMapSessionIDToConnections = STATIC_TRANS<map_ex<int, map_ex<thread::id,UniqueMySQLEx>>*>(m);
    m = ic->Get(DATACENTER_DATAITEM::MapDBNameToSessionID);
    m_pMapDBNameToSessioinID = STATIC_TRANS<map_ex<string, int>*>(m);

    m_mapFuncToHandler.insert(DataCenter::Functions::ConnectDatabase, std::bind(&DBHub::ConnectDatabaseHandler, this, std::placeholders::_1, std::placeholders::_2));
    m_mapFuncToHandler.insert(DataCenter::Functions::QuerySQLCmd , std::bind(&DBHub::QuerySQLCmdHandler, this, std::placeholders::_1, std::placeholders::_2 ));
    m_mapFuncToHandler.insert(DataCenter::Functions::InsertSQLCmd, std::bind(&DBHub::InsertSQLCmdHandler, this, std::placeholders::_1, std::placeholders::_2));
    m_mapFuncToHandler.insert(DataCenter::Functions::UpdateSQLCmd, std::bind(&DBHub::UpdateSQLCmdHandler, this, std::placeholders::_1, std::placeholders::_2));
    m_mapFuncToHandler.insert(DataCenter::Functions::DeleteSQLCmd, std::bind(&DBHub::DeleteSQLCmdHandler, this, std::placeholders::_1, std::placeholders::_2));

    return CResult::Succeed;
}

PackageResult Sloong::DBHub::RequestPackageProcesser(Package *pack)
{
    auto function = (Functions)pack->function();
    if (!DataCenter::Functions_IsValid(function))
    {
        return PackageResult::Make_Error(format("Parser request package function[{}] error.", pack->content()));
    }

    auto req_str = pack->content();
    auto func_name = Functions_Name(function);
    m_pLog->debug(format("Request [{}][{}]:[{}]", function, func_name, CBase64::Encode(req_str)));
    if (!m_mapFuncToHandler.exist(function))
    {
        return PackageResult::Make_Error(format("Function [{}] no handler.", func_name));
    }

    auto res = m_mapFuncToHandler.get(function)(req_str, pack);
   if (res.IsError())
		m_pLog->warn(format("Response [{}]:[{}][{}].", func_name, ResultType_Name(res.GetResult()), res.GetMessage()));
	else
		m_pLog->debug(format("Response [{}]:[{}]", func_name, ResultType_Name(res.GetResult())));
    return PackageResult::Make_OKResult(Package::MakeResponse(pack,res));
}

CResult Sloong::DBHub::ConnectDatabaseHandler(const string &req_obj, Package *pack)
{
    auto req = ConvertStrToObj<ConnectDatabaseRequest>(req_obj);

    if (!m_pMapDBNameToSessioinID->exist(req->database()))
    {
        auto config = IData::GetModuleConfig();
        auto sessions = map_ex<thread::id,UniqueMySQLEx>();
        sessions.insert(this_thread::get_id(),make_unique<MySqlEx>());

        auto connection = sessions.get(this_thread::get_id()).get();
        auto res = connection->Connect((*config)["Address"].asString(), (*config)["Port"].asInt(), (*config)["User"].asString(),
                                       (*config)["Password"].asString(), req->database());
        if (res.IsFialed())
            return res;

        connection->SetLog(m_pLog);
        // TODO：这里直接使用size自增来作为key是非常不保险的，在多线程的情况下很容易冲突。后续需要进行优化
        auto id =  m_pMapSessionIDToConnections->size() + 1;
         (*m_pMapSessionIDToConnections).get(id).merge(sessions);
        (*m_pMapDBNameToSessioinID).insert(req->database(), id);
    }

    auto id = m_pMapDBNameToSessioinID->try_get(req->database());
    if (id == nullptr)
        return CResult::Make_Error("Get session id fialed.");

    ConnectDatabaseResponse response;
    response.set_session(*id);
    m_pLog->debug( format("Connect to database [{}] succeed. session id [{}]", req->database(), response.session() ));
    return CResult::Make_OK(ConvertObjToStr(&response));
}

inline MySqlEx* GetCurrentThreadConnextion(  map_ex<thread::id,UniqueMySQLEx>* sessions )
{
    auto id = this_thread::get_id();
    auto it = sessions->find(id);
    if( it == sessions->end() )
    {
        sessions->insert(id,sessions->begin()->second->Duplicate());
    }
    return sessions->get(id).get();
}

CResult Sloong::DBHub::QuerySQLCmdHandler(const string &req_obj, Package *pack)
{
    auto req = ConvertStrToObj<QuerySQLCmdRequest>(req_obj);

    auto session = m_pMapSessionIDToConnections->try_get(req->session());
    if (session == nullptr)
        return CResult::Make_Error("SessionID is invaild.");

    auto res = GetCurrentThreadConnextion(session)->Query(req->sqlcmd());
    if (res.IsFialed())
        return move(res);

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

CResult Sloong::DBHub::InsertSQLCmdHandler(const string &req_obj, Package *pack)
{
    auto req = ConvertStrToObj<InsertSQLCmdRequest>(req_obj);

    auto session = m_pMapSessionIDToConnections->try_get(req->session());
    if (session == nullptr)
        return CResult::Make_Error("SessionID is invaild.");

    auto s = GetCurrentThreadConnextion(session);
    auto res = s->Insert(req->sqlcmd());
    if (res.IsFialed())
        return move(res);

    InsertSQLCmdResponse response;
    response.set_affectedrows(res.GetResultObject());

    if ( req->getidentity() )
    {
        auto id_res = s->Query("SELECT @@IDENTITY");
        if( id_res.IsFialed() )
            response.set_identity(-1);
        else
        {
            auto obj = id_res.GetResultObject();
            int id;
            if( !ConvertStrToInt(obj->GetData(0,0), &id))
                response.set_identity(-1);
            else
                response.set_identity(id);
        }
    }

    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::DBHub::DeleteSQLCmdHandler(const string &req_obj, Package *pack)
{
    auto req = ConvertStrToObj<DeleteSQLCmdRequest>(req_obj);

    auto session = m_pMapSessionIDToConnections->try_get(req->session());
    if (session == nullptr)
        return CResult::Make_Error("SessionID is invaild.");

    auto res = GetCurrentThreadConnextion(session)->Delete(req->sqlcmd());
    if (res.IsFialed())
        return move(res);

    DeleteSQLCmdResponse response;
    response.set_affectedrows(res.GetResultObject());

    return CResult::Make_OK(ConvertObjToStr(&response));
}

CResult Sloong::DBHub::UpdateSQLCmdHandler(const string &req_obj, Package *pack)
{
    auto req = ConvertStrToObj<UpdateSQLCmdRequest>(req_obj);

    auto session = m_pMapSessionIDToConnections->try_get(req->session());
    if (session == nullptr)
        return CResult::Make_Error("SessionID is invaild.");
        
    auto res = GetCurrentThreadConnextion(session)->Update(req->sqlcmd());
    if (res.IsFialed())
        return move(res);

    UpdateSQLCmdResponse response;
    response.set_affectedrows(res.GetResultObject());

    return CResult::Make_OK(ConvertObjToStr(&response));
}
