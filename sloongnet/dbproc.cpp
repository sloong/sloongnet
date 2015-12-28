#include <univ/univ.h>
#include <univ/log.h>
#include "dbproc.h"
#include <mysql/mysql.h>
using namespace Sloong;
using namespace Sloong::Universal;
CDBProc::CDBProc()
{
    mysql_init(&m_MySql);
}

CDBProc::~CDBProc()
{
	mysql_close(&m_MySql);
}

bool Sloong::CDBProc::Connect(string ip, string user, string passwd, string db, int port)
{
	mysql_real_connect(&m_MySql, ip.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, NULL, 0);
    mysql_set_character_set(&m_MySql, "utf8");
}

int Sloong::CDBProc::Query(string sqlCmd, vector<string>& vRes)
{
	MYSQL_RES* res;
	MYSQL_ROW row;
	
	mysql_query(&m_MySql, sqlCmd.c_str());
    int nRes = mysql_affected_rows(&m_MySql);
    if( nRes == 0 )
        return 0;
    else if( nRes == -1 )
    {
        vRes.push_back(GetError());
        return -1;
    }
	res = mysql_store_result(&m_MySql);
    if( res == NULL )
        return 0;
    int nNums = mysql_num_fields(res);
	while ((row = mysql_fetch_row(res)))
	{
        string line(row[0]);
        CUniversal::Replace(line,"#","\#");
        for (int i = 1; i < nNums; i++)
		{
            string add;
            if( row[i] == NULL )
                add = "";
            else
                add = row[i];
            //string add = CUniversal::toansi(wadd);
            CUniversal::Replace(add,"#","\#");
            line = line + "#" + add;
		}

		vRes.push_back(line);
	}

	mysql_free_result(res);
	return vRes.size();
}

string Sloong::CDBProc::GetError()
{
	return mysql_error(&m_MySql);
}

int Sloong::CDBProc::Modify(string sqlCmd)
{
	mysql_query(&m_MySql, sqlCmd.c_str());
	return mysql_affected_rows(&m_MySql);
}