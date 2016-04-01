#ifndef DBPORCESSER_H
#define DBPORCESSER_H

#include <mysql/mysql.h>
namespace Sloong
{
	struct MySQLConnectInfo;
	class CDBProc
	{
	public:
		CDBProc();
		virtual ~CDBProc();

		void Connect(MySQLConnectInfo* info);
		int Modify(string sqlCmd);
		int Query(string sqlCmd, vector<string>& vRes);

		string GetError();

	private:
		MYSQL m_MySql;
	};
}

#endif // DBPORCESSER_H
