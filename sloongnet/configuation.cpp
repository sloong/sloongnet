/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 17:13:29
 * @Description: file content
 */

#include "configuation.h"
#include "univ/Base64.h"

const static string SERVER_TBL_NAME = "server_list";
const static string TEMPLATE_TBL_NAME = "template_list";
const static string CONFIGS_TBL_NAME = "configuations";

Sloong::CConfiguation::CConfiguation()
{
    m_pDB = make_unique<CSQLiteEx>();
}


CResult Sloong::CConfiguation::Initialize(string dbPath, string uuid)
{
    m_pDB->Initialize(dbPath);
    LoadDB();
    GetConfig(uuid);
    return CResult::Succeed;
}

CResult Sloong::CConfiguation::LoadDB()
{
    LoadKeyValueList(SERVER_TBL_NAME, m_oServerList);
    LoadKeyValueList(TEMPLATE_TBL_NAME, m_oTemplateList);
    LoadKeyValueList(CONFIGS_TBL_NAME, m_oConfigList);
    return CResult::Succeed;
}

template<typename K, typename V>
CResult Sloong::CConfiguation::LoadKeyValueList(string tbName, map<K, V>& out_list)
{
    EasyResult dbRes = make_shared<CDBResult>();
    string error;
    string sql = CUniversal::Format("SELECT `key`,`value` FROM `%s`", tbName.c_str());

    if (!m_pDB->Query(sql, dbRes, error))
    {
        return CResult::Make_Error(m_pDB->GetErrorMessage());
    }
    else if (dbRes->GetLinesNum() >= 0)
    {
        int len = dbRes->GetLinesNum();
        for (int i = 0; i < len; i++)
        {
            auto key = dbRes->GetData(0, "key");
            auto value = dbRes->GetData(0, "value");
            out_list[key] = value;
        }
        return CResult::Succeed;
    }
    else
    {
        return CResult::Make_Error("No support function.");
    }

}



string Sloong::CConfiguation::GetConfig(string uuid)
{
    auto type = m_oServerList.try_get(uuid, "");
    if (type == "")
        return "";
    auto types = CUniversal::split(type, '|');
    if (types[0] == "0")
        return GetTemplate(atoi(types[1].c_str()));
    else
        return m_oConfigList.try_get(types[1], "");
}

CResult Sloong::CConfiguation::SetConfig(string uuid, string config)
{
    int config_id = 0;
    string config_value = m_oServerList.try_get(uuid, "");
    if (config_value == "")
    {
        auto res = AddConfig(config, &config_id);
        if (res.IsFialed())
            return res;

        map<string, string> kvlist = {
            {"key",uuid},
            {"value", CUniversal::Format("1|%d",config_id)},
        };
        res = AddOrUpdateRecord(SERVER_TBL_NAME, kvlist, "");
        if (res.IsFialed())
            return res;
    }
    else
    {
        config_id = atoi(CUniversal::split(config_value, '|')[1].c_str());
        map<string, string> kvlist = {
           {"key", CUniversal::ntos(config_id)},
           {"value", CBase64::Encode(config)},
        };
        auto res = AddOrUpdateRecord(CONFIGS_TBL_NAME, kvlist, "");
        if (!res.IsSucceed())
            return res;
    }
    m_oServerList[uuid] = CUniversal::Format("1|%d", config_id);
    m_oConfigList[CUniversal::ntos(config_id)] = config;
    return CResult::Succeed;
}


CResult Sloong::CConfiguation::SetConfigToTemplate(string uuid, int tpl_id)
{
    if( !m_oServerList.exist(uuid) || !m_oTemplateList.exist(CUniversal::ntos(tpl_id)))
        return CResult(ResultType::Error, "uuid or tpl_id error.");

    map<string, string> kvlist = {
        {"key", uuid},
        {"value", CUniversal::Format("0|%d",tpl_id)},
    };
    auto res = AddOrUpdateRecord(CONFIGS_TBL_NAME, kvlist, "");
    if (!res.IsSucceed())
        return res;

    m_oServerList[uuid] = CUniversal::Format("0|%d", tpl_id);
    return CResult::Succeed;
}

string Sloong::CConfiguation::GetTemplate(int id)
{
    auto type = m_oTemplateList.try_get(CUniversal::ntos(id), "-1");
    if (type == "-1")
        return "";
    return m_oConfigList.try_get(type, "");
}

map<int, string> Sloong::CConfiguation::GetTemplateList()
{
    map<int, string> list;
    for_each(m_oTemplateList.begin(), m_oTemplateList.end(), [&](const pair<string, string>& it) {
        list[atoi(it.first.c_str())] = m_oConfigList.try_get(it.second, "");
        });
    return list;
}


