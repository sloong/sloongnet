#include "lua_ex.h"
#ifdef _WINDOWS
#include <io.h>
#endif // _WINDOWS
using namespace Sloong;

#include "utility.h"

typedef int (*LuaFunc)(lua_State *pLuaState);

#define LOCK_GUARD(m)             \
	{                             \
		lock_guard<mutex> lck(m); \
	}
#include "Lunar.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

Lunar<CLuaPacket>::RegType g_methods[] =
	{
		METHOD(CLuaPacket, clear),
		METHOD(CLuaPacket, setdata),
		METHOD(CLuaPacket, getdata),
		{0, 0}};

CLua::CLua()
{
	m_pScriptContext = luaL_newstate();
	luaL_openlibs(m_pScriptContext);
	Lunar<CLuaPacket>::Register(m_pScriptContext, g_methods);
}

CLua::~CLua()
{
	if (m_pScriptContext)
		lua_close(m_pScriptContext);
}

std::string CLua::findScript(const string &strFullName)
{
	string testFile(""), res("");
	for (auto item : m_listSearchRoute)
	{
		testFile = Helper::Replace(Helper::Replace(item, "${dir}", m_strScriptFolder), "${filename}", strFullName);
		if (CUtility::FileExist(testFile))
		{
			res = testFile;
			break;
		}
	}
	return res;
}

#ifndef lua_pushliteral
#define lua_pushliteral(L, s) lua_pushlstring(L, "" s, (sizeof(s) / sizeof(char)) - 1)
#endif
#define LEVELS1 12 // size of the first part of the stack
#define LEVELS2 10 // size of the second part

std::string CLua::GetCallStack(lua_State *l)
{
	int level = 0;
	int firstpart = 1;

	lua_Debug ar;
	if (!lua_isstring(l, 1))
		return "No stack info";

	lua_settop(l, 1);
	lua_pushliteral(l, "\r\n");
	lua_pushliteral(l, "Call Stack:\r\n");
	while (lua_getstack(l, level++, &ar))
	{
		if (level > LEVELS1 && firstpart)
		{
			if (!lua_getstack(l, level + LEVELS2, &ar))
			{
				level--;
			}
			else
			{
				lua_pushliteral(l, "                    ....\r\n");
				while (lua_getstack(l, level + LEVELS2, &ar))
				{
					level++;
				}
			}
			firstpart = 0;
			continue;
		}

		// lua_pushfstring(l, "%4d-   ", level - 1);
		// 'n': 填充 name 及 namewhat 域；
		// 'S': 填充 source， short_src，linedefined，lastlinedefined，以及 what 域；
		// 'l': 填充 currentline 域；
		// 'u': 填充 nups 域；
		// 'f': 把正在运行中指定级别处函数压入堆栈； （译注：一般用于获取函数调用中的信息， 级别是由 ar 中的私有部分来提供。 如果用于获取静态函数，那么就直接把指定函数重新压回堆栈， 但这样做通常无甚意义。）
		// 'L': 压一个 table 入栈，这个 table 中的整数索引用于描述函数中哪些行是有效行。 （有效行指有实际代码的行， 即你可以置入断点的行。 无效行包括空行和只有注释的行。）
		lua_getinfo(l, "Snl", &ar);
		lua_pushfstring(l, "{}:", ar.short_src);
		if (ar.currentline > 0)
			lua_pushfstring(l, "{}:", ar.currentline);

		switch (*ar.namewhat)
		{
		case 'g': // global
		case 'l': // local
		case 'f': // field
		case 'm': // method
			lua_pushfstring(l, " In function '{}'", ar.name);
			break;
		default:
		{
			if (*ar.what == 'm')
				lua_pushfstring(l, "in main chunk");
			else if (*ar.what == 'C') // c function
				lua_pushfstring(l, "{}", ar.short_src);
			else
				lua_pushfstring(l, " in function <{}:{}>", ar.short_src, ar.linedefined);
		}
		}
		lua_pushliteral(l, "\r\n");
		lua_concat(l, lua_gettop(l));
	}

	lua_concat(l, lua_gettop((l)));

	return lua_tostring(l, -1);
}

bool CLua::AddFunction(const string &pFunctionName, LuaFunctionType pFunction)
{
	LuaFunc pFunc = (LuaFunc)pFunction;
	lua_register(m_pScriptContext, pFunctionName.c_str(), pFunc);
	return true;
}

string CLua::GetString(lua_State *l, int nNum, const string &pDefault /* = "" */)
{
	auto str = luaL_optstring(l, nNum, pDefault.c_str());
	return str;
}

double CLua::GetDouble(lua_State *l, int nNum, double dDefault /* = -1.0f */)
{
	return luaL_optnumber(l, nNum, dDefault);
}

void CLua::PushString(lua_State *l, const string &strString)
{
	lua_pushstring(l, strString.c_str());
}

void CLua::PushDouble(lua_State *l, double dValue)
{
	lua_pushnumber(l, dValue);
}

void CLua::PushNil(lua_State *l)
{
	lua_pushnil(l);
}

