#include <univ/univ.h>
#include "dbporcesser.h"
#include <mysql/mysql.h>
using namespace Sloong;
using namespace Sloong::Universal;
CDBPorcesser::CDBPorcesser()
{
    MYSQL mysql;
    MYSQL_RES* res;
    MYSQL_ROW row;

    mysql_init(&mysql);
    mysql_real_connect(&mysql,"localhost","root","sloong","sloong",0,NULL,0);
    mysql_query(&mysql,"select * from users");
    res = mysql_store_result(&mysql);
    while((row = mysql_fetch_row(res)))
    {
        CLog::showLog(INF,CUniversal::Format("%s - %s",row[0],row[1]));
    }

    mysql_free_result(res);
    mysql_close(&mysql);
}
