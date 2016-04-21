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
#include "univ/exception.h"
#include <mutex>
#include "version.h"
#include "serverconfig.h"
using namespace std;
using namespace cimg_library;
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

CGlobalFunction* CGlobalFunction::g_pThis = NULL;
mutex g_SQLMutex;

LuaFunctionRegistr g_LuaFunc[] =
{
	{ "ShowLog", CGlobalFunction::Lua_showLog },
	{ "QuerySQL", CGlobalFunction::Lua_querySql },
	{ "GetThumbImage", CGlobalFunction::Lua_getThumbImage },
	{ "GetEngineVer", CGlobalFunction::Lua_getEngineVer },
	{ "Base64_encode", CGlobalFunction::Lua_Base64_Encode },
	{ "Base64_decode", CGlobalFunction::Lua_Base64_Decode },
	{ "MD5_encode", CGlobalFunction::Lua_MD5_Encode },
	{ "SendFile", CGlobalFunction::Lua_SendFile },
	{ "ReloadScript", CGlobalFunction::Lua_ReloadScript },
	{ "Get", CGlobalFunction::Lua_GetConfig },
	{ "MoveFile", CGlobalFunction::Lua_MoveFile },
	{ "GenUUID", CGlobalFunction::Lua_GenUUID },
    { "GetSQLError", CGlobalFunction::Lua_getSqlError},
};

CGlobalFunction::CGlobalFunction()
{
    m_pUtility = new CUtility();
    m_pDBProc = new CDBProc();
	g_pThis = this;
	m_pReloadTagList = NULL;
}


CGlobalFunction::~CGlobalFunction()
{
	SAFE_DELETE(m_pUtility);
	SAFE_DELETE(m_pDBProc);

	unique_lock<mutex> lck(m_oListMutex);
	int nSize = m_oSendExMapList.size();
	for (int i = 0; i < nSize; i++)
	{
		if (!m_oSendExMapList[i].m_bIsEmpty)
			SAFE_DELETE_ARR(m_oSendExMapList[i].m_pData)
			m_oSendExMapList[i].m_bIsEmpty = true;
	}
	lck.unlock();
	SAFE_DELETE_ARR(m_pReloadTagList);
}

void Sloong::CGlobalFunction::Initialize(CLog* plog, MySQLConnectInfo* info, bool bShowCmd, bool bShowRes)
{
    m_pLog = plog;
	m_bShowSQLCmd = bShowCmd;
	m_bShowSQLResult = bShowRes;
    // connect to db
    try
    { 
        m_pDBProc->Connect(info);
    }
    catch(normal_except e)
    {
         g_pThis->m_pLog->Info(e.what(),"ERR");
    }
}



