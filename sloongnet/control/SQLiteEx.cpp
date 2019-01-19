
#include "main.h"
#include "SQLiteEx.h"

string Sloong::CSQLiteEx::g_strResult = "";

Sloong::CSQLiteEx::~CSQLiteEx()
{
    sqlite3_close(m_pDB);
}

bool Sloong::CSQLiteEx::Initialize(string dbPath)
{
    auto rc = sqlite3_open(dbPath.c_str(), &m_pDB);
    if (rc == SQLITE_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Sloong::CSQLiteEx::Query(const string &strSQL, string &strResult, string &strError)
{
    char *zErrMsg = 0;
    auto res = sqlite3_exec(m_pDB, strSQL.c_str(), &QueryCallBack, (void *)&m_oSync, &zErrMsg);
    if (res != SQLITE_OK)
    {
        strError = string(zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }

    m_oSync.wait();
    strResult = g_strResult;
    return true;
}

int Sloong::CSQLiteEx::QueryCallBack(void *param, int argc, char **argv, char **azColName)
{
    CEasySync *pSync = static_cast<CEasySync *>(param);
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    pSync->notify_one();
    return 0;
}

string Sloong::CSQLiteEx::QueryEx(string table_name, string domain, string key)
{
    string res;
    string error;
    string sql = CUniversal::Format("select value from %s where domain=`%s` and key=`%s`",
                                    table_name.c_str(), domain.c_str(), key.c_str());

    if (Query(sql, res, error))
    {
        return res;
    }
    else
    {
        throw normal_except(error);
    }
}

string Sloong::CSQLiteEx::GetErrorMessage()
{
    return sqlite3_errmsg(m_pDB);
}