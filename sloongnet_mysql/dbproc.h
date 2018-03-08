#ifndef DBPORCESSER_H
#define DBPORCESSER_H

#include <mysql/mysql.h>
#include "sloongnet_mysql.h"
namespace Sloong
{
	struct MySQLConnectInfo
	{
		bool Enable;
		
	};

	class CDBProc
	{
	public:
		CDBProc();
		virtual ~CDBProc();
		static CDBProc* TryGet(lua_State* l);

		void Connect(string Address,int Port,string User,string Password,string Database);
		int Query(string sqlCmd, vector<string>* vRes);

		string GetError();

	private:
		MYSQL m_MySql;
		bool m_bIsConnect = false;
	};
}

#endif // DBPORCESSER_H
