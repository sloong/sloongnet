/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 17:13:29
 * @Description: file content
 */

#include "configuation.h"
#include "univ/Base64.h"


Sloong::CConfiguation::CConfiguation()
{
}

CResult Sloong::CConfiguation::Initialize(const string& dbPath)
{
    m_oStorage = make_unique<Storage>(InitStorage(dbPath));
    return CResult::Succeed;
}

CResult Sloong::CConfiguation::GetConfig(const string& uuid)
{
    try
    {
        auto module_item = m_oStorage->get<ModuleInfo>(uuid);
        return CResult::Make_OK(CBase64::Decode(m_oStorage->get<ConfigInfo>(module_item.configuation_id).configuation));
    }
    catch (exception ex)
    {
        return CResult::Make_Error(ex.what());
    }
}

CResult Sloong::CConfiguation::SetConfig(const string& uuid, string config)
{
    try
    {
        // Step 1: check the config for uuid is not exist in module_list table.
        auto item = m_oStorage->get_no_throw<ModuleInfo>(uuid);

        if (item == nullptr)
        {
            // Step 1.1: no exist , create in configuations table.
            auto id = m_oStorage->insert<ConfigInfo>(ConfigInfo{ -1,CBase64::Encode(config) });
            ModuleInfo info;
            info.uuid = uuid;
            info.configuation_id = id;
            m_oStorage->insert<ModuleInfo>(info);
        }
        else
        {
            auto config_item = m_oStorage->get<ConfigInfo>(item->configuation_id);
            config_item.configuation = CBase64::Encode(config);
            m_oStorage->update<ConfigInfo>(config_item);
        }
        return CResult::Succeed;
    }
    catch (exception ex)
    {
        return CResult::Make_Error(ex.what());
    }

}


CResult Sloong::CConfiguation::SetConfigToTemplate(const string& uuid, int tpl_id)
{
    try
    {
        auto template_item = m_oStorage->get_pointer<TemplateInfo>(tpl_id);
        if (template_item == nullptr)
            return CResult::Make_Error(CUniversal::Format("Template id [%s] is no exist.", tpl_id));

        auto module_item = m_oStorage->get_no_throw<ModuleInfo>(uuid);
        if (module_item == nullptr)
        {
            ModuleInfo info;
            info.uuid = uuid;
            info.template_id = template_item->id;
            info.configuation_id = template_item->configuation_id;
            m_oStorage->insert<ModuleInfo>(info);
            return CResult::Succeed;
        }
        else
        {
            module_item->template_id = tpl_id;
            m_oStorage->update(*module_item.get());
            return CResult::Succeed;
        }
    }
    catch (exception ex)
    {
        return CResult::Make_Error(ex.what());
    }
}

CResult Sloong::CConfiguation::GetTemplate(int id)
{
    try
    {
        auto template_item = m_oStorage->get<TemplateInfo>(id);
        return CResult::Make_OK(CBase64::Decode(m_oStorage->get<ConfigInfo>(template_item.configuation_id).configuation));
    }
    catch (exception ex)
    {
        return CResult::Make_Error(ex.what());
    }
}

map<int, string> Sloong::CConfiguation::GetTemplateList()
{
    map<int, string> list;
    try
    {
        auto all_template = m_oStorage->get_all<TemplateInfo>();
        
        for (auto& it : all_template)
        {
            list[it.id] = it.name;
        }
        return list;
    }
    catch (exception ex)
    {
        return list;
    }
}


TResult<int> Sloong::CConfiguation::AddConfig(string config)
{
    try
    {
        auto id = m_oStorage->insert<ConfigInfo>(ConfigInfo{ -1,CBase64::Encode(config) });
        return TResult<int>::Make_OK(id);
    }
    catch (exception ex)
    {
        return TResult<int>::Make_Error(ex.what());
    }
}


CResult Sloong::CConfiguation::AddTemplate( string config, int* out_id)
{
    auto res = AddConfig(config);
    if (res.IsFialed())
        return res;

    try
    {
        TemplateInfo item;
        item.configuation_id = res.ResultObject();
        item.id = -1;
        auto id = m_oStorage->insert<TemplateInfo>(item);
        return CResult::Succeed;
    }
    catch (exception ex)
    {
        return TResult<int>::Make_Error(ex.what());
    }
}


CResult Sloong::CConfiguation::SetTemplate( int id, string config)
{
    try
    {
        auto template_item = m_oStorage->get<TemplateInfo>(id);
        auto config_item = m_oStorage->get<ConfigInfo>(template_item.configuation_id);
        config_item.configuation = CBase64::Encode(config);
        m_oStorage->update(config_item);
    }
    catch (exception ex)
    {
        return CResult::Make_Error(ex.what());
    }
    return CResult::Succeed;
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
	

	return CResult::Succeed;
}

CResult Sloong::CConfiguation::GetStringConfig(string table_name, string key, string& outValue)
{
    EasyResult dbRes = make_shared<CDBResult>();
    string error;
    string sql = CUniversal::Format("SELECT `value` FROM `%s` WHERE `key`=\"%s\"",
                                    table_name.c_str(), key.c_str());

    /*if (!m_pDB->Query(sql, dbRes, error) )
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
    }*/
  
}
