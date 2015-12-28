#include "globalfunction.h"
#include <stdlib.h>
#include <univ/log.h>
#include <univ/univ.h>
using namespace Sloong;
using namespace Sloong::Universal;
#include <boost/foreach.hpp>
#include "dbproc.h"
#include "utility.h"
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

CGlobalFunction* CGlobalFunction::g_pThis = NULL;

LuaFunctionRegistr g_LuaFunc[] =
{
	{ "showLog", CGlobalFunction::Lua_showLog },
	{ "querySql", CGlobalFunction::Lua_querySql },
	{ "modifySql", CGlobalFunction::Lua_modifySql },
    { "getSqlError", CGlobalFunction::Lua_getSqlError },
};

CGlobalFunction::CGlobalFunction()
{
    m_pUtility = new CUtility();
    m_pDBProc = new CDBProc();
	g_pThis = this;
}


CGlobalFunction::~CGlobalFunction()
{
}

void Sloong::CGlobalFunction::Initialize( CLog* plog,CLua* pLua)
{
    m_pLog = plog;
    m_pLua = pLua;
	m_pLua->SetErrorHandle(CGlobalFunction::HandleError);

    vector<LuaFunctionRegistr> funcList(g_LuaFunc, g_LuaFunc + ARRAYSIZE(g_LuaFunc));
	m_pLua->AddFunctions(&funcList);

    // connect to db
    m_pDBProc->Connect("localhost","root","sloong","sloong",0);
}

int Sloong::CGlobalFunction::Lua_querySql(lua_State* l)
{
	auto lua = g_pThis->m_pLua;
	vector<string> res;
    g_pThis->m_pDBProc->Query(lua->GetStringArgument(1), res);
	string allLine;
	BOOST_FOREACH(string item, res)
	{
        string add = item;
        CUniversal::Replace(add,"&","\&");
		if ( allLine.empty())
		{
            allLine = add;
		}
		else
            allLine = allLine + "&" + add;
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

int Sloong::CGlobalFunction::Lua_getSqlError(lua_State *l)
{
    g_pThis->m_pLua->PushString(g_pThis->m_pDBProc->GetError());
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


