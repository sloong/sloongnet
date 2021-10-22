/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2018-02-28 10:55:37
 * @LastEditTime: 2021-10-22 12:16:55
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/ControlHub.cpp
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


#include "ControlHub.h"
#include "events/NormalEvent.hpp"
using namespace Sloong::Events;

CResult Sloong::CControlHub::Initialize(int quantity, spdlog::logger* log)
{
	if (quantity < 1)
		return CResult::Make_Error("CControlHub work quantity must big than 0.");
	m_pLog = log;
	ThreadPool::AddWorkThread(std::bind(&CControlHub::MessageWorkLoop, this), quantity);
	TaskPool::Initialize(3);
	Run();
	return CResult::Succeed;
}

void Sloong::CControlHub::Exit()
{
	m_emStatus = RUN_STATUS::Exit;
	m_oSync.notify_all();
}

void *Sloong::CControlHub::Get(uint64_t key)
{
	auto baseitem = m_oDataList.try_get(key);
	if (baseitem == nullptr || (*baseitem)->Type != DataItemType::Object)
		return nullptr;

	auto item = dynamic_pointer_cast<ObjectData>(*baseitem);

	return item->Ptr;
}

string Sloong::CControlHub::GetTempString(const string &key, bool erase)
{
	auto baseitem = m_oTempDataList.try_get(key);
	if (baseitem == nullptr || (*baseitem)->Type != DataItemType::String)
		return string();

	auto item = dynamic_pointer_cast<StringData>(*baseitem);
	auto res = item->Data;
	if (erase)
		m_oTempDataList.erase(key);

	return res;
}

void *Sloong::CControlHub::GetTempObject(const string &key, int *out_size, bool erase)
{
	auto baseitem = m_oTempDataList.try_get(key);
	if (baseitem == nullptr || (*baseitem)->Type != DataItemType::Object)
		return nullptr;

	auto item = dynamic_pointer_cast<ObjectData>(*baseitem);
	if (out_size)
		*out_size = item->Size;
	auto ptr = item->Ptr;
	if (erase)
		m_oTempDataList.erase(key);

	return ptr;
}

unique_ptr<char[]> Sloong::CControlHub::GetTempBytes(const string &key, int *out_in_size)
{
	auto baseitem = m_oTempDataList.try_get(key);
	if (baseitem == nullptr || (*baseitem)->Type != DataItemType::Bytes)
		return nullptr;

	auto item = dynamic_pointer_cast<BytesData>(*baseitem);
	if (out_in_size)
		*out_in_size = item->Size;

	auto ptr = std::move(item->Ptr);
	m_oTempDataList.erase(key);
	return ptr;
}

shared_ptr<void> Sloong::CControlHub::GetTempSharedPtr(const string &key, bool erase)
{
	auto baseitem = m_oTempDataList.try_get(key);
	if (baseitem == nullptr || (*baseitem)->Type != DataItemType::SharedPtr)
		return nullptr;

	auto item = dynamic_pointer_cast<SharedPtrData>(*baseitem);
	auto ptr = item->Ptr;
	if (erase)
		m_oTempDataList.erase(key);

	return ptr;
}

void Sloong::CControlHub::SendMessage(int msgType)
{
	auto event = make_unique<NormalEvent>();
	event->SetEvent(msgType);
	m_oMsgList.push(std::move(event));

	m_oSync.notify_one();
}

void Sloong::CControlHub::SendMessage(SharedEvent evt)
{
	m_oMsgList.push(std::move(evt));
	m_oSync.notify_one();
}

/**
 * @Description: One message only have one handler. so cannot register handled message again.
 * @Params: 
 * @Return: 
 */
void Sloong::CControlHub::RegisterEventHandler(int t, MsgHandlerFunc func)
{
	if (!m_oMsgHandlerList.exist(t))
		m_oMsgHandlerList.insert(t, vector<MsgHandlerFunc>());

	m_oMsgHandlerList.get(t).push_back(func);
}

void Sloong::CControlHub::CallMessage(SharedEvent event)
{
	try
	{
		auto evt_type = event->GetEvent();
		auto handler_list = m_oMsgHandlerList.get(evt_type);
		auto handler_num = handler_list.size();
		if (handler_num == 0)
			return;

		for (auto func : handler_list)
		{
			func(event);
			if ( event->IsOneTimeEvent() )
				break;
		}
	}
	catch (const exception &ex)
	{
		cerr << "Exception happed in CallMessage." << ex.what() << endl;
	}
	catch (...)
	{
		cerr << "Unhandle exception in CallMessage function." << endl;
	}
}

void Sloong::CControlHub::MessageWorkLoop()
{
	auto pid = this_thread::get_id();
	string spid = Helper::ntos(pid);
	
	m_pLog->info("Control hub work thread is running. PID:" + spid);

	while (m_emStatus == RUN_STATUS::Created)
	{
		this_thread::sleep_for(std::chrono::microseconds(10));
	}

	while (m_emStatus != RUN_STATUS::Exit)
	{
		try
		{
			if (m_oMsgList.empty())
			{
				m_oSync.wait_for(100);
				continue;
			}

			auto event = m_oMsgList.pop(nullptr);
			if (event != nullptr)
			{
				// Get the message handler list.
				CallMessage(event);
			}
		}
		catch (...)
		{
			m_pLog->critical( "Unhandle exception in CControlHub work loop.");
		}
	}
	m_pLog->info(format( "Control hub work thread[{}] is exit " ,spid));
}
