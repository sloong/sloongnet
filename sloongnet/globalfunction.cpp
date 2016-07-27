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
#include "epollex.h"
#include<sys/socket.h>
#include <netinet/in.h>
using namespace std;
using namespace cimg_library;
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

CGlobalFunction* CGlobalFunction::g_pThis = NULL;
mutex g_SQLMutex;

static string g_temp_file_path = "/tmp/sloong/receivefile/temp.tmp";

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
	{ "ReceiveFile", CGlobalFunction::Lua_ReceiveFile},
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
	m_pSQLInfo = info;
    // connect to db
	if (info->Enable)
	{
		try
		{
			m_pDBProc->Connect(info);
		}
		catch (normal_except& e)
		{
			g_pThis->m_pLog->Info(e.what(), "ERR");
			throw e;
		}
	}
}



int Sloong::CGlobalFunction::Lua_querySql(lua_State* l)
{
	if (!g_pThis->m_pSQLInfo->Enable)
	{
		CLua::PushInteger(l, -1);
		CLua::PushString(l, "SQL Server is no enable in config file.");
		return 2;
	}

	string cmd = CLua::GetStringArgument(l, 1);
	if ( g_pThis->m_bShowSQLCmd )
		g_pThis->m_pLog->Info(cmd,"SQL");
	vector<string> res;
	unique_lock<mutex> lck(g_SQLMutex);
	int nRes = 0;
	try
	{
		nRes = g_pThis->m_pDBProc->Query(cmd, &res);
	}
	catch (normal_except& e)
	{
		lck.unlock();
		string err = CUniversal::Format("SQL Query error:[%s]", e.what());
		g_pThis->m_pLog->Info( err , "ERROR");
		CLua::PushInteger(l, -1);
		CLua::PushString(l, err);
		return 2;
	}
	
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
	if (!g_pThis->m_pSQLInfo->Enable)
		CLua::PushString(l, "SQL Server is no enable in config file.");
	else
	   CLua::PushString(l,g_pThis->m_pDBProc->GetError());
    return 1;
}


int Sloong::CGlobalFunction::Lua_getThumbImage(lua_State* l)
{
	auto path = CLua::GetStringArgument(l,1);
	auto width = CLua::GetNumberArgument(l,2);
	auto height = CLua::GetNumberArgument(l,3);
	auto quality = CLua::GetNumberArgument(l,4);
	auto folder = CLua::GetStringArgument(l, 5, "");
	
	if ( access(path.c_str(),ACC_E) != -1 )
	{
		if (folder == "")
			folder = path.substr(0, path.find_last_of('/'));

		string fileName = path.substr(path.find_last_of('/') + 1);
		string extension = fileName.substr(fileName.find_last_of('.')+1);
		fileName = fileName.substr(0, fileName.length() - extension.length()-1);
		string thumbpath = CUniversal::Format("%s/%s_%d_%d_%d.%s", folder, fileName, width, height, quality,extension);
		CUniversal::CheckFileDirectory(thumbpath);	
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
	CLua::PushString(l, "");
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
	catch (normal_except& e)
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
			CLua::PushString(l, "succeed");
			return 2;
		}
	}

	SendExDataInfo info;
	info.m_nDataSize = nSize;
	info.m_pData = pBuf;
	info.m_bIsEmpty = false;
	rList[rList.size()] = info;
	CLua::PushNumber(l, rList.size() - 1);
	CLua::PushString(l, "succeed");
	return 2;
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
	catch (normal_except& e)
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

		string dir = CUniversal::CheckFileDirectory(newName);
		if (access(dir.c_str(), ACC_RW) != 0)
		{
			nRes = -1;
			throw normal_except("Move File error.folder can not read / write" + dir);
		}

		system(CUniversal::Format("mv -f %s %s", orgName.c_str(), newName.c_str()).c_str());
	}
	catch (normal_except& e)
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

int CGlobalFunction::Lua_GenUUID(lua_State* l)
{
	CLua::PushString(l, CUtility::GenUUID());
	return 1;
}

