#ifndef SLOONGNET_MODULE_DATACENTER_MYSQLEX_H
#define SLOONGNET_MODULE_DATACENTER_MYSQLEX_H

#include <mariadb/mysql.h>

#include "core.h"
#include "DBResult.h"
namespace Sloong
{
	typedef TResult<EasyResult> DResult;
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

		inline void SetLog(CLog *log) { m_pLog = log; }

		CResult Connect(const string &Address, int Port, const string &User, const string &Password, const string &Database);

		DResult Query(const string &);

		inline NResult Insert(const string &cmd){
			return RunModifySQLCmd(cmd);
		}
		inline NResult Delete(const string &cmd){
			return RunModifySQLCmd(cmd);
		}
		inline NResult Update(const string &cmd){
			return RunModifySQLCmd(cmd);
		}

		const string &GetError()
		{
			m_strErrorMsg = mysql_error(&m_MySql);
			return m_strErrorMsg;
		}
	protected:
		NResult RunModifySQLCmd( const string& );

	private:
		CLog *m_pLog = nullptr;
		MYSQL m_MySql;
		string m_strErrorMsg;
		bool m_bIsConnect = false;
		static char constexpr ROW_SEP = 0x09;
		static char constexpr LINE_SEP = 0x0A;
	};
} // namespace Sloong

#endif // SLOONGNET_MODULE_DATACENTER_MYSQLEX_H
