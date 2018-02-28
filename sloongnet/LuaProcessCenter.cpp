#include "LuaProcessCenter.h"
#include "main.h"
#include <univ/luapacket.h>
#include <univ/lua.h>
#include "serverconfig.h"
#include "globalfunction.h"
using namespace Sloong;

CLog* Sloong::CLuaProcessCenter::g_pLog = nullptr;

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

	g_pLog = TYPE_TRANS<CLog*>(m_iData->Get(DATA_ITEM::Logger));
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
	int num = 10;
	for (int i = 0; i < num; i++)
		NewThreadInit();

}


void Sloong::CLuaProcessCenter::HandleError(string err)
{
	g_pLog->Error(CUniversal::Format("[Script]:[%s]", err));
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
	}
	SAFE_RELEASE_EVENT(ev);
}

void Sloong::CLuaProcessCenter::ReloadContext()
{
	int n = m_pLuaList.size();
	for (int i=0;i<n;i++)
	{
		m_oReloadList.push_back(true);
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
	m_oFreeLuaContext.push(m_pLuaList.size() - 1);
	int id = m_pLuaList.size() - 1;
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




bool Sloong::CLuaProcessCenter::MsgProcess(CLuaPacket * pUInfo, string & msg, string & res, string& exData, int& exSize)
//int Sloong::CLuaProcessCenter::MsgProcess(IEvent* evt)
{
	
	// In process, need add the lua script runtime and call lua to process.
	// In here, just show log to test.

	// process msg, get the md5 code and the swift number.
	int id = GetFreeLuaContext();
	CLua* pLua = m_pLuaList[id];

	if (m_oReloadList[id] == true)
	{
		InitLua(pLua, m_pConfig->m_oLuaConfigInfo.ScriptFolder);
		m_oReloadList[id] = false;
	}
	//CNormalEvent* msg_event = EVENT_TRANS<CNormalEvent*>(evt);
	CLuaPacket creq;
	creq.SetData("jreq", msg);//msg_event->GetMessage());
	CLuaPacket cres;
	if (pLua->RunFunction(m_pConfig->m_oLuaConfigInfo.ProcessFunction, pUInfo, &creq, &cres))
	{
		exData.clear();
		string need = cres.GetData("NeedExData");
		if ( need == "true"	)
		{
			exData = cres.GetData("ExDataUUID");
			exSize = atoi( cres.GetData("ExDataSize").c_str());
		}
		return true;
	}
	else
	{// 运行lua脚本失败
		return false;
	}
	/*
	int nRes = pLua->RunFunction(m_pLuaConfig->ProcessFunction, pUInfo, msg, res);
	if (nRes >= 0)
	{	
		// 对于发送扩展数据，这里模式仍采用同模式，但是实现方式需要改掉
		// 这里将不再直接访问GFunc，而是在lua中准备数据的时候，由GFunc直接将数据存储到iData中，
		// 这里直接去iData中去取相应的数据。
		if (nRes >= (int)m_pGFunc->m_oSendExMapList.size())
		{
			m_pLog->Warn(CUniversal::Format("Call function end, but the res is error: res [%d], SendMapList size[%d]", nRes, m_pGFunc->m_oSendExMapList.size()));
			return 0;
		}
		pBuf = m_pGFunc->m_oSendExMapList[nRes].m_pData;
		int nSize = m_pGFunc->m_oSendExMapList[nRes].m_nDataSize;
		m_pLog->Verbos(CUniversal::Format("Send Ex Data, Size[%d], Message[%s]", nSize, msg.c_str()));
		unique_lock<mutex> lck(m_pGFunc->m_oListMutex);
		m_pGFunc->m_oSendExMapList[nRes].m_pData = NULL;
		m_pGFunc ->m_oSendExMapList[nRes].m_nDataSize = 0;
		m_pGFunc->m_oSendExMapList[nRes].m_bIsEmpty = true;
		lck.unlock();
		nRes = nSize;
	}
	else
	{
		m_pLog->Verbos(res);
		nRes = 0;
	}
	m_oFreeLuaContext.push(id);
	return nRes;*/
}
#define LUA_CONTEXT_WAIT_SECONDE  10
int Sloong::CLuaProcessCenter::GetFreeLuaContext()
{
	for ( int i = 0; i<LUA_CONTEXT_WAIT_SECONDE&&m_oFreeLuaContext.empty(); i++)
	{
		m_oSSync.wait_for(1);
	}

	unique_lock<mutex> lck(m_oLuaContextMutex);
	if (m_oFreeLuaContext.empty())
	{
		return -1;
	}
	int nID = m_oFreeLuaContext.front();
	m_oFreeLuaContext.pop();
	lck.unlock();
	return nID;
}

