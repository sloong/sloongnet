/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2018-02-28 10:55:37
 * @LastEditTime: 2021-02-26 11:28:47
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/middleLayer/lua/LuaProcessCenter.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

#include "LuaProcessCenter.h"
#include "globalfunction.h"
#include "luaMiddleLayer.h"
#include "IData.h"
#include "events/LuaEvent.hpp"
using namespace Sloong;
using namespace Sloong::Events;

static const int MAX_TRY_NUM = 99;

CResult Sloong::CLuaProcessCenter::Initialize(IControl *iMsg)
{
	IObject::Initialize(iMsg);

	m_pConfig = IData::GetModuleConfig();

	// 主要的循环方式为，根据输入的处理数来初始化指定数量的lua环境。
	// 然后将其加入到可用队列
	// 在处理开始之前根据队列情况拿到某lua环境的id并将其移除出可用队列
	// 在处理完毕之后重新加回到可用队列中。
	// 这里使用处理线程池的数量进行初始化，保证在所有线程都在处理Lua请求时不会因luacontext发生堵塞
	auto num = m_pConfig->operator[]("LuaContextQuantity").asInt();
	if (num < 1)
		return CResult::Make_Error("LuaContextQuantity must be bigger than 0,");

	for (int i = 0; i < num; i++)
	{
		auto res = NewThreadInit();
		if (res.IsFialed())
			return res;
	}

	m_iC->RegisterEventHandler(LUA_EVENT_TYPE::ProcessLuaEvent, std::bind(&CLuaProcessCenter::OnProcessLuaEvent, this, std::placeholders::_1));
	
	return CResult::Succeed;
}

void Sloong::CLuaProcessCenter::OnProcessLuaEvent(SharedEvent e)
{
	auto event = EVENT_TRANS<LuaEvent>(e);
	int id = GetFreeLuaContext(MAX_TRY_NUM);
	auto pLua = m_listLuaContent[id]->Content.get();
	pLua->RunEventFunction(m_pConfig->operator[]("LuaEventFunction").asString(), event->GetLuaEvent(), event->GetLuaEventParams() );
	FreeLuaContext(id);
}

void Sloong::CLuaProcessCenter::ReloadContext()
{
	for (auto &i : m_listLuaContent)
	{
		i->Reload.store(true);
	}
}

CResult Sloong::CLuaProcessCenter::NewThreadInit()
{
	auto c = InitLua();
	if (c.IsFialed())
		return move(c);
		
	auto lua = make_unique<LuaContent>();
	lua->Content = c.MoveResultObject();
	lua->Reload.store(false);

	lua->Content->EnableLog(m_pLog);
	
	m_listLuaContent.push_back(move(lua));
	int id = (int)m_listLuaContent.size() - 1;
	FreeLuaContext(id);
	return CResult::Succeed;
}

TResult<unique_ptr<CLua>> Sloong::CLuaProcessCenter::InitLua()
{
	auto lua = make_unique<CLua>();
	auto folder = m_pConfig->operator[]("LuaScriptFolder").asString();
	m_pLog->Info("Init lua base on folder : " + folder);
	lua->SetScriptFolder(folder);
	
	CGlobalFunction::Instance->RegistFuncToLua(lua.get());
	
	auto res = lua->RunScript(m_pConfig->operator[]("LuaEntryFile").asString());
	if (res.IsFialed())
	{
		return TResult<unique_ptr<CLua>>::Make_Error("Run Script Fialed." + res.GetMessage());
	}
	res = lua->RunFunction(m_pConfig->operator[]("LuaEntryFunction").asString(), Helper::Format("'%s'", m_pConfig->operator[]("LuaScriptFolder").asString().c_str()));
	if (res.IsFialed())
	{
		return TResult<unique_ptr<CLua>>::Make_Error("Run Function Fialed." + res.GetMessage());
	}
	return TResult<unique_ptr<CLua>>::Make_OKResult(move(lua));
}

void Sloong::CLuaProcessCenter::CloseSocket(CLuaPacket *uinfo)
{
	// call close function.
	int id = GetFreeLuaContext(MAX_TRY_NUM);
	auto pLua = m_listLuaContent[id]->Content.get();
	pLua->RunFunction(m_pConfig->operator[]("LuaSocketCloseFunction").asString(), uinfo, 0, "", "");
	FreeLuaContext(id);
}

SResult Sloong::CLuaProcessCenter::MsgProcess(int function, CLuaPacket *pUInfo, const string &msg, const string &extend)
{
	int id = GetFreeLuaContext();
	if (id < 0)
		return SResult::Make_Error("server is busy now. please try again.");
	try
	{
		auto &content = m_listLuaContent[id];
		if (content->Reload.load())
		{
			auto c = InitLua();
			if( c.IsFialed() )
				return SResult::Make_Error("Error when reload script: " + c.GetMessage());
			content->Content = c.MoveResultObject();
			content->Reload.store(false);
		}
		string extendUUID("");
		auto res = content->Content->RunFunction(m_pConfig->operator[]("LuaProcessFunction").asString(), pUInfo, function, msg, extend, &extendUUID);
		FreeLuaContext(id);
		if (res.IsFialed())
			return SResult::Make_Error(res.GetMessage());
		else
			return SResult::Make_OKResult(extendUUID, res.GetMessage());
	} 
	catch (const exception &ex)
	{
		FreeLuaContext(id);
		return SResult::Make_Error("Server process error." + string(ex.what()));
	}
	catch (...)
	{
		FreeLuaContext(id);
		return SResult::Make_Error("Server process error. Unexpected exceptions happened.");
	}
}

int Sloong::CLuaProcessCenter::GetFreeLuaContext(int try_num)
{
	for (int i = 0; i < try_num && m_oFreeLuaContext.empty(); i++)
	{
		m_pLog->Debug("Wait lua context 1 sencond :" + Helper::ntos(i));
		m_oSSync.wait_for(500);
	}

	if (m_oFreeLuaContext.empty())
	{
		m_pLog->Error("no free context");
		return -1;
	}
	int nID = m_oFreeLuaContext.pop(-1);
	return nID;
}
