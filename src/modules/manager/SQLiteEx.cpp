/*
 * @Author: WCB
 * @Date: 2019-11-05 08:59:19
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 17:04:28
 * @Description: file content
 */

#include "SQLiteEx.h"

Sloong::CSQLiteEx::~CSQLiteEx()
{
    if (m_pDB != nullptr)
        sqlite3_close(m_pDB);
}

CResult Sloong::CSQLiteEx::Initialize(const string &dbPath, const string &createSQL)
{
    auto rc = sqlite3_open_v2(dbPath.c_str(), &m_pDB,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
    if (rc == SQLITE_OK)
    {
        return CResult::Succeed;
    }
    else
    {
        rc = sqlite3_open_v2(dbPath.c_str(), &m_pDB,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
        if (rc == SQLITE_OK)
        {
            if( sqlite3_exec(m_pDB, createSQL.c_str(), nullptr, 0, nullptr) != SQLITE_OK)
            {
                 return CResult::Make_Error("Create DBFile succeed, but prepare init db fialed: " + GetErrorMessage());
            }
            return CResult::Succeed;
            /*sqlite3_stmt *stmt = NULL;
            if (sqlite3_prepare_v2(m_pDB, createSQL.c_str(), -1, &stmt, NULL) != SQLITE_OK)
            {
                if (stmt) sqlite3_finalize(stmt);
                sqlite3_close(m_pDB);
                return CResult::Make_Error("Create DBFile succeed, but prepare init db fialed: " + GetErrorMessage());
            }
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                if (stmt) sqlite3_finalize(stmt);
                sqlite3_close(m_pDB);
                return CResult::Make_Error("Create DBFile succeed, but init db fialed: " + GetErrorMessage());
            }
            sqlite3_finalize(stmt);
            return CResult::Succeed;*/
        }
        else
        {
            return CResult::Make_Error("Create DBfile error:" + GetErrorMessage());
        }
    }
}

bool Sloong::CSQLiteEx::Query(const string &strSQL, EasyResult dbResult, string &strError)
{
    sqlite3_stmt *stmt = NULL;
    auto res = sqlite3_prepare_v2(m_pDB, strSQL.c_str(), -1, &stmt, NULL);
    if (res != SQLITE_OK)
    {
        if (stmt) sqlite3_finalize(stmt);
        strError = string(sqlite3_errmsg(m_pDB));
        return false;
    }

    int nColumn = sqlite3_column_count(stmt);

    for (int i = 0; i < nColumn; i++)
        dbResult->AppendColumn(sqlite3_column_name(stmt, i));

    //对于DDL和DML语句而言，sqlite3_step执行正确的返回值
    //只有SQLITE_DONE，对于SELECT查询而言，如果有数据返回SQLITE_ROW，当到达结果集末尾时则返回
    //SQLITE_DONE。
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int index = dbResult->AppendLine();
        for (int i = 0; i < nColumn; i++)
        {
            const char *data = (const char *)sqlite3_column_text(stmt, i);
            auto name = sqlite3_column_name(stmt, i);
            data = data == NULL ? "" : data;
            dbResult->SetItemData(index, name, data);
        }
    }

    sqlite3_finalize(stmt);
    return true;
}

string Sloong::CSQLiteEx::GetErrorMessage()
{
    return sqlite3_errmsg(m_pDB);
}