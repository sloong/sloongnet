/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2015-12-11 15:05:40
 * @LastEditTime: 2020-08-10 16:22:41
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/datacenter/mysqlex.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#include "mysqlex.h"

unique_ptr<MySqlEx> Sloong::MySqlEx::Duplicate()
{
	auto d = make_unique<MySqlEx>();
	d->Connect(this->Address, this->Port, this->User, this->Password, this->Database);
	d->SetLog(this->m_pLog);
	return d;
}


CResult Sloong::MySqlEx::Connect(const string &Address, int Port, const string &User, const string &Password, const string &Database)
{
	this->Address = Address;
	this->Port = Port;
	this->User = User;
	this->Password = Password;
	this->Database = Database;
	if (!mysql_real_connect(&m_MySql, Address.c_str(), User.c_str(),
							Password.c_str(), Database.c_str(), Port, NULL, 0))
	{
		return CResult::Make_Error(GetError());
	}
	mysql_set_character_set(&m_MySql, "utf8");
	m_bIsConnect = true;
	return CResult::Succeed;
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
	if (res == nullptr  )
	{
		if( mysql_errno(&m_MySql) != 0 )
			return DResult::Make_Error(GetError());
		else
			return DResult::Make_OKResult(dbresult);;
	}

	if( mysql_num_rows(res) > 0)
	{
		int nNums = mysql_num_fields(res);
		for( int i = 0; i < nNums; i++ )
		{
			dbresult->AppendColumn("");
		}
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(res)))
		{
			int index = dbresult->AppendLine();
			for (int i = 0; i < nNums; i++)
			{
				dbresult->SetItemData(index,i,row[i]);
			}
		}
	}

	mysql_free_result(res);
	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[Rows:[%d]", dbresult->GetLinesNum()));
	return DResult::Make_OKResult(dbresult);
}

U64Result Sloong::MySqlEx::RunModifySQLCmd(const string &sqlCmd)
{
	if (!m_bIsConnect)
	{
		return U64Result::Make_Error("No connect to server. please call Connect function first.");
	}
	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[%s]", sqlCmd.c_str()));

	mysql_ping(&m_MySql);
	if (0 != mysql_query(&m_MySql, sqlCmd.c_str()))
	{
		return U64Result::Make_Error(GetError());
	}

	int nRes = mysql_affected_rows(&m_MySql);

	if (m_pLog)
		m_pLog->Verbos(Helper::Format("[SQL]:[Insert Line:[%d]", nRes));
	return U64Result::Make_OKResult(nRes);
}
