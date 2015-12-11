#include "globalfunction.h"
#include <stdlib.h>
#include <univ/log.h>
#include <univ/univ.h>
using namespace Sloong;
using namespace Sloong::Universal;
#include <boost/foreach.hpp>
#include "dbproc.h"

CGlobalFunction* CGlobalFunction::g_pThis = NULL;

LuaFunctionRegistr g_LuaFunc[] =
{
	{ "showLog", CGlobalFunction::Lua_showLog },
	{ "querySql", CGlobalFunction::Lua_querySql },
	{ "modifySql", CGlobalFunction::Lua_modifySql },
};

CGlobalFunction::CGlobalFunction()
{
	g_pThis = this;
}


CGlobalFunction::~CGlobalFunction()
{
}

void Sloong::CGlobalFunction::Initialize()
{
	m_pLua->SetErrorHandle(CGlobalFunction::HandleError);

	vector<LuaFunctionRegistr> funcList(g_LuaFunc, g_LuaFunc + sizeof(g_LuaFunc));
	m_pLua->AddFunctions(&funcList);
}

int Sloong::CGlobalFunction::Lua_querySql(lua_State* l)
{
	auto lua = g_pThis->m_pLua;
	vector<string> res;
	g_pThis->m_pDBProc->Query(lua->GetStringArgument(1), res);
	string allLine;
	BOOST_FOREACH(string item, res)
	{
		if ( allLine.empty())
		{
			allLine = item;
		}
		else
			allLine = allLine + "&" + item;
	}

	lua->PushString(allLine);
	return 1;
}

int Sloong::CGlobalFunction::Lua_modifySql(lua_State* l)
{
	auto lua = g_pThis->m_pLua;
	int nRes = g_pThis->m_pDBProc->Modify(lua->GetStringArgument(1));
	
	lua->PushString(CUniversal::ntos(nRes));
	return 1;
}

void CGlobalFunction::HandleError(string err)
{
	g_pThis->m_pLog->Log(err, ERR, -2);
}


int CGlobalFunction::Lua_showLog(lua_State* l)
{
	g_pThis->m_pLog->Log(g_pThis->m_pLua->GetStringArgument(1));
}


