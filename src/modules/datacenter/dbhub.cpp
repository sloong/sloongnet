#include "dbhub.h"


CResult Sloong::DBHub::Initialize(IControl *ic)
{
    IObject::Initialize(ic);
    auto config = IData::GetModuleConfig();
    auto res = m_pMySql->Connect((*config)["Address"].asString().c_str(), (*config)["Port"].asInt(), (*config)["User"].asString().c_str(),
                                 (*config)["Password"].asString().c_str(), (*config)["Database"].asString().c_str());

    if (res.IsFialed())
        return res;

    m_mapFuncToHandler[DataCenter::Functions::RunSQL] = std::bind(&DBHub::RunSQLHandler, this, std::placeholders::_1, std::placeholders::_2);


    return CResult::Succeed();
}



CResult Sloong::DBHub::RequestPackageProcesser(CDataTransPackage *pack)
{
	auto function = (Functions)pack->GetFunction();
	if (!DataCenter::Functions_IsValid(function))
	{
		pack->ResponsePackage(ResultType::Error, Helper::Format("Parser request package function[%s] error.", pack->GetRecvMessage().c_str()));
		return CResult::Succeed();
	}

	auto req_obj = pack->GetRecvMessage();
	auto func_name = Functions_Name(function);
	m_pLog->Debug(Helper::Format("Request [%d][%s]:[%s]", function, func_name.c_str(), req_obj.c_str()));
	if (!m_mapFuncToHandler.exist(function))
	{
		pack->ResponsePackage(ResultType::Error, Helper::Format("Function [%s] no handler.", func_name.c_str()));
		return CResult::Succeed();
	}

	auto res = m_mapFuncToHandler[function](req_obj, pack);
	m_pLog->Debug(Helper::Format("Response [%s]:[%s][%s].", func_name.c_str(), ResultType_Name(res.GetResult()).c_str(), res.GetMessage().c_str()));
	pack->ResponsePackage(res);
	return CResult::Succeed();
}



CResult Sloong::DBHub::RunSQLHandler(const string &req_obj, CDataTransPackage *pack)
{
    auto req = ConvertStrToObj<RunSQLRequest>(req_obj);

    vector<string> vRes;
    auto res = m_pMySql->Query(req->sqlcmd(),&vRes);
    if( res.IsFialed())
        return res;

    RunSQLResponse response;
    response.set_affectedrows(res.GetResultObject());
    for( auto& item: vRes)
    {
        auto p = response.add_results();
        *p = item;
    }
    return CResult::Make_OK(ConvertObjToStr(&response));
}