int Sloong::CGlobalFunction::Lua_querySql(lua_State* l)
{
	string cmd = CLua::GetStringArgument(l, 1);
	if ( g_pThis->m_bShowSQLCmd )
		g_pThis->m_pLog->Info(cmd,"SQL");
	vector<string> res;
	unique_lock<mutex> lck(g_SQLMutex);
	int nRes = g_pThis->m_pDBProc->Query(cmd, &res);
	lck.unlock();
	string allLine;
	char line = 0x0A;
	BOOST_FOREACH(string item, res)
	{
        string add = item;
		if ( allLine.empty())
		{
            allLine = add;
		}
		else
            allLine = allLine + line + add;
	}
	if (g_pThis->m_bShowSQLResult)
		g_pThis->m_pLog->Info(CUniversal::Format("Rows:[%d],Res:[%s]", nRes, allLine.c_str()), "SQL");
	
	CLua::PushInteger(l, nRes);
	CLua::PushString(l,allLine);
	return 2;
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

int Sloong::CGlobalFunction::Lua_SendFile(lua_State* l)
{
	auto filename = CLua::GetStringArgument(l, 1, "");
	if (filename == "")
	{
		CLua::PushNumber(l,-1);
		CLua::PushString(l,"Param is empty.");
		return 2;
	}
		
	char* pBuf = NULL;
	int nSize = 0;
	try
	{
		nSize = CUtility::ReadFile(filename, pBuf);
	}
	catch (normal_except e)
	{
		g_pThis->m_pLog->Log(e.what(), ERR);
		CLua::PushNumber(l, -1);
		CLua::PushString(l, e.what());
		return 2;
	}

	auto& rList = g_pThis->m_oSendExMapList;
	unique_lock<mutex> lck(g_pThis->m_oListMutex);
	int nListSize = rList.size();
	for (int i = 0; i < nListSize; i++)
	{
		if (rList[i].m_bIsEmpty)
		{
			rList[i].m_nDataSize = nSize;
			rList[i].m_pData = pBuf;
			rList[i].m_bIsEmpty = false;
			lck.unlock();
			CLua::PushNumber(l, i);
			return 1;
		}
	}

	SendExDataInfo info;
	info.m_nDataSize = nSize;
	info.m_pData = pBuf;
	info.m_bIsEmpty = false;
	rList[rList.size()] = info;
	return rList.size() - 1;
}

int Sloong::CGlobalFunction::Lua_ReloadScript(lua_State* l)
{
	for (int i = 0; i < g_pThis->m_nTagSize; i++)
	{
		g_pThis->m_pReloadTagList[i] = true;
	}
	return 0;
}

int Sloong::CGlobalFunction::Lua_GetConfig(lua_State* l)
{
	string section = CLua::GetStringArgument(l,1);
	string key = CLua::GetStringArgument(l,2);
	string def = CLua::GetStringArgument(l,3);
	
	string value("");
	try
	{
		value = CServerConfig::GetStringConfig(section, key, def);
	}
	catch (normal_except e)
	{
		CLua::PushString(l, "");
		CLua::PushString(l, e.what());
		return 2;
	}
	
	CLua::PushString(l, value);
	return 1;
}

int Sloong::CGlobalFunction::Lua_MoveFile(lua_State* l)
{
	string orgName = CLua::GetStringArgument(l, 1, "");
	string newName = CLua::GetStringArgument(l, 2, "");
	int nRes(0);
	try
	{
		if (orgName == "" || newName == "")
		{
			nRes = -2;
			throw normal_except("Move File error. File name cannot empty. orgName:" + orgName + ";newName:" + newName);
		}

		if (access(orgName.c_str(), ACC_W) != 0)
		{
			nRes = -1;
			throw normal_except("Move File error. Origin file not exist or can not write" + orgName);
		}

		string dir = CUtility::CheckFileDirectory(newName);
		if (access(dir.c_str(), ACC_RW) != 0)
		{
			nRes = -1;
			throw normal_except("Move File error.folder can not read / write" + dir);
		}

		system(CUniversal::Format("mv -f %s %s", orgName.c_str(), newName.c_str()).c_str());
	}
	catch (normal_except e)
	{
		g_pThis->m_pLog->Log(e.what());
		CLua::PushString(l, e.what());
		CLua::PushNumber(l, nRes);
		return 2;
	}
	
	// if succeed return 0, else return nozero
	CLua::PushString(l, "mv file succeed");
	CLua::PushInteger(l, 0);
	return 2;
}

int Sloong::CGlobalFunction::Lua_GenUUID(lua_State* l)
{
	CLua::PushString(l, CUtility::GenUUID());
	return 1;
}

void CGlobalFunction::HandleError(string err)
{
	g_pThis->m_pLog->Log(err, ERR, -2);
}

int CGlobalFunction::Lua_showLog(lua_State* l)
{
	string luaMode = CLua::GetStringArgument(l, 2, "");
	if ( luaMode == "" )
	{
		luaMode = "Script";
	}
	else
	{
		luaMode = CUniversal::Format("Script:%s", luaMode);
	}
	g_pThis->m_pLog->Info(CLua::GetStringArgument(l,1),luaMode.c_str());
	return 1;
}


