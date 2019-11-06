#ifndef SLOONGNET_SQLITE_EX_H
#define SLOONGNET_SQLITE_EX_H

#include <sqlite3.h>
#include "main.h"
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