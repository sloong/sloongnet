#ifndef SLOONGNET_MODULE_DATACENTER_MYSQLEX_H
#define SLOONGNET_MODULE_DATACENTER_MYSQLEX_H

#include <mariadb/mysql.h>

#include "core.h"
namespace Sloong
{
	typedef TResult<int> NResult;
	class MySqlEx
	{
	public:
		MySqlEx()
		{
			mysql_init(&m_MySql);
			// enable re connect;
			char value = 1;
			mysql_options(&m_MySql, MYSQL_OPT_RECONNECT, &value);
		}
		virtual ~MySqlEx()
		{
			mysql_close(&m_MySql);
		}

		CResult Connect(const string &Address, int Port, const string &User, const string &Password, const string &Database)
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
		
		NResult Query(const string &, vector<string> *);
		NResult Query(const string &, string* );

		const string &GetError()
		{
			m_strErrorMsg = mysql_error(&m_MySql);
			return m_strErrorMsg;
		}

	private:
		MYSQL m_MySql;
		string m_strErrorMsg;
		bool m_bIsConnect = false;
		static char constexpr ROW_SEP = 0x09;
		static char constexpr LINE_SEP = 0x0A;
	};
} // namespace Sloong

#endif // SLOONGNET_MODULE_DATACENTER_MYSQLEX_H
