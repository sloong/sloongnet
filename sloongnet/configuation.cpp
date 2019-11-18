/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 17:13:29
 * @Description: file content
 */

#include "configuation.h"
#include "univ/Base64.h"

const static string CONFIG_TPL_TBL_NAME = "configutaion_template";
const static string CONFIGS_TBL_NAME = "configuations";


Sloong::CConfiguation::CConfiguation()
{
    m_pDB = make_unique<CSQLiteEx>();
}


CResult Sloong::CConfiguation::Initialize(string dbPath, string uuid)
{
    m_pDB->Initialize(dbPath);
    LoadConfigTemplate(CONFIG_TPL_TBL_NAME);
    LoadConfig(uuid);
    return CResult::Succeed;
}

CResult Sloong::CConfiguation::LoadConfigTemplate(string tbName)
{
    EasyResult dbRes = make_shared<CDBResult>();
    string error;
    string sql = CUniversal::Format("SELECT `key`,`value` FROM `%s`", tbName.c_str() );

    if (!m_pDB->Query(sql, dbRes, error))
    {
        return CResult(false);
    }
    else if( dbRes->GetLinesNum() >= 0 ) 
    {
        int len = dbRes->GetLinesNum();
        for( int i = 0; i < len; i++ )
        {
            auto key = dbRes->GetData(0,"key");
            auto value = dbRes->GetData(0, "value");
            m_oTemplateList[key] = value;
        }
        return CResult::Succeed;
    }
    else
    {
		return CResult(false, "No support function.");
    }
    
}

/**
 * @Remarks: Load target uuid configuation. 
 * @Params: 
 * @Return: 
 */
CResult Sloong::CConfiguation::LoadConfig(string uuid)
{
	auto config = GetStringConfig(CONFIGS_TBL_NAME, uuid, "");
	if (config.length() > 0)
		m_oServerConfigList[uuid] = CBase64::Decode(config);
    return CResult::Succeed;
}

CResult Sloong::CConfiguation::SaveConfig(string uuid, string config)
{
    // TODO: add sqlite write function.
	map<string, string> kvlist = {
		{"key",uuid},
		{"value", CBase64::Encode(config)},
	};
	auto res = AddOrInsertRecord(CONFIGS_TBL_NAME, kvlist, "");// CUniversal::Format("`key`='%s'", uuid.c_str()));
	if (res.IsSucceed())
	{
		m_oServerConfigList[uuid] = config;
		return CResult::Succeed;
	}
	return res;
}

CResult Sloong::CConfiguation::SaveTemplate( string id, string config)
{
    // TODO: add sqlite write function.
	return CResult::Succeed;
}

CResult Sloong::CConfiguation::ReloadTemplate( string id )
{
    m_oTemplateList[id] = GetStringConfig(CONFIG_TPL_TBL_NAME, id, "");
	return CResult::Succeed;
}

CResult Sloong::CConfiguation::AddOrInsertRecord(const string& table_name,const map<string,string>& list, string where_str)
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
		return CResult(false,error);
	}

	return CResult::Succeed;
}

string Sloong::CConfiguation::GetStringConfig(string table_name, string key, string def)
{
    EasyResult dbRes = make_shared<CDBResult>();
    string error;
    string sql = CUniversal::Format("SELECT `value` FROM `%s` WHERE `key`=\"%s\"",
                                    table_name.c_str(), key.c_str());

    if (!m_pDB->Query(sql, dbRes, error) || dbRes->GetLinesNum() == 0)
    {
        return def;
    }
    else if( dbRes->GetLinesNum() > 0 ) 
    {
        return dbRes->GetData(0,"value");
    }
    else
    {
        throw normal_except("No support function.");
    }
  
}


string Sloong::CConfiguation::GetConfig(string uuid)
{
    if( m_oServerConfigList.find(uuid) == m_oServerConfigList.end() )
        return "";
    return m_oServerConfigList[uuid];
}