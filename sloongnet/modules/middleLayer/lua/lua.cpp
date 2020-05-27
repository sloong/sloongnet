#include "stdafx.h"
#include "lua.h"
#include "univ.h"
#ifdef _WINDOWS
#include <io.h>
#endif // _WINDOWS
using namespace Sloong::Universal;

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
		testFile = Helper::Replace(Helper::Replace(item, "%pathdir%", m_strScriptFolder), "%filename%", strFullName);
		if (0 == ACCESS(testFile.c_str(), ACC_R))
		{
			res = testFile;
			break;
		}
	}
	return res;
}

bool CLua::RunScript(const string &strFileName)
{
	std::string strFullName = findScript(strFileName);

	if (0 != luaL_loadfile(m_pScriptContext, strFullName.c_str()))
	{
		HandlerError("Load Script", strFullName);
		return false;
	}

	if (0 != lua_pcall(m_pScriptContext, 0, LUA_MULTRET, 0))
	{
		HandlerError("Run Script", strFullName);
		return false;
	}
	return true;
}

bool CLua::RunBuffer(LPCSTR pBuffer, size_t sz)
{
	if (0 != luaL_loadbuffer(m_pScriptContext, (LPCSTR)pBuffer, sz, NULL))
	{
		HandlerError("Load Buffer", pBuffer);
		return false;
	}

	if (0 != lua_pcall(m_pScriptContext, 0, LUA_MULTRET, 0))
	{
		HandlerError("Run Buffer", pBuffer);
		return false;
	}
	return true;
}

bool CLua::RunString(const string &strCommand)
{
	if (0 != luaL_loadstring(m_pScriptContext, strCommand.c_str()))
	{
		HandlerError("String Load", strCommand);
		return false;
	}

	if (0 != lua_pcall(m_pScriptContext, 0, LUA_MULTRET, 0))
	{
		HandlerError("Run String", strCommand);
		return false;
	}

	return true;
}
#ifndef lua_pushliteral
#define lua_pushliteral(L, s) lua_pushlstring(L, "" s, (sizeof(s) / sizeof(char)) - 1)
#endif
#define LEVELS1 12 // size of the first part of the stack
#define LEVELS2 10 // size of the second part

