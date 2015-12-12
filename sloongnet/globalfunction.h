#ifndef CGLOBALFUNCTION_H
#define CGLOBALFUNCTION_H

#include <univ/lua.h>
namespace Sloong
{
	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;
	class CUtility;
	class CDBProc;
	class CGlobalFunction
	{
	public:
        CGlobalFunction( );
		~CGlobalFunction();

        void Initialize(CLog* plog ,CLua* pLua);
	protected:
        CUtility * m_pUtility;
		CDBProc* m_pDBProc;
		CLua*	m_pLua;
		CLog*	m_pLog;
	public:
		static void HandleError(string err);

		static int Lua_showLog(lua_State* l);
		static int Lua_querySql(lua_State* l);
		static int Lua_modifySql(lua_State* l);
        static int Lua_getSqlError(lua_State* l);
		
	public:
		static CGlobalFunction* g_pThis;
	};



}
#endif // !CGLOBALFUNCTION_H



