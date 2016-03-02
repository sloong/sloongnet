#include "globalfunction.h"
#include <stdlib.h>
#include <univ/log.h>
#include <univ/univ.h>
using namespace Sloong;
using namespace Sloong::Universal;
#include <boost/foreach.hpp>
#include "dbproc.h"
#include "utility.h"
#include "jpeg.h"
#define cimg_display 0
#include "CImg.h"
#include <mutex>
#include "version.h"
using namespace std;
using namespace cimg_library;
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

CGlobalFunction* CGlobalFunction::g_pThis = NULL;
mutex g_SQLMutex;

LuaFunctionRegistr g_LuaFunc[] =
{
	{ "showLog", CGlobalFunction::Lua_showLog },
	{ "querySql", CGlobalFunction::Lua_querySql },
	{ "modifySql", CGlobalFunction::Lua_modifySql },
    { "getSqlError", CGlobalFunction::Lua_getSqlError },
	{ "getThumbImage", CGlobalFunction::Lua_getThumbImage },
	{ "getEngineVer", CGlobalFunction::Lua_getEngineVer },
	{ "Base64_encode", CGlobalFunction::Lua_Base64_Encode },
	{ "Base64_decode", CGlobalFunction::Lua_Base64_Decode },
	{ "MD5_encode", CGlobalFunction::Lua_MD5_Encode },
};

CGlobalFunction::CGlobalFunction()
{
    m_pUtility = new CUtility();
    m_pDBProc = new CDBProc();
	g_pThis = this;
}


CGlobalFunction::~CGlobalFunction()
{
	SAFE_DELETE(m_pUtility);
	SAFE_DELETE(m_pDBProc);
}

void Sloong::CGlobalFunction::Initialize( CLog* plog)
{
    m_pLog = plog;
    // connect to db
    m_pDBProc->Connect("localhost","root","sloong","sloong",0);
}



int Sloong::CGlobalFunction::Lua_querySql(lua_State* l)
{
	vector<string> res;
	unique_lock<mutex> lck(g_SQLMutex);
    g_pThis->m_pDBProc->Query(CLua::GetStringArgument(l,1), res);
	lck.unlock();
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

	CLua::PushString(l,allLine);
	return 1;
}

int Sloong::CGlobalFunction::Lua_modifySql(lua_State* l)
{
	int nRes = g_pThis->m_pDBProc->Modify(CLua::GetStringArgument(l,1));

	CLua::PushString(l,CUniversal::ntos(nRes));
	return 1;
}

int Sloong::CGlobalFunction::Lua_getSqlError(lua_State *l)
{
    CLua::PushString(l,g_pThis->m_pDBProc->GetError());
    return 1;
}


int Sloong::CGlobalFunction::Lua_getThumbImage(lua_State* l)
{
	auto path = CLua::GetStringArgument(l,1);
	auto width = CLua::GetNumberArgument(l,2);
	auto height = CLua::GetNumberArgument(l,3);
	auto quality = CLua::GetNumberArgument(l,4);
	
	if ( access(path.c_str(),ACC_E) != -1 )
	{
		string thumbpath = CUniversal::Format("%s_%d_%d_%d.%s", path.substr(0, path.length() - 4), width, height, quality, path.substr(path.length() - 3));
		if (access(thumbpath.c_str(), ACC_E) != 0)
		{
            CImg<byte> img(path.c_str());
            double ratio = (double)img.width() / (double)img.height();
            if( ratio > 1.0f )
            {
                height = width / ratio;
            }
            else
            {
                width = height * ratio;
            }
            if( width == 0 || height == 0 )
            {
				CLua::PushString(l,path);
                return 1;
            }
            img.resize(width,height);
            img.save(thumbpath.c_str());
		}
		CLua::PushString(l,thumbpath);
	}
	return 1;
}

void Sloong::CGlobalFunction::InitLua(CLua* pLua)
{
	pLua->SetErrorHandle(CGlobalFunction::HandleError);
	vector<LuaFunctionRegistr> funcList(g_LuaFunc, g_LuaFunc + ARRAYSIZE(g_LuaFunc));
	pLua->AddFunctions(&funcList);
}

int Sloong::CGlobalFunction::Lua_getEngineVer(lua_State* l)
{
	CLua::PushString(l, VERSION_TEXT);
	return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Encode(lua_State* l)
{
	string res = CUniversal::Base64_Encoding(CLua::GetStringArgument(l, 1, ""));
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Decode(lua_State* l)
{
	string res = CUniversal::Base64_Decoding(CLua::GetStringArgument(l, 1, ""));
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_MD5_Encode(lua_State* l)
{
	string res = CUniversal::MD5_Encoding(CLua::GetStringArgument(l, 1, ""));
	CLua::PushString(l, res);
	return 1;
}

void CGlobalFunction::HandleError(string err)
{
	g_pThis->m_pLog->Log(err, ERR, -2);
}


int CGlobalFunction::Lua_showLog(lua_State* l)
{
	g_pThis->m_pLog->Log(CLua::GetStringArgument(l,1));
	return 1;
}