std::string CLua::GetErrorString()
{
	int level = 0;
	int firstpart = 1;

	lua_Debug ar;
	if (!lua_isstring(m_pScriptContext, 1))
		return "";

	lua_settop(m_pScriptContext, 1);
	lua_pushliteral(m_pScriptContext, "\r\n");
	lua_pushliteral(m_pScriptContext, "Call Stack:\r\n");
	while (lua_getstack(m_pScriptContext, level++, &ar))
	{
		char buff[10] = {0};
		if (level > LEVELS1 && firstpart)
		{
			if (!lua_getstack(m_pScriptContext, level + LEVELS2, &ar))
			{
				level--;
			}
			else
			{
				lua_pushliteral(m_pScriptContext, "                    ....\r\n");
				while (lua_getstack(m_pScriptContext, level + LEVELS2, &ar))
				{
					level++;
				}
			}
			firstpart = 0;
			continue;
		}

		sprintf(buff, "%4d-   ", level - 1);
		lua_pushstring(m_pScriptContext, buff);
		lua_getinfo(m_pScriptContext, "Snl", &ar);
		lua_pushfstring(m_pScriptContext, "%s:", ar.short_src);
		if (ar.currentline > 0)
			lua_pushfstring(m_pScriptContext, "%d:", ar.currentline);

		switch (*ar.namewhat)
		{
		case 'g': // global
		case 'l': // local
		case 'f': // field
		case 'm': // method
			lua_pushfstring(m_pScriptContext, " In function '%s'", ar.name);
			break;
		default:
		{
			if (*ar.what == 'm')
				lua_pushfstring(m_pScriptContext, "in main chunk");
			else if (*ar.what == 'C') // c function
				lua_pushfstring(m_pScriptContext, "%s", ar.short_src);
			else
				lua_pushfstring(m_pScriptContext, " in function <%s:%d>", ar.short_src, ar.linedefined);
		}
		}
		lua_pushliteral(m_pScriptContext, "\r\n");
		lua_concat(m_pScriptContext, lua_gettop(m_pScriptContext));
	}

	lua_concat(m_pScriptContext, lua_gettop((m_pScriptContext)));

	return lua_tostring(m_pScriptContext, -1);
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

bool CLua::RunFunction(string strFunctionName, CLuaPacket *pUserInfo, CLuaPacket *pRequest, CLuaPacket *pResponse)
{
	int nTop = lua_gettop(m_pScriptContext);
	int nErr = 0;

	PushFunction("OnError");
	nErr = lua_gettop(m_pScriptContext);

	PushFunction(strFunctionName);

	PushPacket(pUserInfo);
	PushPacket(pRequest);
	PushPacket(pResponse);

	if (0 != lua_pcall(m_pScriptContext, 3, LUA_MULTRET, nErr))
	{
		GetErrorString();
		return false;
	}
	lua_settop(m_pScriptContext, nTop);
	return true;
}

int Sloong::Universal::CLua::RunFunction(string strFunctionName, CLuaPacket *pUserInfo, string &strRequest, string &strResponse)
{
	int nTop = lua_gettop(m_pScriptContext);
	int nErr = 0;

	PushFunction("OnError");
	nErr = lua_gettop(m_pScriptContext);

	PushFunction(strFunctionName);

	PushPacket(pUserInfo);
	PushString(strRequest);

	if (0 != lua_pcall(m_pScriptContext, 2, LUA_MULTRET, nErr))
	{
		strResponse = GetErrorString();
		return -2;
	}
	strResponse = lua_tostring(m_pScriptContext, -2);
	int nRes = (int)lua_tonumber(m_pScriptContext, -1);

	lua_settop(m_pScriptContext, nTop);
	return nRes;
}

void Sloong::Universal::CLua::RunFunction(string strFunctionName, CLuaPacket *pUserInfo)
{
	int nTop = lua_gettop(m_pScriptContext);
	int nErr = 0;

	PushFunction("OnError");
	nErr = lua_gettop(m_pScriptContext);

	PushFunction(strFunctionName);

	PushPacket(pUserInfo);

	if (0 != lua_pcall(m_pScriptContext, 1, LUA_MULTRET, nErr))
		HandlerError("RunFunction", "lua_pcall function failed");
	else
		lua_settop(m_pScriptContext, nTop);
}

int Sloong::Universal::CLua::GetInteger(lua_State *l, int nNum, int nDef /*= -1*/)
{
	return (int)luaL_optinteger(l, nNum, nDef);
}

bool Sloong::Universal::CLua::GetBoolen(lua_State *l, int nNum)
{
	return lua_toboolean(l, nNum);
}

void *Sloong::Universal::CLua::GetPointer(lua_State *l, int nNum)
{
	return (void *)lua_topointer(l, nNum);
}

void Sloong::Universal::CLua::PushInteger(lua_State *l, int nValue)
{
	lua_pushinteger(l, nValue);
}

void Sloong::Universal::CLua::PushBoolen(lua_State *l, bool b)
{
	lua_pushboolean(l, b);
}

void Sloong::Universal::CLua::PushPointer(lua_State *l, void *pPointer)
{
	lua_pushlightuserdata(l, pPointer);
}

unique_ptr<map<std::string, std::string>> Sloong::Universal::CLua::GetTableParam(lua_State *l, int index)
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

void Sloong::Universal::CLua::PushPacket(CLuaPacket *pData)
{
	if (pData)
		Lunar<CLuaPacket>::push(m_pScriptContext, pData, false);
	else
		PushNil(m_pScriptContext);
}