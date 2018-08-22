#include "globalfunction.h"
// sys 
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <boost/foreach.hpp>
#include <mutex>
// univ
#include "defines.h"
#include <univ/Base64.h>

#include "utility.h"
#include "version.h"
#include "serverconfig.h"
#include "epollex.h"
#include "NormalEvent.h"

using namespace std;

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;
using namespace Sloong::Interface;

CGlobalFunction* CGlobalFunction::g_pThis = NULL;
mutex g_SQLMutex;
map<int,string> g_RecvDataConnList;

map<int,string> g_RecvDataConnList;
map<string,map<string,RecvDataPackage>> g_RecvDataInfoList;

static string g_temp_file_path = "/tmp/sloong/receivefile/temp.tmp";

LuaFunctionRegistr g_LuaFunc[] =
{
	{ "Sloongnet_ShowLog", CGlobalFunction::Lua_showLog },
	{ "Sloongnet_GetEngineVer", CGlobalFunction::Lua_getEngineVer },
	{ "Sloongnet_Base64_encode", CGlobalFunction::Lua_Base64_Encode },
	{ "Sloongnet_Base64_decode", CGlobalFunction::Lua_Base64_Decode },
	{ "Sloongnet_MD5_encode", CGlobalFunction::Lua_MD5_Encode },
	{ "Sloongnet_SHA256_encode", CGlobalFunction::Lua_SHA256_Encode },
	{ "Sloongnet_SHA512_encode", CGlobalFunction::Lua_SHA512_Encode },
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

	CServerConfig* pConfig = TYPE_TRANS<CServerConfig*>(m_iData->Get(DATA_ITEM::Configuation));
	if( pConfig->m_bEnableDataReceive)
	{
		EnableDataReceive( pConfig->m_nDataReceivePort);
	}
}

void Sloong::CGlobalFunction::RegistFuncToLua(CLua* pLua)
{
	vector<LuaFunctionRegistr> funcList(g_LuaFunc, g_LuaFunc + ARRAYSIZE(g_LuaFunc));
	pLua->AddFunctions(&funcList);
}

void Sloong::CGlobalFunction::EnableDataReceive(int port)
{
	m_nDataRecvPort = port;
	if ( m_nDataRecvPort > 0 )
	{
		int m_ListenSock = socket(AF_INET, SOCK_STREAM, 0);
		int sock_op = 1;
		// SOL_SOCKET:在socket层面设置
		// SO_REUSEADDR:允许套接字和一个已在使用中的地址捆绑
		setsockopt(m_ListenSock, SOL_SOCKET, SO_REUSEADDR, &sock_op, sizeof(sock_op));

		struct sockaddr_in address;
		memset(&address, 0, sizeof(address));
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(port);

		errno = bind(m_ListenSock, (struct sockaddr*)&address, sizeof(address));

		if (errno == -1)
			throw normal_except(CUniversal::Format("bind to %d field. errno = %d", m_pConfig->m_nPort, errno));

		errno = listen(m_ListenSock, 1024);
		SetSocketNonblocking(m_ListenSock);

		CThreadPool::AddWorkThread(RecvDataConnFunc, pThis, 1);
	}
}


// 
void* Sloong::CGlobalFunction::RecvDataConnFunc(void* pParam)
{
	CGlobalFunction* pThis = (CGlobalFunction*)pParam;
	CLog* pLog = pThis->m_pLog;
	
	while( m_bIsRunning)
	{
		int conn_sock = -1;
		if(( conn_sock = accept(pThis->m_ListenSock, NULL, NULL)) > 0 )
		{
			static int uuid_length = 32;
			// When accept the connect , receive the uuid data. and 
			char* pCheckBuf = new char[uuid_length + 1];
			memset(pCheckBuf, 0, uuid_length + 1);
			// In Check function, client need send the check key in 3 second. 
			// 这里仍然使用Universal提供的ReceEx。这里不需要进行SSL接收
			int nLen = CUniversal::RecvEx(conn_sock, pCheckBuf, uuid_length, m_pConfig->m_nClientCheckTime);
			// Check uuid length
			if (nLen != m_nClientCheckKeyLength)
			{
				pLog->Warn(CUniversal::Format("The uuid length error:[%d]. Close connect.",nLen))
				close(conn_sock);
				continue;
			}
			// Check uuid validity
			if( g_RecvDataInfoList.find(pCheckBuf) == g_RecvDataInfoList.end())
			{
				pLog->Warn(CUniversal::Format("The uuid is not find in list:[%s]. Close connect.",pCheckBuf))
				close(conn_sock);
				continue;
			}
			// Add to connect list
			g_RecvDataConnList[conn_sock] = pCheckBuf;
			// Start new thread to recv data for this connect.
			int* pSock = new int();
			*pSock = conn_sock;
			LPVOID* params = new LPVOID[2]();
			params[0] = pThis;
			params[1] = pSock;
			CThreadPool::AddWorkThread(RecvFileFunc,params);
		}
	}
}

