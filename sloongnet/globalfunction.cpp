#include "globalfunction.h"
// sys 
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <boost/foreach.hpp>
#include <mutex>
// univ
#include <univ/log.h>
#include <univ/univ.h>
#include "univ/exception.h"
#include <univ/Base64.h>
#include <univ/MD5.h>
// cimag
#define cimg_display 0
#include "CImg.h"

#include "utility.h"
#include "jpeg.h"
#include "version.h"
#include "serverconfig.h"
#include "epollex.h"
#include "NormalEvent.h"

using namespace std;
using namespace cimg_library;
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;
using namespace Sloong::Interface;

CGlobalFunction* CGlobalFunction::g_pThis = NULL;
mutex g_SQLMutex;

static string g_temp_file_path = "/tmp/sloong/receivefile/temp.tmp";

LuaFunctionRegistr g_LuaFunc[] =
{
	{ "Sloongnet_ShowLog", CGlobalFunction::Lua_showLog },
	{ "Sloongnet_GetThumbImage", CGlobalFunction::Lua_getThumbImage },
	{ "Sloongnet_GetEngineVer", CGlobalFunction::Lua_getEngineVer },
	{ "Sloongnet_Base64_encode", CGlobalFunction::Lua_Base64_Encode },
	{ "Sloongnet_Base64_decode", CGlobalFunction::Lua_Base64_Decode },
	{ "Sloongnet_MD5_encode", CGlobalFunction::Lua_MD5_Encode },
	{ "Sloongnet_SendFile", CGlobalFunction::Lua_SendFile },
	{ "Sloongnet_ReloadScript", CGlobalFunction::Lua_ReloadScript },
	{ "Sloongnet_Get", CGlobalFunction::Lua_GetConfig },
	{ "Sloongnet_MoveFile", CGlobalFunction::Lua_MoveFile },
	{ "Sloongnet_GenUUID", CGlobalFunction::Lua_GenUUID },
	{ "Sloongnet_ReceiveFile", CGlobalFunction::Lua_ReceiveFile},
	{ "Sloongnet_SetCommData", CGlobalFunction::Lua_SetCommData},
	{ "Sloongnet_GetCommData", CGlobalFunction::Lua_GetCommData },
	{ "Sloongnet_GetLogObject", CGlobalFunction::Lua_GetLogObject },
};

CGlobalFunction::CGlobalFunction()
{
    m_pUtility = new CUtility();
	g_pThis = this;
}


CGlobalFunction::~CGlobalFunction()
{
	SAFE_DELETE(m_pUtility);
}

void Sloong::CGlobalFunction::Initialize(IMessage* iMsg, IData* iData)
{
	m_iMsg = iMsg;
	m_iData = iData;
    m_pLog = TYPE_TRANS<CLog*>(m_iData->Get(Logger));
	CServerConfig* config = TYPE_TRANS<CServerConfig*>(m_iData->Get(Configuation));
}


int Sloong::CGlobalFunction::Lua_getThumbImage(lua_State* l)
{
	auto path = CLua::GetString(l,1);
	auto width = CLua::GetDouble(l,2);
	auto height = CLua::GetDouble(l,3);
	auto quality = CLua::GetDouble(l,4);
	auto folder = CLua::GetString(l, 5, "");
	
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
            CImg<UCHAR> img(path.c_str());
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
		return 1;
	}
	CLua::PushString(l, "");
	return 1;
}

