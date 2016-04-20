#include <univ/univ.h>
#include <univ/log.h>
#include "dbproc.h"
#include "structs.h"
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

void Sloong::CDBProc::Connect(MySQLConnectInfo* info)
{
	mysql_real_connect(&m_MySql, info->Address.c_str(), info->User.c_str(), 
		info->Password.c_str(), info->Database.c_str(), info->Port, NULL, 0);
    mysql_set_character_set(&m_MySql, "utf8");
}

int Sloong::CDBProc::Query(string sqlCmd, vector<string>* vRes)
{
	mysql_query(&m_MySql, sqlCmd.c_str());
	int nRes = mysql_affected_rows(&m_MySql);

	if ( vRes )
	{
		MYSQL_RES* res;
		MYSQL_ROW row;

		res = mysql_store_result(&m_MySql);

		if (res == NULL)
		{
			if (nRes == 0)
				return 0;
			else if (nRes == -1)
			{
				vRes->push_back(GetError());
				return -1;
			}
			else
				return 0;
		}

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

		mysql_free_result(res);
	}
	
	return nRes;
}

string Sloong::CDBProc::GetError()
{
	return mysql_error(&m_MySql);
}