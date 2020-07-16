#include "mysqlex.h"

CResult Sloong::MySqlEx::Connect(const string &Address, int Port, const string &User, const string &Password, const string &Database)
{
	if (!mysql_real_connect(&m_MySql, Address.c_str(), User.c_str(),
							Password.c_str(), Database.c_str(), Port, NULL, 0))
	{
		return CResult::Make_Error(GetError());
	}
	mysql_set_character_set(&m_MySql, "utf8");
	m_bIsConnect = true;
	return CResult::Succeed();
}

DResult Sloong::MySqlEx::Query(const string &sqlCmd)
{
	if (!m_bIsConnect)
	{
		return DResult::Make_Error("No connect to server. please call Connect function first.");
	}
	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[%s]", sqlCmd.c_str()));

	mysql_ping(&m_MySql);
	if (0 != mysql_query(&m_MySql, sqlCmd.c_str()))
	{
		return DResult::Make_Error(GetError());
	}

	auto res = mysql_store_result(&m_MySql);
	auto dbresult = make_shared<DBResult>();
	if (res == nullptr && mysql_errno(&m_MySql) != 0 )
	{
		return DResult::Make_Error(GetError());
	}

	if( mysql_num_rows(res) > 0)
	{
		int nNums = mysql_num_fields(res);
		for( int i = 1; i < nNums; i++ )
		{
			dbresult->AppendColumn("");
		}
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(res)))
		{
			int index = dbresult->AppendLine();
			for (int i = 1; i < nNums; i++)
			{
				dbresult->SetItemData(index,i,row[i]);
			}
		}
	}

	if( res != nullptr)
		mysql_free_result(res);
	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[Rows:[%d]", dbresult->GetLinesNum()));
	return DResult::Make_OK(dbresult);
}

NResult Sloong::MySqlEx::RunModifySQLCmd(const string &sqlCmd)
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

	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[Insert Line:[%d]", nRes));
	return NResult::Make_OK(nRes);
}
