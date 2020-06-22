/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-19 17:39:43
 * @Description: file content
 */

#include "configuation.h"

unique_ptr<CConfiguation> Sloong::CConfiguation::Instance = make_unique<CConfiguation>();

Sloong::CConfiguation::CConfiguation()
{
}

CResult Sloong::CConfiguation::Initialize(const string &dbPath)
{
    if (0 != access(dbPath.c_str(), R_OK))
    {
        return CResult::Make_Error("Configuation database file no exist or no access. file path: " + dbPath);
    }

    unique_lock<mutex> lck(m_oMutex);
    m_oStorage = make_unique<Storage>(InitStorage(dbPath));
    m_bInitialized = true;
    return CResult::Succeed();
}

TResult<TemplateInfo> Sloong::CConfiguation::GetTemplate(int id)
{
    unique_lock<mutex> lck(m_oMutex);
    try
    {
        auto template_item = m_oStorage->get<TemplateInfo>(id);
        return TResult<TemplateInfo>::Make_OK(template_item);
    }
    catch (system_error &ex)
    {
        return TResult<TemplateInfo>::Make_Error(ex.what());
    }
}

bool Sloong::CConfiguation::CheckTemplateExist(int id)
{
    unique_lock<mutex> lck(m_oMutex);
    try
    {
        if (m_oStorage->count<TemplateInfo>(where(c(&TemplateInfo::id) == id)) > 0)
            return true;
        else
            return false;
    }
    catch (system_error &ex)
    {
        return false;
    }
}

vector<TemplateInfo> Sloong::CConfiguation::GetTemplateList()
{
    unique_lock<mutex> lck(m_oMutex);
    try
    {
        return m_oStorage->get_all<TemplateInfo>();
    }
    catch (system_error &ex)
    {
        return vector<TemplateInfo>();
    }
}

CResult Sloong::CConfiguation::AddTemplate(const TemplateInfo &config, int *out_id)
{
    unique_lock<mutex> lck(m_oMutex);
    try
    {
        int id = m_oStorage->insert<TemplateInfo>(config);
        if (out_id)
            *out_id = id;
        return CResult::Succeed();
    }
    catch (system_error &ex)
    {
        return TResult<int>::Make_Error(ex.what());
    }
}

CResult Sloong::CConfiguation::DeleteTemplate(int id)
{
    unique_lock<mutex> lck(m_oMutex);
    try
    {
        m_oStorage->remove<TemplateInfo>(id);
        return CResult::Succeed();
    }
    catch (system_error &ex)
    {
        return TResult<int>::Make_Error(ex.what());
    }
}

CResult Sloong::CConfiguation::SetTemplate(int id, const TemplateInfo &config)
{
    unique_lock<mutex> lck(m_oMutex);
    try
    {
        m_oStorage->update(config);
    }
    catch (system_error &ex)
    {
        return CResult::Make_Error(ex.what());
    }
    return CResult::Succeed();
}
