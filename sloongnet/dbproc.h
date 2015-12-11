#ifndef DBPORCESSER_H
#define DBPORCESSER_H

#include <mysql/mysql.h>
namespace Sloong
{
	class CDBProc
	{
	public:
		CDBProc();
		virtual ~CDBProc();

		bool Connect(string ip, string user, string passwd, string db, int port);
		int Modify(string sqlCmd);
		int Query(string sqlCmd, vector<string>& vRes);

		string GetError();

	private:
		MYSQL m_MySql;
	};
}

#endif // DBPORCESSER_H