int CGlobalFunction::Lua_ReceiveFile(lua_State * l)
{
	int succeed_num = 0;
	string succeed_md5_list("");
	try
	{
		string uuid = CLua::GetStringArgument(l, 1);
		if (uuid.empty() || uuid == "")
		{
			throw normal_except("uuid is empty");
		}
		int port = CLua::GetNumberArgument(l, 2);
		if (port < 1024 || port > 65535)
		{
			throw normal_except("port is illegal");
		}
		int max_size = CLua::GetNumberArgument(l, 3);
		auto fileList = CLua::GetTableParam(l, 4);
		int otime = CLua::GetNumberArgument(l, 5, 5);
		string temp_file_path = CLua::GetStringArgument(l, 6, g_temp_file_path);
		int rSocket = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in address;
		memset(&address, 0, sizeof(address));
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);
		errno = bind(rSocket, (struct sockaddr*)&address, sizeof(address));
		if (errno == -1)
		{
			throw normal_except(CUniversal::Format("bind to %d field. errno = %d", port, errno));
		}

		errno = listen(rSocket, 1);

		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(rSocket, &rset);
		struct timeval tv;
		tv.tv_sec = otime;
		tv.tv_usec = 0;
		int res = select(rSocket + 1, &rset, NULL, NULL, &tv);
		int cSocket = -1;
		if (res == 0)
		{
			close(rSocket);
			throw normal_except("client no connect in set time.");
		}
		else if (res > 0)
		{
			cSocket = accept(rSocket, NULL, NULL);
			close(rSocket);
		}
		else
		{
			// unknown error
			close(rSocket);
			throw normal_except("unknown error happened when wait client connect.");
		}

		char* pBuf = NULL;
		// receive the uuid
		char strUuid[37] = { 0 };
		pBuf = strUuid;
		res = CEpollEx::RecvEx(cSocket, &pBuf, 36, 0);
		if (string(strUuid) != uuid)
		{
			close(cSocket);
			throw normal_except("uuid check error.");
		}
		
		for (map<string, string>::iterator i = fileList.begin(); i != fileList.end(); i++)
		{
			// receive the length
			char strLen[9] = { 0 };
			pBuf = strLen;
			res = CEpollEx::RecvEx(cSocket, &pBuf, 8, 0);
			long long nRecvLen = atoi(strLen);
			// check the length
			if (max_size < nRecvLen)
			{
				close(cSocket);
				throw normal_except("receive file size is big than" + CUniversal::ntos(max_size));
			}
			// receive the data
			pBuf = new char[nRecvLen];
			memset(pBuf, 0, nRecvLen);
			res = CEpollEx::RecvEx(cSocket, &pBuf, nRecvLen, 0);

			// check target file path is not exist
			CUniversal::CheckFileDirectory(temp_file_path);

			// save to file
			ofstream of;
			of.open(temp_file_path.c_str(), ios::out | ios::trunc | ios::binary);
			of.write(pBuf, nRecvLen);
			of.close();
			// check md5
			string md5 = CUniversal::MD5_Encoding(temp_file_path.c_str(), true);
		    CUniversal::tolower(md5);
			if (fileList.count(md5) == 0 )
			{
				CUniversal::touper(md5);
				if (fileList.count(md5) == 0)
				{
					// 没有目标md5，表示文件有问题
					// Close the socket
					close(cSocket);
					throw normal_except(CUniversal::Format("no find target md5[%s] in list.",md5.c_str()));
				}
			}
			
			string netpath = fileList[md5];
			CUniversal::CheckFileDirectory(netpath);
			system(CUniversal::Format("mv -f %s %s", temp_file_path.c_str(), netpath.c_str()).c_str());
			succeed_md5_list = succeed_md5_list + md5 + ";";
			succeed_num++;
		}
		
		// Close the socket
		close(cSocket);

		CLua::PushInteger(l, succeed_num);
		CLua::PushString(l, succeed_md5_list);
		CLua::PushString(l, "succeed");
		return 3;
	}
	catch (normal_except& ex)
	{
		CLua::PushInteger(l, succeed_num);
		CLua::PushString(l, succeed_md5_list);
		CLua::PushString(l, ex.what());
		return 3;
	}
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


