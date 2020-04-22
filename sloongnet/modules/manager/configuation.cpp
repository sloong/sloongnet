/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-22 20:38:27
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
    return CResult::Succeed();
}

TResult<TemplateInfo> Sloong::CConfiguation::GetTemplate(int id)
{
    try
    {
        auto template_item = m_oStorage->get<TemplateInfo>(id);
        return TResult<TemplateInfo>::Make_OK(template_item);
    }
    catch (system_error ex)
    {
        return TResult<TemplateInfo>::Make_Error(ex.what());
    }
}

bool Sloong::CConfiguation::CheckTemplateExist(int id)
{
    try
    {
        if( m_oStorage->count<TemplateInfo>(where(c(&TemplateInfo::id) == id)) > 0 )
            return true;
        else
            return false;
    }
    catch (system_error ex)
    {
        return false;
    }
}

vector<TemplateInfo> Sloong::CConfiguation::GetTemplateList()
{
    try
    {
        return m_oStorage->get_all<TemplateInfo>();
    }
    catch (system_error ex)
    {
        return vector<TemplateInfo>();
    }
}


CResult Sloong::CConfiguation::AddTemplate(const TemplateInfo& config, int* out_id)
{
    try
    {
        int id = m_oStorage->insert<TemplateInfo>(config);
        if( out_id) *out_id = id;
        return CResult::Succeed();
    }
    catch (system_error ex)
    {
        return TResult<int>::Make_Error(ex.what());
    }
}


CResult Sloong::CConfiguation::DeleteTemplate(int id)
{
    try
    {
        m_oStorage->remove<TemplateInfo>(id);
        return CResult::Succeed();
    }
    catch (system_error ex)
    {
        return TResult<int>::Make_Error(ex.what());
    }
}


CResult Sloong::CConfiguation::SetTemplate( int id, const TemplateInfo& config)
{
    try
    {
        m_oStorage->update(config);
    }
    catch (system_error ex)
    {
        return CResult::Make_Error(ex.what());
    }
    return CResult::Succeed();
}
