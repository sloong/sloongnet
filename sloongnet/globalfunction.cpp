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

<<<<<<< HEAD
=======
int Sloong::CGlobalFunction::Lua_getThumbImage(lua_State* l)
{
	auto lua = g_pThis->m_pLua;
	auto path = lua->GetStringArgument(1);
	auto width = lua->GetNumberArgument(2);
	auto height = lua->GetNumberArgument(3);
	auto quality = lua->GetNumberArgument(4);
	
	if ( access(path.c_str(),ACC_E) != -1 )
	{
		string thumbpath = CUniversal::Format("%s_%d_%d_%d.%s", path.substr(0, path.length() - 4), width, height, quality, path.substr(path.length() - 3));
		if (access(thumbpath.c_str(), ACC_E) != 0)
		{
			CJPEG jpg;
			jpg.Load(path);
			jpg.Save(quality, width, height, thumbpath);
		}
		lua->PushString(thumbpath);
	}
	return 1;
}


>>>>>>> a5283e8... modify: when send a message fialed, the message is add to epoll list, then function is returnd. but now the message is not send done, in this time, send a other message, the message should be add to epoll list. no should send with direct.
void CGlobalFunction::HandleError(string err)
{
	g_pThis->m_pLog->Log(err, ERR, -2);
}


int CGlobalFunction::Lua_showLog(lua_State* l)
{
    g_pThis->m_pLog->Log(g_pThis->m_pLua->GetStringArgument(1));
}


