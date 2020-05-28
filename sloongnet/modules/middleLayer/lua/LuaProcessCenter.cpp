#include "LuaProcessCenter.h"
#include "globalfunction.h"
#include "IData.h"
using namespace Sloong;

CLog *g_pLog = nullptr;

CLuaProcessCenter::CLuaProcessCenter()
{
	m_pGFunc = make_unique<CGlobalFunction>();
}

CLuaProcessCenter::~CLuaProcessCenter()
{
	size_t nLen = m_pLuaList.size();
	for (size_t i = 0; i < nLen; i++)
	{
		SAFE_DELETE(m_pLuaList[i]);
	}
}

void Sloong::CLuaProcessCenter::Initialize(IControl *iMsg)
{
	IObject::Initialize(iMsg);
	g_pLog = m_pLog;

	m_pGFunc->Initialize(m_iC);
	m_pConfig = IData::GetModuleConfig();

	m_iC->RegisterEvent(EVENT_TYPE::ReloadLuaContext);
	m_iC->RegisterEventHandler(ReloadLuaContext, std::bind(&CLuaProcessCenter::ReloadContext, this, std::placeholders::_1));
	// 主要的循环方式为，根据输入的处理数来初始化指定数量的lua环境。
	// 然后将其加入到可用队列
	// 在处理开始之前根据队列情况拿到某lua环境的id并将其移除出可用队列
	// 在处理完毕之后重新加回到可用队列中。
	// 这里使用处理线程池的数量进行初始化，保证在所有线程都在处理Lua请求时不会因luacontext发生堵塞
	for (int i = 0; i < m_pConfig->operator[]("LuaContextQuantity").asInt(); i++)
		NewThreadInit();
}

void Sloong::CLuaProcessCenter::HandleError(const string &err)
{
	g_pLog->Error(Helper::Format("[Script]:[%s]", err.c_str()));
}

void Sloong::CLuaProcessCenter::ReloadContext(IEvent *event)
{
	size_t n = m_pLuaList.size();
	for (size_t i = 0; i < n; i++)
	{
		m_oReloadList[i] = true;
	}
}

int Sloong::CLuaProcessCenter::NewThreadInit()
{
	CLua *pLua = new CLua();
	pLua->SetErrorHandle(HandleError);
	pLua->SetScriptFolder(m_pConfig->operator[]("LuaScriptFolder").asString());
	m_pGFunc->RegistFuncToLua(pLua);
	InitLua(pLua, m_pConfig->operator[]("LuaScriptFolder").asString());
	m_pLuaList.push_back(pLua);
	m_oReloadList.push_back(false);
	int id = (int)m_pLuaList.size() - 1;
	FreeLuaContext(id);
	return id;
}

void Sloong::CLuaProcessCenter::InitLua(CLua *pLua, string folder)
{
	if (!pLua->RunScript(m_pConfig->operator[]("LuaEntryFile").asString()))
	{
		throw normal_except("Run Script Fialed.");
	}
	char tag = folder[folder.length() - 1];
	if (tag != '/' && tag != '\\')
	{
		folder += '/';
	}
	pLua->RunFunction(m_pConfig->operator[]("LuaEntryFunction").asString(), Helper::Format("'%s'", folder.c_str()));
}

void Sloong::CLuaProcessCenter::CloseSocket(CLuaPacket *uinfo)
{
	// call close function.
	int id = GetFreeLuaContext();
	CLua *pLua = m_pLuaList[id];
	pLua->RunFunction(m_pConfig->operator[]("LuaSocketCloseFunction").asString(), uinfo);
	FreeLuaContext(id);
}

CResult Sloong::CLuaProcessCenter::MsgProcess(int function, CLuaPacket *pUInfo, const string &msg, const string &extend)
{
	int id = GetFreeLuaContext();
	if (id < 0)
		return CResult::Make_Error("server is busy now. please try again.");
	try
	{
		CLua *pLua = m_pLuaList[id];
		if (m_oReloadList[id] == true)
		{
			InitLua(pLua, m_pConfig->operator[]("LuaScriptFolder").asString());
			m_oReloadList[id] = false;
		}
		CLuaPacket creq;
		creq.SetData("request_message", msg);
		if (extend.length() > 0)
			creq.SetData("request_extend", extend);
		CLuaPacket cres;
		if (pLua->RunFunction(m_pConfig->operator[]("LuaProcessFunction").asString(), pUInfo, &creq, &cres))
		{
			FreeLuaContext(id);
			auto str_res = cres.GetData("response_result", "");
			int res;
			if ( !ConvertStrToInt(str_res,&res)||!ResultType_IsValid(res) )
			{
				return CResult::Make_Error("Get result fialed " + str_res);
			}
			auto res_msg = cres.GetData("response_message", "");

			return CResult((ResultType)res, res_msg);
		}
		else
		{
			FreeLuaContext(id);
			return CResult::Make_Error("server process happened error.");
		}
	}
	catch (const exception& ex)
	{
		FreeLuaContext(id);
		return CResult::Make_Error("server process error."+string(ex.what()));
	}
	catch (...)
	{
		FreeLuaContext(id);
		return CResult::Make_Error("server process error.");
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