void* Sloong::CGlobalFunction::RecvFileFunc(void* pParam)
{
	LPVOID* pParams = (LPVOID*)pParam;
	CGlobalFunction* pThis = (CGlobalFunction*)pParams[0];
	CLog* pLog = pThis->m_pLog;
	int* pSock = (int*)pParams[1];
	int conn_sock = *pSock;
	SAFE_DELETE(pSock);
	SAFE_DELETE_ARR(pParams);
	// Find the recv uuid.
	auto conn_item = g_RecvDataConnList.find(conn_sock);
	if(conn_item == g_RecvDataConnList.end())
	{
		p_Log->Error("The socket id is not find in conn list.");
		return nullptr;
	}
	string uuid = conn_item.second;
	// Find the recv info list.
	auto info_item = g_RecvDataInfoList.find(uuid);
	if(info_item == g_RecvDataInfoList.end() )
	{
		p_Log->Error("The uuid is not find in info list.");
		return nullptr;
	}
	map<string,RecvDataPackage> recv_file_list = info_item.second;
	bool bLoop = true;
	while (bLoop)
	{
		// 先读取消息长度
		static int data_package_len = 8;
		char* pLongBuffer = new char[data_package_len + 1]();//dataLeng;
		memset(pLongBuffer, 0, data_package_len + 1);
		int nRecvSize = CUniversal::RecvEx( conn_sock, pLongBuffer, data_package_len, m_pConfig->m_nReceiveTimeout);
		if (nRecvSize <= 0)
		{
			// 读取错误,将这个连接从监听中移除并关闭连接
			CloseConnect(nSocket);
			SAFE_DELETE_ARR(pLongBuffer);
			pLog->Warn("Recv data package length error.");
			return nullptr;
		}
		else
		{
			long long dtlen = CUniversal::BytesToLong(pLongBuffer);
			SAFE_DELETE_ARR(pLongBuffer);
			// package length cannot big than 2147483648. this is max value for int.
			if (dtlen <= 0 || dtlen > 2147483648 || nRecvSize != data_package_len)
			{
				m_pLog->Error("Receive data length error.");
				CloseConnect(nSocket);
				return nullptr;
			}

			char szMD5 = new char[33];
			memset(szMD5, 0 ,33);
			nRecvSize = CUniversal::RecvEx( conn_sock, szMD5, 32, m_pConfig->m_nReceiveTimeout, true);
			if (nRecvSize <= 0)
			{
				m_pLog->Error("Receive data package md5 error.");
				CloseConnect(nSocket);
				SAFE_DELETE_ARR(szMD5);
				return nullptr;
			}
			auto recv_file_item = recv_file_list.find(tmd5);
			if( recv_file_item == recv_file_list.end())
			{
				m_pLog->Error("the file md5 is not find in recv list.");
				CloseConnect(nSocket);
				return nullptr;
			}
			RecvDataPackage pack = recv_file_item.second;
			pack.emStatus = RecvStatus::Receiving;

			char* data = new char[dtlen + 1];
			memset(data, 0, dtlen + 1);

			nRecvSize = CUniversal::RecvEx(conn_sock,data, dtlen, m_pConfig->m_nReceiveTimeout, true);//一次性接受所有消息
			if (nRecvSize <= 0)
			{
				m_pLog->Error("Receive data error.");
				CloseConnect(nSocket);
				SAFE_DELETE_ARR(data);
				return nullptr;
			}
			pack.emStatus = RecvStatus::Saveing;			
			
			// check target file path is not exist
			CUniversal::CheckFileDirectory(pack.strPath);

			// save to file
			ofstream of;
			of.open(pack.strPath + pack.strName, ios::out | ios::trunc | ios::binary);
			of.write(pMsg,dtlen);
			of.close();
			SAFE_DELETE_ARR(data);

			// check md5
			if(0 != stricmp(tmd5,CMD5::Encode(pack.strPath+pack.strName,true)))
			{
				m_pLog->Error("the file data is different with md5 code.");
				pack.emStatus = RecvStatus::Error;
			}
			else
			{
				pack.emStatus = RecvStatus::Done;
			}
			bLoop = true;
		}
	}

}

int Sloong::CGlobalFunction::Lua_getEngineVer(lua_State* l)
{
	CLua::PushString(l, VERSION_TEXT);
	return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Encode(lua_State* l)
{
    string req = CLua::GetString(l, 1, "");
    string res = CBase64::Encode(req);
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_Base64_Decode(lua_State* l)
{
    string req = CLua::GetString(l, 1, "");
    string res = CBase64::Decode(req);
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_MD5_Encode(lua_State* l)
{
    string res = CMD5::Encode(CLua::GetString(l, 1, ""));
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_SHA256_Encode(lua_State* l)
{
	string res = CSHA256::Encode(CLua::GetString(l, 1, ""));
	CLua::PushString(l, res);
	return 1;
}

int Sloong::CGlobalFunction::Lua_SHA512_Encode(lua_State* l)
{
	string res = CSHA512::Encode(CLua::GetString(l, 1, ""));
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

	g_pThis->m_iData->AddTemp("SendList" + uuid, pBuf);
	CLua::PushDouble(l, nSize);
	CLua::PushString(l, uuid);
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


// Receive File funcs
// Client requeset with file list info 
// and here add the info to list and Build one uuid and return this uuid.
int CGlobalFunction::Lua_ReceiveFile(lua_State * l)
{
	try
	{
		string save_folder = CLua::GetString(l, 1);
		if (save_folder.empty() || save_folder == "")
		{
			throw normal_except("save folder is empty");
		}

		// The file list, key is md5 ,value is file name
		auto fileList = CLua::GetTableParam(l,2);
		string uuid = CUtility::GenUUID();
		
		map<string,RecvDataPackage> recv_list;
		for (auto i = fileList.begin(); i != fileList.end(); i++)
		{
			RecvDataPackage pack;
			pack.strName = i.second;
			pack.strPath = save_folder;
			pack.strMD5 = i.first;
			pack.emStatus = RecvStatus::Wait;
			recv_list[i.first] = pack;
		}

		g_RecvDataInfoList[uuid] = recv_list;

		CLua::PushBoolen(l, true);
		CLua::PushString(l, uuid);
		return 2;
	}
	catch (normal_except& ex)
	{
		CLua::PushBoolen(l, false);
		CLua::PushString(l, ex.what());
		return 2;
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