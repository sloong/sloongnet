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
    sqlite3_close(m_pDB);
}

bool Sloong::CSQLiteEx::Initialize(string dbPath)
{
    auto rc = sqlite3_open_v2(dbPath.c_str(), &m_pDB, 
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, NULL);
    if (rc == SQLITE_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Sloong::CSQLiteEx::Query(const string &strSQL, EasyResult dbResult, string &strError)
{
    sqlite3_stmt *stmt = NULL;
    auto res = sqlite3_prepare_v2(m_pDB, strSQL.c_str(), -1, &stmt, NULL);
    if (res != SQLITE_OK)
    {
        strError = string(sqlite3_errmsg(m_pDB));
        return false;
    }

    int nColumn = sqlite3_column_count(stmt);

    for ( int i=0 ; i< nColumn; i++ )
        dbResult->AppendColumn( sqlite3_column_name(stmt,i));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int index = dbResult->AppendLine();
        for( int i = 0; i< nColumn; i++)
        {
            const char* data = (const char*)sqlite3_column_text(stmt, i);
            auto name = sqlite3_column_name(stmt,i);
            data = data == NULL ? "" : data;
            dbResult->SetItemData( index, name, data);
        }
    }

    sqlite3_finalize(stmt);
    return true;
}


string Sloong::CSQLiteEx::GetErrorMessage()
{
    return sqlite3_errmsg(m_pDB);
}