#include "sloongnet-db.h"
#include <univ/lua.h>
#include "dbproc.h"
#include <boost/foreach.hpp>

using namespace Sloong;
using namespace Sloong::Universal;

bool g_bShowSQLCmd = false;
bool g_bShowSQLResult = false;
CLog* g_pLog = nullptr;

int Lua_querySql(lua_State* l)
{
	CDBProc* s = CDBProc::TryGet(l);

	string cmd = CLua::GetString(l, 2);
	if (g_bShowSQLCmd && g_pLog)
		g_pLog->Verbos(CUniversal::Format("[SQL]:[%s]", cmd));
	vector<string> res;
	int nRes = 0;
	try
	{
		nRes = s->Query(cmd, &res);
	}
	catch (normal_except& e)
	{
		string err = CUniversal::Format("SQL Query error:[%s]", e.what());
		if( g_pLog )
			g_pLog->Warn(CUniversal::Format("[SQL]:[%s]", err));
		CLua::PushInteger(l, -1);
		CLua::PushString(l, err);
		return 2;
	}

	string allLine;
	char line = 0x0A;
	BOOST_FOREACH(string item, res)
	{
		string add = item;
		if (allLine.empty())
		{
			allLine = add;
		}
		else
			allLine = allLine + line + add;
	}
	if (g_bShowSQLResult && g_pLog )
		g_pLog->Verbos(CUniversal::Format("[SQL]:[Rows:[%d],Res:[%s]]", nRes, allLine.c_str()));

	CLua::PushInteger(l, nRes);
	CLua::PushString(l, allLine);
	return 2;
}

int Lua_getSqlError(lua_State *l)
{
	CDBProc* s = CDBProc::TryGet(l);
	CLua::PushString(l, s->GetError());
	return 1;
}


// 连接sql服务器
// lua中需要使用英文句号"."进行调用
// 参数分别为(地址，端口，用户，密码，表名称)
// 返回2个参数(连接结果(bool),错误信息(string))
int Lua_connect(lua_State *l)
{
	CDBProc* s = CDBProc::TryGet(l);
	if (s)
	{
		string addr = CLua::GetString(l, 2);
		int port = CLua::GetInteger(l, 3);
		string user= CLua::GetString(l, 4);
		string pwd = CLua::GetString(l, 5);
		string db_name = CLua::GetString(l, 6);
		try
		{
			s->Connect(addr, port, user, pwd, db_name);
		}
		catch (normal_except e)
		{
			lua_pushboolean(l, false);
			lua_pushstring(l,e.what());
			return 2;
		}
		lua_pushboolean(l, true);
		lua_pushstring(l, "");
		return 2;
	}
	lua_pushboolean(l, false);
	lua_pushstring(l, "trans object type error, please use the pointer to call this function.");
	return 2;
}


static int l_newObject(lua_State* l)
{
	CDBProc **p = (CDBProc**)lua_newuserdata(l, sizeof(CDBProc *));
	*p = new CDBProc;
	luaL_getmetatable(l, SLOONGNET_MYSQL_METHOD_NAME);
	lua_setmetatable(l, -2);//设定 userdata的 metatable
	return 1;
}

static int lua_auto_gc(lua_State* L)
{
	CDBProc **s = (CDBProc**)luaL_checkudata(L, 1, SLOONGNET_MYSQL_METHOD_NAME);
	if (s)
	{
		delete *s;
	}
	return 0;
}


static int lua_toString(lua_State* L)
{
	CDBProc* s = CDBProc::TryGet(L);
	if (s)
		lua_pushfstring(L, "Sloongnet extend model for mysql object");
	return 1;
}


static int Lua_setLog(lua_State* L)
{
	g_pLog = static_cast<CLog*>(CLua::GetPointer(L, 1));
	assert(g_pLog);
	g_bShowSQLCmd = CLua::GetBoolen(L, 2);
	g_bShowSQLResult = CLua::GetBoolen(L, 3);
	return 0;
}

static const struct luaL_Reg sloongnet_mysql_Function[] =  //导出到库中
{
	{ "new",l_newObject },
	{ "SetLog", Lua_setLog },
	{ NULL,NULL }
};

static const struct luaL_Reg sloongnet_mysql_Methods[] =  //导出到元表中
{
	{ "Connect",Lua_connect },
	{ "Query",Lua_querySql },
	{ "GetError", Lua_getSqlError },
	{ "__gc", lua_auto_gc },
	{ "__tostring", lua_toString },
	{ NULL,NULL }
};



extern "C"
int luaopen_sloongnet_mysql(lua_State* L)
{
	luaL_newmetatable(L, SLOONGNET_MYSQL_METHOD_NAME);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, sloongnet_mysql_Methods);
	luaL_register(L, "sloongnet_mysql", sloongnet_mysql_Function);
	return 1;
}