LuaType CLua::CheckType(int index)
{
	int nType = lua_type(m_pScriptContext, index);
	return (LuaType)nType;
}

bool CLua::GetLuaFuncRef(int &nFunc, const string &strFuncName)
{
	if (strFuncName.length() == 0)
		return false;

	lua_getglobal(m_pScriptContext, strFuncName.c_str());
	if (!lua_isfunction(m_pScriptContext, -1))
		return false;

	nFunc = luaL_ref(m_pScriptContext, LUA_REGISTRYINDEX);
	return true;
}

bool CLua::PushFunction(int nFuncRef)
{
	lua_rawgeti(m_pScriptContext, LUA_REGISTRYINDEX, nFuncRef);
	if (!lua_isfunction(m_pScriptContext, -1))
	{
		return false;
	}
	return true;
}

static int GlobalErrorHandler(lua_State *L)
{
	CLua::PushString(L, CLua::GetCallStack(L));
	return 1;
}

#ifndef LUA_OK
#define LUA_OK 0
#endif

CResult CLua::RunScript(const string &strFileName)
{
	std::string strFullName = findScript(strFileName);
	if (strFullName.length() == 0)
	{
		return CResult::Make_Error(format("RunScript error. No find scritp file [{}] in folder [{}]" ,strFileName, m_strScriptFolder));
	}

	lua_pushcfunction(m_pScriptContext, GlobalErrorHandler);
	auto nErr = lua_gettop(m_pScriptContext);

	if (0 != luaL_loadfile(m_pScriptContext, strFullName.c_str()))
	{
		return HandlerError("Load Script", strFullName);
	}

	auto res = lua_pcall(m_pScriptContext, 0, LUA_MULTRET, nErr);
	lua_remove(m_pScriptContext, nErr);
	if (res != LUA_OK)
	{
		return HandlerError("Run Script", strFullName, res);
	}
	return CResult::Succeed;
}

CResult CLua::RunBuffer(LPCSTR pBuffer, size_t sz)
{
	lua_pushcfunction(m_pScriptContext, GlobalErrorHandler);
	auto nErr = lua_gettop(m_pScriptContext);

	if (0 != luaL_loadbuffer(m_pScriptContext, (LPCSTR)pBuffer, sz, NULL))
	{
		return HandlerError("Load Buffer", pBuffer);
	}

	auto res = lua_pcall(m_pScriptContext, 0, LUA_MULTRET, nErr);
	lua_remove(m_pScriptContext, nErr);
	if (res != LUA_OK)
	{
		return HandlerError("Run Buffer", pBuffer, res);
	}
	return CResult::Succeed;
}

CResult CLua::RunString(const string &strCommand)
{
	lua_pushcfunction(m_pScriptContext, GlobalErrorHandler);
	auto nErr = lua_gettop(m_pScriptContext);
	PushString(strCommand);
	auto res = lua_pcall(m_pScriptContext, 1, LUA_MULTRET, nErr);
	lua_remove(m_pScriptContext, nErr);
	if (res != LUA_OK)
	{
		return HandlerError("Run String", strCommand, res);
	}

	return CResult::Succeed;
}
CResult Sloong::CLua::RunFunction(const string &strFunctionName, CLuaPacket *pUserInfo, int funcid, const string &strRequest, const string &strExtend, string *extendDataUUID)
{
	int nTop = lua_gettop(m_pScriptContext);

	lua_pushcfunction(m_pScriptContext, GlobalErrorHandler);
	auto nErr = lua_gettop(m_pScriptContext);

	PushFunction(strFunctionName);
	// Push params
	PushInteger(funcid);
	PushPacket(pUserInfo);
	PushString(strRequest);
	PushString(strExtend);
	auto res = lua_pcall(m_pScriptContext, 4, LUA_MULTRET, nErr);
	lua_remove(m_pScriptContext, nErr);
	if (res != LUA_OK)
	{
		return HandlerError("Run String", strFunctionName, res);
	}

	int retNum = lua_gettop(m_pScriptContext);
	if (m_pLog)
	{
		for (int i = 1; i <= retNum; i++)
		{
			m_pLog->debug(format("Function returned no.[{}] params. type:[{}]", i, lua_typename(m_pScriptContext, lua_type(m_pScriptContext, i))));
		}
	}
		
	if (retNum < 2 || retNum > 3)
	{
		return CResult::Make_Error("Incorrect number of returned parameters");
	}

	if (!lua_isinteger(m_pScriptContext, 1))
	{
		return CResult::Make_Error(format("first returned type[{}] error. it's must be interger.", lua_typename(m_pScriptContext, lua_type(m_pScriptContext, 1))));
	}

	if (!lua_isstring(m_pScriptContext, 2))
	{
		return CResult::Make_Error(format("second returned type[{}] error. it's must be string.", lua_typename(m_pScriptContext, lua_type(m_pScriptContext, 2))));
	}

	if (retNum == 3 && !lua_isstring(m_pScriptContext, 3))
	{
		return CResult::Make_Error(format("third returned type[{}] error. it's must be string.", lua_typename(m_pScriptContext, lua_type(m_pScriptContext, 3))));
	}

	int nRes = (int)GetInteger(1, 0);
	auto strResponse = GetString(2, "");
	if (extendDataUUID)
		*extendDataUUID = GetString(3, "");

	lua_settop(m_pScriptContext, nTop);

	if (!ResultType_IsValid(nRes) || nRes == ResultType::Invalid)
	{
		return CResult::Make_Error("ResultType_IsValid " + Helper::ntos(nRes));
	}
	return CResult((ResultType)nRes, strResponse);
}

