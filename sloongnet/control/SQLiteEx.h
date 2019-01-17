#ifndef SLOONGNET_SQLITE_EX_H
#define SLOONGNET_SQLITE_EX_H

#include <sqlite3.h>

namespace Sloong
{
    class CSQLiteEx
    {
    public:
        ~CSQLiteEx();

        bool Initialize( string dbPath );
        string GetErrorMessage();

        bool Query(string strSQL, string& strResult, string& strError);

        string QueryEx( string table_name, string domain, string key );
        
    protected:
        static int QueryCallBack(void *NotUsed, int argc, char **argv, char **azColName); 
        static string g_strResult;  

    protected:
        sqlite3* m_pDB;
        EasySync m_oSync;
    };
}


#endif