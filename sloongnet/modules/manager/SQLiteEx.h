/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-16 17:15:07
 * @Description: file content
 */
#ifndef SLOONGNET_SQLITE_EX_H
#define SLOONGNET_SQLITE_EX_H

#include <sqlite3.h>
#include "core.h"
#include "DBResult.h"
namespace Sloong
{
    class CSQLiteEx
    {
    public:
        ~CSQLiteEx();

        bool Initialize( string dbPath );
        string GetErrorMessage();

        bool Query(const string& strSQL, EasyResult strResult, string& strError);
 
    protected:
        sqlite3* m_pDB = nullptr;
    };
}


#endif