CResult Sloong::CLua::RunEventFunction(const string &strFunctionName, int eventid, const string &strRequest)
{
	int nTop = lua_gettop(m_pScriptContext);

	lua_pushcfunction(m_pScriptContext, GlobalErrorHandler);
	auto nErr = lua_gettop(m_pScriptContext);

	PushFunction(strFunctionName);
	// Push params
	PushInteger(eventid);
	PushString(strRequest);
	auto res = lua_pcall(m_pScriptContext, 2, LUA_MULTRET, nErr);
	lua_remove(m_pScriptContext, nErr);
	if (res != LUA_OK)
	{
		return HandlerError("Run String", strFunctionName, res);
	}

	lua_settop(m_pScriptContext, nTop);
	return CResult::Succeed;
}

int Sloong::CLua::GetInteger(lua_State *l, int nNum, int nDef /*= -1*/)
{
	return (int)luaL_optinteger(l, nNum, nDef);
}

bool Sloong::CLua::GetBoolen(lua_State *l, int nNum)
{
	return lua_toboolean(l, nNum);
}

void *Sloong::CLua::GetPointer(lua_State *l, int nNum)
{
	return (void *)lua_topointer(l, nNum);
}

void Sloong::CLua::PushInteger(lua_State *l, int nValue)
{
	lua_pushinteger(l, nValue);
}

void Sloong::CLua::PushBoolen(lua_State *l, bool b)
{
	lua_pushboolean(l, b);
}

void Sloong::CLua::PushPointer(lua_State *l, void *pPointer)
{
	lua_pushlightuserdata(l, pPointer);
}

void Sloong::CLua::PushTable(lua_State *l, const map<string, string> &mapValue)
{
	lua_newtable(l);
	for (auto &item : mapValue)
	{
		lua_pushstring(l, item.first.c_str());
		lua_pushstring(l, item.second.c_str());
		lua_rawset(l, -3);
	}
}

void Sloong::CLua::PushTable(lua_State *l, const list<string> &listValue)
{
	lua_newtable(l);
	int index = 1;
	for (auto &item : listValue)
	{
		lua_pushstring(l, item.c_str());
		lua_rawseti(l, -2, index);
		index++;
	}
}

void Sloong::CLua::Push2DTable(lua_State *l, const list<list<string>> &listValue)
{
	lua_newtable(l);
	int index = 1;
	for (auto &item : listValue)
	{
		PushTable(l, item);
		lua_rawseti(l, -2, index);
		index++;
	}
}

void Sloong::CLua::Push2DTable(lua_State *l, const list<map<string, string>> &listValue)
{
	lua_newtable(l);
	int index = 1;
	for (auto &item : listValue)
	{
		PushTable(l, item);
		lua_rawseti(l, -2, index);
		index++;
	}
}

void Sloong::CLua::Push2DTable(lua_State *l, const map<string, map<string, string>> &listValue)
{
	lua_newtable(l);
	int index = 1;
	for (auto &item : listValue)
	{
		lua_pushstring(l, item.first.c_str());
		PushTable(l, item.second);
		lua_rawseti(l, -3, index);
		index++;
	}
}

unique_ptr<map<std::string, std::string>> Sloong::CLua::GetTableParam(lua_State *l, int index)
{
	auto data = make_unique<map<string, string>>();
	lua_pushnil(l);
	// 现在的栈：-1 => nil; index => table
	if (index >= lua_gettop(l))
	{
		return nullptr;
	}

	while (lua_next(l, index))
	{
		// 现在的栈：-1 => value; -2 => key; index => table
		// 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
		lua_pushvalue(l, -2);
		// 现在的栈：-1 => key; -2 => value; -3 => key; index => table

		string key = std::string(lua_tostring(l, -1));
		string value = std::string(lua_tostring(l, -2));

		data->operator[](key) = value;
		// 弹出 value 和拷贝的 key，留下原始的 key 作为下一次 lua_next 的参数
		lua_pop(l, 2);
		// 现在的栈：-1 => key; index => table
	}
	// 现在的栈：index => table （最后 lua_next 返回 0 的时候它已经把上一次留下的 key 给弹出了）
	// 所以栈已经恢复到进入这个函数时的状态
	return data;
}

void Sloong::CLua::PushPacket(CLuaPacket *pData)
{
	if (pData)
		Lunar<CLuaPacket>::push(m_pScriptContext, pData, false);
	else
		PushNil(m_pScriptContext);
}