#include "mysqlex.h"

NResult Sloong::MySqlEx::Query(const string& sqlCmd, vector<string> *vRes)
{
	if (!m_bIsConnect)
	{
		return NResult::Make_Error("No connect to server. please call Connect function first.");
	}
	if ( m_pLog )
		m_pLog->Verbos(Helper::Format("[SQL]:[%s]", sqlCmd.c_str()));

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

	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[Rows:[%d]", nRes));
	return NResult::Make_OK(nRes);
}

NResult Sloong::MySqlEx::Query(const string& sqlCmd, string *strRes)
{
	if (!m_bIsConnect)
	{
		return NResult::Make_Error("No connect to server. please call Connect function first.");
	}
	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[%s]", sqlCmd.c_str()));

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
		*strRes = allLine;
	}

	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[Rows:[%d],Res:[%s]]", nRes, strRes->c_str()));
	return NResult::Make_OK(nRes);
}