void Sloong::CGlobalFunction::InitLua(CLua* pLua)
{
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
    string req = CLua::GetString(l, 1, "");
    string res = CBase64::Encoding((const unsigned char*)req.c_str(),req.length());
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Decode(lua_State* l)
{
    string req = CLua::GetString(l, 1, "");
    string res = CBase64::Decoding(req);
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_MD5_Encode(lua_State* l)
{
    string res = CMD5::Encoding(CLua::GetString(l, 1, ""));
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_SendFile(lua_State* l)
{
	auto filename = CLua::GetString(l, 1, "");
	if (filename == "")
	{
		CLua::PushDouble(l,-1);
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
		g_pThis->m_pLog->Error(e.what());
		CLua::PushDouble(l, -1);
		CLua::PushString(l, e.what());
		return 2;
	}

	auto uuid = CUtility::GenUUID();

	SendExDataInfo* info=new SendExDataInfo();
	info->m_nDataSize = nSize;
	info->m_pData = pBuf;
	g_pThis->m_iData->AddTemp("SendList" + uuid, info);

	CLua::PushString(l, uuid);
	CLua::PushString(l, "succeed");
	return 2;
}

int Sloong::CGlobalFunction::Lua_ReloadScript(lua_State* l)
{
	CNormalEvent* evt = new CNormalEvent();
	evt->SetEvent(MSG_TYPE::ReloadLuaContext);
	g_pThis->m_iMsg->SendMessage(evt);
	return 0;
}

int Sloong::CGlobalFunction::Lua_GetConfig(lua_State* l)
{
	string section = CLua::GetString(l,1);
	string key = CLua::GetString(l,2);
	string def = CLua::GetString(l,3);
	
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
	string orgName = CLua::GetString(l, 1, "");
	string newName = CLua::GetString(l, 2, "");
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
		g_pThis->m_pLog->Error(e.what());
		CLua::PushString(l, e.what());
		CLua::PushDouble(l, nRes);
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
		string uuid = CLua::GetString(l, 1);
		if (uuid.empty() || uuid == "")
		{
			throw normal_except("uuid is empty");
		}
		int port = CLua::GetDouble(l, 2);
		if (port < 1024 || port > 65535)
		{
			throw normal_except("port is illegal");
		}
		int max_size = CLua::GetDouble(l, 3);
		auto fileList = CLua::GetTableParam(l, 4);
		int otime = CLua::GetDouble(l, 5, 5);
		string temp_file_path = CLua::GetString(l, 6, g_temp_file_path);
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
        res = CUniversal::RecvEx(cSocket, pBuf, 36, otime);
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
            res = CUniversal::RecvEx(cSocket, pBuf, 8, otime);
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
            res = CUniversal::RecvEx(cSocket, pBuf, nRecvLen, otime);

			// check target file path is not exist
			CUniversal::CheckFileDirectory(temp_file_path);

			// save to file
			ofstream of;
			of.open(temp_file_path.c_str(), ios::out | ios::trunc | ios::binary);
			of.write(pBuf, nRecvLen);
			of.close();
			// check md5
            string md5 = CMD5::Encoding(temp_file_path, true);
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

int CGlobalFunction::Lua_showLog(lua_State* l)
{
	string luaTitle = CLua::GetString(l, 2, "Info");
	string msg = CLua::GetString(l, 1);
	if (msg == "")
		return 0;
	else
		msg = CUniversal::Format("[Script]:[%s]", msg);

	auto log = g_pThis->m_pLog;
	if (luaTitle == "Info")
		log->Info(msg);
	else if (luaTitle == "Warn")
		log->Warn(msg);
	else if (luaTitle=="Debug")
		log->Debug(msg);
	else if (luaTitle == "Error")
		log->Error(msg);
	else if (luaTitle == "Verbos")
		log->Verbos(msg);
	else if (luaTitle == "Assert")
		log->Assert(msg);
	else if (luaTitle == "Fatal")
		log->Fatal(msg);
	else
		log->Log(msg,luaTitle);
	return 0;
}


int CGlobalFunction::Lua_SetCommData(lua_State* l)
{
	
}

int CGlobalFunction::Lua_GetCommData(lua_State* l)
{

}

int CGlobalFunction::Lua_GetLogObject(lua_State* l)
{
	CLua::PushPointer(l, g_pThis->m_pLog);
	return 1;
}