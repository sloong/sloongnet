#include <univ/univ.h>
#include <univ/log.h>
#include "dbproc.h"
#include "structs.h"
#include <univ/exception.h>
using namespace Sloong;
using namespace Sloong::Universal;
CDBProc::CDBProc()
{
    mysql_init(&m_MySql);
	// enable re connect;
	char value = 1;
	mysql_options(&m_MySql, MYSQL_OPT_RECONNECT, &value);
}

CDBProc::~CDBProc()
{
	mysql_close(&m_MySql);
}

void Sloong::CDBProc::Connect(MySQLConnectInfo* info)
{
	if(!mysql_real_connect(&m_MySql, info->Address.c_str(), info->User.c_str(), 
		info->Password.c_str(), info->Database.c_str(), info->Port, NULL, 0))
	{
		throw normal_except(GetError());
	}
    mysql_set_character_set(&m_MySql, "utf8");
}

int Sloong::CDBProc::Query(string sqlCmd, vector<string>* vRes)
{
	mysql_ping(&m_MySql);
	if(0 != mysql_query(&m_MySql, sqlCmd.c_str()))
	{
		throw normal_except(GetError());
	}

	int nRes = mysql_affected_rows(&m_MySql);
	MYSQL_RES* res;
	res = mysql_store_result(&m_MySql);
	if (res == NULL)
	{
			return nRes;
	}
	if ( vRes )
	{
		MYSQL_ROW row;

		int nNums = mysql_num_fields(res);
		char tab = 0x09;
		while ((row = mysql_fetch_row(res)))
		{
			string line(row[0]);
			for (int i = 1; i < nNums; i++)
			{
				string add;
				if (row[i] == NULL)
					add = "";
				else
					add = row[i];
				line = line + tab + add;
			}

			vRes->push_back(line);
		}
		if( nRes == -1 )
			nRes = vRes->size();
		mysql_free_result(res);
	}
	
	return nRes;
}

string Sloong::CDBProc::GetError()
{
	return mysql_error(&m_MySql);
}
