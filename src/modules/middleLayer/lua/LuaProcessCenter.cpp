#include "LuaProcessCenter.h"
#include "globalfunction.h"
#include "IData.h"
using namespace Sloong;

CLuaProcessCenter::~CLuaProcessCenter()
{
	size_t nLen = m_pLuaList.size();
	for (size_t i = 0; i < nLen; i++)
	{
		SAFE_DELETE(m_pLuaList[i]);
	}
}

CResult Sloong::CLuaProcessCenter::Initialize(IControl *iMsg)
{
	IObject::Initialize(iMsg);
	 
	CGlobalFunction::Instance->Initialize(m_iC);
	m_pConfig = IData::GetModuleConfig();

	m_iC->RegisterEvent(EVENT_TYPE::ReloadLuaContext);
	m_iC->RegisterEventHandler(ReloadLuaContext, std::bind(&CLuaProcessCenter::ReloadContext, this, std::placeholders::_1));
	// 主要的循环方式为，根据输入的处理数来初始化指定数量的lua环境。
	// 然后将其加入到可用队列
	// 在处理开始之前根据队列情况拿到某lua环境的id并将其移除出可用队列
	// 在处理完毕之后重新加回到可用队列中。
	// 这里使用处理线程池的数量进行初始化，保证在所有线程都在处理Lua请求时不会因luacontext发生堵塞
	auto num = m_pConfig->operator[]("LuaContextQuantity").asInt();
	if( num < 1 )
		return CResult::Make_Error("LuaContextQuantity must be bigger than 0,");

	for (int i = 0; i < num; i++)
	{
		auto res = NewThreadInit();
		if( res.IsFialed() )
			return res;
	}
	return CResult::Succeed();
}

void Sloong::CLuaProcessCenter::HandleError(const string &err)
{
	m_pLog->Error(Helper::Format("[Script]:[%s]", err.c_str()));
}

void Sloong::CLuaProcessCenter::ReloadContext(IEvent *event)
{
	size_t n = m_pLuaList.size();
	for (size_t i = 0; i < n; i++)
	{
		m_oReloadList[i] = true;
	}
}

CResult Sloong::CLuaProcessCenter::NewThreadInit()
{
	CLua *pLua = new CLua();
	pLua->SetErrorHandle(std::bind(&CLuaProcessCenter::HandleError,this,placeholders::_1));
	pLua->SetScriptFolder(m_pConfig->operator[]("LuaScriptFolder").asString());
	CGlobalFunction::Instance->RegistFuncToLua(pLua);
	auto res = InitLua(pLua, m_pConfig->operator[]("LuaScriptFolder").asString());
	if( res.IsFialed() )
		return res;
	m_pLuaList.push_back(pLua);
	m_oReloadList.push_back(false);
	int id = (int)m_pLuaList.size() - 1;
	FreeLuaContext(id);
	return CResult::Succeed();
}

CResult Sloong::CLuaProcessCenter::InitLua(CLua *pLua, string folder)
{
	if (!pLua->RunScript(m_pConfig->operator[]("LuaEntryFile").asString()))
	{
		return CResult::Make_Error("Run Script Fialed.");
	}
	if(!pLua->RunFunction(m_pConfig->operator[]("LuaEntryFunction").asString(), Helper::Format("'%s'", folder.c_str())))
	{
		return CResult::Make_Error("Run Function Fialed.");
	}
	return CResult::Succeed();
}

void Sloong::CLuaProcessCenter::CloseSocket(CLuaPacket *uinfo)
{
	// call close function.
	int id = GetFreeLuaContext();
	CLua *pLua = m_pLuaList[id];
	pLua->RunFunction(m_pConfig->operator[]("LuaSocketCloseFunction").asString(), uinfo, 0, "", "" );
	FreeLuaContext(id);
}

SResult Sloong::CLuaProcessCenter::MsgProcess(int function, CLuaPacket *pUInfo, const string &msg, const string &extend)
{
	int id = GetFreeLuaContext();
	if (id < 0)
		return SResult::Make_Error("server is busy now. please try again.");
	try
	{
		CLua *pLua = m_pLuaList[id];
		if (m_oReloadList[id] == true)
		{
			InitLua(pLua, m_pConfig->operator[]("LuaScriptFolder").asString());
			m_oReloadList[id] = false;
		}
		string extendUUID("");
		auto res = pLua->RunFunction(m_pConfig->operator[]("LuaProcessFunction").asString(), pUInfo, function, msg, extend, &extendUUID);
		FreeLuaContext(id);
		if( res.IsFialed() )
			return SResult::Make_Error(res.GetMessage());
		else
			return SResult::Make_OK(extendUUID,res.GetMessage());
	}
	catch (const exception& ex)
	{
		FreeLuaContext(id);
		return SResult::Make_Error("server process error."+string(ex.what()));
	}
	catch (...)
	{
		FreeLuaContext(id);
		return SResult::Make_Error("server process error.");
	}
}
#define LUA_CONTEXT_WAIT_SECONDE 10
int Sloong::CLuaProcessCenter::GetFreeLuaContext()
{

	for (int i = 0; i < LUA_CONTEXT_WAIT_SECONDE && m_oFreeLuaContext.empty(); i++)
	{
		m_pLog->Debug("Wait lua context 1 sencond :" + Helper::ntos(i));
		m_oSSync.wait_for(500);
	}

	if (m_oFreeLuaContext.empty())
	{
		m_pLog->Debug("no free context");
		return -1;
	}
	int nID = m_oFreeLuaContext.front();
	m_oFreeLuaContext.pop();
	return nID;
}
