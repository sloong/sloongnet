#include "LuaProcessCenter.h"
#include "main.h"
#include <univ/luapacket.h>
#include <univ/lua.h>
#include "serverconfig.h"
#include "globalfunction.h"
using namespace Sloong;

CLog* Sloong::CLuaProcessCenter::m_pLog = nullptr;

CLuaProcessCenter::CLuaProcessCenter()
{
}


CLuaProcessCenter::~CLuaProcessCenter()
{
	int nLen = m_pLuaList.size();
	for (int i = 0; i < nLen; i++)
	{
		SAFE_DELETE(m_pLuaList[i]);
	}
}

void Sloong::CLuaProcessCenter::Initialize(IMessage* iMsg, IData* iData)
{
	m_iMsg = iMsg;
	m_iData = iData;

	m_pLog = TYPE_TRANS<CLog*>(m_iData->Get(DATA_ITEM::Logger));
	m_pConfig = TYPE_TRANS<CServerConfig*>(iData->Get(DATA_ITEM::Configuation));

	m_iMsg->RegisterEvent(MSG_TYPE::ProcessMessage);
	m_iMsg->RegisterEvent(MSG_TYPE::ReloadLuaContext);
	m_iMsg->RegisterEventHandler(ProcessMessage, this, EventHandler);
	m_iMsg->RegisterEventHandler(ReloadLuaContext, this, EventHandler);
	m_iMsg->RegisterEventHandler(ReveivePackage, this, EventHandler);
	// 主要的循环方式为，根据输入的处理数来初始化指定数量的lua环境。
	// 然后将其加入到可用队列
	// 在处理开始之前根据队列情况拿到某lua环境的id并将其移除出可用队列
	// 在处理完毕之后重新加回到可用队列中。
	// 这里使用处理线程池的数量进行初始化，保证在所有线程都在处理Lua请求时不会因luacontext发生堵塞
	for (int i = 0; i < m_pConfig->m_nProcessThreadQuantity; i++)
		NewThreadInit();

}


void Sloong::CLuaProcessCenter::HandleError(string err)
{
	m_pLog->Error(CUniversal::Format("[Script]:[%s]", err));
}

void* Sloong::CLuaProcessCenter::EventHandler(LPVOID evt, LPVOID obj)
{
	IEvent* ev = TYPE_TRANS<IEvent*>(evt);
	auto type = ev->GetEvent();
	CLuaProcessCenter * pThis = TYPE_TRANS<CLuaProcessCenter*>(obj);
	switch (type)
	{
	case ProcessMessage:
		//pThis->MsgProcess(ev);
		break;
	case ReveivePackage:

		break;
	case ReloadLuaContext:
		pThis->ReloadContext();
		break;
	default:
		break;
	}
	SAFE_RELEASE_EVENT(ev);
	return nullptr;
}

void Sloong::CLuaProcessCenter::ReloadContext()
{
	int n = m_pLuaList.size();
	for (int i=0;i<n;i++)
	{
		m_oReloadList[i] = true;
	}
}



int Sloong::CLuaProcessCenter::NewThreadInit()
{
	CLua* pLua = new CLua();
	pLua->SetErrorHandle(HandleError);
	pLua->SetScriptFolder(m_pConfig->m_oLuaConfigInfo.ScriptFolder);
	auto pGFunc = TYPE_TRANS<CGlobalFunction*>(m_iData->Get(GlobalFunctions));
	pGFunc->InitLua(pLua);
	InitLua(pLua, m_pConfig->m_oLuaConfigInfo.ScriptFolder);
	m_pLuaList.push_back(pLua);
	m_oReloadList.push_back(false);
	int id = m_pLuaList.size() - 1;
	m_oFreeLuaContext.push(id);
	return id;
}

void Sloong::CLuaProcessCenter::InitLua(CLua* pLua, string folder)
{
	if (!pLua->RunScript(m_pConfig->m_oLuaConfigInfo.EntryFile))
	{
		throw normal_except("Run Script Fialed.");
	}
	// get current path
	char szDir[MAX_PATH] = { 0 };

	getcwd(szDir, MAX_PATH);
	string strDir(szDir);
	strDir += "/" + folder;
	pLua->RunFunction(m_pConfig->m_oLuaConfigInfo.EntryFunction, CUniversal::Format("'%s'", strDir));
}

void Sloong::CLuaProcessCenter::CloseSocket(CLuaPacket* uinfo)
{
	// call close function.
	int id = GetFreeLuaContext();
	CLua* pLua = m_pLuaList[id];
	pLua->RunFunction(m_pConfig->m_oLuaConfigInfo.SocketCloseFunction, uinfo);
	m_oFreeLuaContext.push(id);
}


string FormatJSONErrorMessage(string message, string code)
{
	return CUniversal::Format("{\"errno\": \"%s\",\"errmsg\" : \"%s\"}", code, message);
}

bool Sloong::CLuaProcessCenter::MsgProcess(CLuaPacket * pUInfo, string & msg, string & res, char*& exData, int& exSize)
{
	// In process, need add the lua script runtime and call lua to process.
	// In here, just show log to test.
	exData = nullptr;
	exSize = 0;
	// process msg, get the md5 code and the swift number.
	int id = GetFreeLuaContext();
	if ( id < 0 )
	{
		res = "{\"errno\": \"-1\",\"errmsg\" : \"server is busy now. please try again.\"}";
		return true;
	}
	CLua* pLua = m_pLuaList[id];

	if (m_oReloadList[id] == true)
	{
		InitLua(pLua, m_pConfig->m_oLuaConfigInfo.ScriptFolder);
		m_oReloadList[id] = false;
	}

	CLuaPacket creq;
	creq.SetData("json_request_message", msg);
	CLuaPacket cres;
	bool bRes = pLua->RunFunction(m_pConfig->m_oLuaConfigInfo.ProcessFunction, pUInfo, &creq, &cres);
	m_oFreeLuaContext.push(id);
	if (bRes)
	{
		res = cres.GetData("json_response_message");
		string need = cres.GetData("NeedExData");
		if ( need == "true"	)
		{
			auto uuid = cres.GetData("ExDataUUID");
			auto len = cres.GetData("ExDataSize");
			auto pData = m_iData->GetTemp("SendList" + uuid);
			if (pData == nullptr)
			{
				res = FormatJSONErrorMessage("ExData no saved in DataCenter, The uuid is " + uuid,"-1");
				return true;
			}
			else
			{
				char* pBuf = TYPE_TRANS<char*>(pData);
				exData = pBuf;
				exSize = atoi(len.c_str());
			}
		}
		return true;
	}
	else
	{// 运行lua脚本失败
		return false;
	}
}
#define LUA_CONTEXT_WAIT_SECONDE  10
int Sloong::CLuaProcessCenter::GetFreeLuaContext()
{
	
	for ( int i = 0; i<LUA_CONTEXT_WAIT_SECONDE&&m_oFreeLuaContext.empty(); i++)
	{
		m_pLog->Debug("Wait lua context 1 sencond :"+CUniversal::ntos(i));
		m_oSSync.wait_for(1);
	}

	unique_lock<mutex> lck(m_oLuaContextMutex);
	if (m_oFreeLuaContext.empty())
	{
		m_pLog->Debug("no free context");
		return -1;
	}	
	int nID = m_oFreeLuaContext.front();
	m_oFreeLuaContext.pop();
	lck.unlock();
	return nID;
}

