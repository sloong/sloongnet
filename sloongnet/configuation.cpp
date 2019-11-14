/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 17:13:29
 * @Description: file content
 */

#include "configuation.h"

const static string CONFIG_TPL_TBL_NAME = "configutaion_template";
const static string CONFIGS_TBL_NAME = "configuations";


Sloong::CConfiguation::CConfiguation()
{
    m_pDB = make_unique<CSQLiteEx>();
}


bool Sloong::CConfiguation::Initialize(string dbPath, string uuid)
{
    m_pDB->Initialize(dbPath);
    LoadConfigTemplate(CONFIG_TPL_TBL_NAME);
    LoadConfig(uuid);
    return true;
}

bool Sloong::CConfiguation::LoadConfigTemplate(string tbName)
{
    EasyResult dbRes = make_shared<CDBResult>();
    string error;
    string sql = CUniversal::Format("SELECT `key`,`value` FROM `%s`", tbName.c_str() );

    if (!m_pDB->Query(sql, dbRes, error))
    {
        return false;
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
        return true;
    }
    else
    {
        throw normal_except("No support function.");
    }
    return false;
}

/**
 * @Remarks: Load target uuid configuation. 
 * @Params: 
 * @Return: 
 */
bool Sloong::CConfiguation::LoadConfig(string uuid)
{
    m_oServerConfigList[uuid] = GetStringConfig(CONFIGS_TBL_NAME,uuid,"");
    return true;
}

bool Sloong::CConfiguation::SaveConfig(string uuid, string config)
{
    // TODO: add sqlite write function.
	return true;
}

bool Sloong::CConfiguation::SaveTemplate( string id, string config)
{
    // TODO: add sqlite write function.
	return true;
}

bool Sloong::CConfiguation::ReloadTemplate( string id )
{
    m_oTemplateList[id] = GetStringConfig(CONFIG_TPL_TBL_NAME, id, "");
	return true;
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