CResult Sloong::CConfiguation::AddConfig(string config, int* out_id)
{
    /// Add config to configuations table.
    int config_id_max = 0;
    for_each(m_oConfigList.begin(), m_oConfigList.end(), [&](const pair<string, string>& it) {config_id_max = max(atoi(it.first.c_str()), config_id_max); });
    config_id_max++;
    map<string, string> kvlist = {
        {"key", CUniversal::ntos(config_id_max)},
        {"value", CBase64::Encode(config)},
    };
    auto res = AddOrUpdateRecord(CONFIGS_TBL_NAME, kvlist, "");
    if (!res.IsSucceed())
        return res;

    m_oConfigList[CUniversal::ntos(config_id_max)] = config;
    if (out_id != nullptr)
        *out_id = config_id_max;
    return CResult::Succeed;
}


CResult Sloong::CConfiguation::AddTemplate( string config, int* out_id)
{
    int config_id = 0;
    auto res = AddConfig(config, &config_id);
    if (res.IsFialed())
        return res;

    /// add template map to data table
    int tpl_id_max = 0;
    for_each(m_oTemplateList.begin(), m_oTemplateList.end(), [&](const pair<string, string>& it) {tpl_id_max = max(atoi(it.first.c_str()), tpl_id_max); });
    tpl_id_max++;
    map<string, string> kvlist = {
        {"key", CUniversal::ntos(tpl_id_max)},
        {"value", CUniversal::ntos(config_id)},
    };
    res = AddOrUpdateRecord(TEMPLATE_TBL_NAME, kvlist, "");
    if (!res.IsSucceed())
        return res;

    
    m_oTemplateList[CUniversal::ntos(tpl_id_max)] = config_id;
    if (out_id != nullptr) 
        *out_id = tpl_id_max;
    return CResult::Succeed;
}


CResult Sloong::CConfiguation::SetTemplate( int id, string config)
{
    auto config_id = m_oTemplateList.try_get(CUniversal::ntos(id), "-1");
    if (config_id == "-1") 
        return CResult(ResultType::Error,"No find id in template list.");
    if( !m_oConfigList.exist(config_id)) 
        return CResult(ResultType::Error, "No find id in configuations list.");
    map<string, string> kvlist = {
        {"key",config_id},
        {"value", CBase64::Encode(config)},
    };
    auto res = AddOrUpdateRecord(CONFIGS_TBL_NAME, kvlist, "");
    if (res.IsSucceed())
    {
        m_oConfigList[config_id] = config;
        return CResult::Succeed;
    }
	return res;
}

CResult Sloong::CConfiguation::AddOrUpdateRecord(const string& table_name,const map<string,string>& list, string where_str)
{
	auto i = list.begin();
	string key_list = "'" + i->first + "'";
	string value_list = "'" + i->second + "'";
	for ( i++ ; i != list.end(); i++) {
		if (i->first.length() == 0)
			continue;
		key_list = CUniversal::Format("%s,'%s'", key_list, i->first.c_str());
		value_list = CUniversal::Format("%s,'%s'", value_list, i->second.c_str());
	}
	string sql = CUniversal::Format("REPLACE INTO %s (%s) VALUES(%s)",
		table_name.c_str(), key_list.c_str(), value_list.c_str());
	if (where_str.length() > 0)
		sql += " ON DUPLICATE KEY UPDATE " + where_str;

	EasyResult dbRes = make_shared<CDBResult>();
	string error;
	if (!m_pDB->Query(sql, dbRes, error))
	{
		return CResult::Make_Error(error);
	}

	return CResult::Succeed;
}

CResult Sloong::CConfiguation::GetStringConfig(string table_name, string key, string& outValue)
{
    EasyResult dbRes = make_shared<CDBResult>();
    string error;
    string sql = CUniversal::Format("SELECT `value` FROM `%s` WHERE `key`=\"%s\"",
                                    table_name.c_str(), key.c_str());

    if (!m_pDB->Query(sql, dbRes, error) )
    {
        return CResult(ResultType::Error, m_pDB->GetErrorMessage());
    }
    else if (dbRes->GetLinesNum() == 0)
    {
        return CResult(ResultType::Retry);
    }
    else if( dbRes->GetLinesNum() > 0 ) 
    {
        outValue = dbRes->GetData(0, "value");
        return CResult::Succeed;
    }
    else
    {
        return CResult(ResultType::Error, "Unknow error.");
    }
  
}
