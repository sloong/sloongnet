#include "mysqlex.h"

NResult Sloong::MySqlEx::Query(string sqlCmd, vector<string> *vRes)
{
	if (!m_bIsConnect)
	{
		return NResult::Make_Error("No connect to server. please call Connect function first.");
	}
	if (g_bShowSQLCmd && g_pLog)
		g_pLog->Verbos(CUniversal::Format("[SQL]:[%s]", cmd));

	mysql_ping(&m_MySql);
	if (0 != mysql_query(&m_MySql, sqlCmd.c_str()))
	{
		return NResult::Make_Error(GetError());
	}

	int nRes = mysql_affected_rows(&m_MySql);
	MYSQL_RES *res;
	res = mysql_store_result(&m_MySql);
	if (res == NULL)
	{
		return NResult::Make_OK(nRes);
	}
	if (vRes)
	{
		MYSQL_ROW row;

		int nNums = mysql_num_fields(res);
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
				line = line + ROW_SEP + add;
			}

			vRes->push_back(line);
		}
		if (nRes == -1)
			nRes = vRes->size();
		mysql_free_result(res);
	}

	if (g_bShowSQLResult && g_pLog)
		g_pLog->Verbos(CUniversal::Format("[SQL]:[Rows:[%d]", nRes));
	return nRes;
}

NResult Sloong::MySqlEx::Query(string sqlCmd, string *strRes)
{
	if (!m_bIsConnect)
	{
		return NResult::Make_Error("No connect to server. please call Connect function first.");
	}
	if (g_bShowSQLCmd && g_pLog)
		g_pLog->Verbos(CUniversal::Format("[SQL]:[%s]", cmd));

	mysql_ping(&m_MySql);
	if (0 != mysql_query(&m_MySql, sqlCmd.c_str()))
	{
		return NResult::Make_Error(GetError());
	}

	int nRes = mysql_affected_rows(&m_MySql);
	MYSQL_RES *res;
	res = mysql_store_result(&m_MySql);
	if (res == NULL)
	{
		return NResult::Make_OK(nRes);
	}

	if (strRes)
	{
		MYSQL_ROW row;

		string allLine;
		int nNums = mysql_num_fields(res);
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
				line = line + ROW_SEP + add;
			}
			allLine = allLine + LINE_SEP + line;
		}
	}

	if (g_bShowSQLResult && g_pLog)
		g_pLog->Verbos(CUniversal::Format("[SQL]:[Rows:[%d],Res:[%s]]", nRes, allLine.c_str()));
	return nRes;
}
