
#include "ControlHub.h"
#include "univ/exception.h"
#include "NormalEvent.hpp"
using namespace Sloong::Events;

CResult Sloong::CControlHub::Initialize(int quantity)
{
	if (quantity < 1)
		return CResult::Make_Error("CControlHub work quantity must big than 0.");
	CThreadPool::AddWorkThread(std::bind(&CControlHub::MessageWorkLoop, this), quantity);
	CThreadPool::Run();
	Run();
	return CResult::Succeed();
}

void Sloong::CControlHub::Exit()
{
	m_emStatus = RUN_STATUS::Exit;
	m_listMsgHook.clear();
	m_oMsgHandlerList.clear();
}

void *Sloong::CControlHub::Get(DATA_ITEM item)
{
	auto data = m_oDataList.find(item);
	if (data == m_oDataList.end())
		return nullptr;

	return (*data).second;
}

string Sloong::CControlHub::GetTempString(const string &key)
{
	auto item = m_oTempStringList.find(key);
	if (item == m_oTempStringList.end())
		return string();

	auto res = (*item).second;
	m_oTempStringList.erase(key);
	return res;
}

void *Sloong::CControlHub::GetTempObject(const string &name, int *out_size)
{
	auto it = m_oTempObjectList.find(name);
	if (it == m_oTempObjectList.end())
		return nullptr;

	auto item = (*it).second;
	if (out_size)
		*out_size = item.size;
	auto ptr = item.ptr;
	m_oTempObjectList.erase(name);
	return ptr;
}

unique_ptr<char[]> Sloong::CControlHub::GetTempBytes(const string &name, int *out_in_size)
{
	auto it = m_oTempBytesList.find(name);
	if (it == m_oTempBytesList.end())
		return nullptr;

	auto &item = (*it).second;

	if (out_in_size)
		*out_in_size = item.size;

	auto ptr = std::move(item.ptr);
	m_oTempBytesList.erase(name);
	return ptr;
}

void Sloong::CControlHub::SendMessage(EVENT_TYPE msgType)
{
	auto event = make_unique<CNormalEvent>();
	event->SetEvent(msgType);
	m_oMsgList.push_move(std::move(event));

	m_oSync.notify_one();
}

void Sloong::CControlHub::SendMessage(UniqueEvent evt)
{
	m_oMsgList.push_move(std::move(evt));
	m_oSync.notify_one();
}

/**
 * @Remarks: One message only have one handler. so cannot register handled message again.
 * @Params: 
 * @Return: 
 */
void Sloong::CControlHub::RegisterEventHandler(EVENT_TYPE t, MsgHandlerFunc func)
{
	if (m_oMsgHandlerList.find(t) == m_oMsgHandlerList.end())
	{
		throw normal_except("Target event is not regist.");
	}
	else
	{
		m_oMsgHandlerList[t].push_back(func);
	}
}

void Sloong::CControlHub::CallMessage(UniqueEvent event)
{
	try
	{
		auto evt_type = event->GetEvent();
		auto handler_list = m_oMsgHandlerList[evt_type];
		auto handler_num = handler_list.size();
		if (handler_num == 0)
			return;

		for (auto func : handler_list)
			func(event.get());

		if (m_listMsgHook.exist(evt_type))
			m_listMsgHook[evt_type](std::move(event));

		event = nullptr;
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
	UniqueEvent event = nullptr;
	while (m_emStatus != RUN_STATUS::Exit)
	{
		try
		{
			if (m_emStatus == RUN_STATUS::Created)
			{
				this_thread::sleep_for(std::chrono::microseconds(100));
				continue;
			}
			if (m_oMsgList.empty())
			{
				m_oSync.wait_for(1000);
				continue;
			}

			if (m_oMsgList.TryMovePop(event) && event != nullptr)
			{
				// Get the message handler list.
				CallMessage(std::move(event));
			}
		}
		catch (...)
		{
			cerr << "Unhandle exception in CControlHub work loop." << endl;
		}
	}
}
