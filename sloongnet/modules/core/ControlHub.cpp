
#include "ControlHub.h"
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

void Sloong::CControlHub::Run()
{
	m_emStatus = RUN_STATUS::Running;
}

void Sloong::CControlHub::Exit()
{
	m_emStatus = RUN_STATUS::Exit;
}

bool Sloong::CControlHub::Add(DATA_ITEM item, void *object)
{
	auto it = m_oDataList.find(item);
	if (it != m_oDataList.end())
		return false;

	m_oDataList.insert(make_pair(item, object));
	return false;
}

void *Sloong::CControlHub::Get(DATA_ITEM item)
{
	auto data = m_oDataList.find(item);
	if (data == m_oDataList.end())
		return nullptr;

	return (*data).second;
}

bool Sloong::CControlHub::Remove(DATA_ITEM item)
{
	auto it = m_oDataList.find(item);
	if (it == m_oDataList.end())
		return false;

	m_oDataList.erase(item);
	return true;
}

bool Sloong::CControlHub::AddTemp(const string &name, void *object)
{
	auto it = m_oTempDataList.find(name);
	if (it != m_oTempDataList.end())
		return false;

	m_oTempDataList[name] = object;
	return true;
}

void *Sloong::CControlHub::GetTemp(const string &name)
{
	auto item = m_oTempDataList.find(name);
	if (item == m_oTempDataList.end())
		return nullptr;

	m_oTempDataList.erase(name);
	return (*item).second;
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

void Sloong::CControlHub::RegisterEvent(EVENT_TYPE t)
{
	m_oMsgHandlerList[t] = vector<MsgHandlerFunc>();
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

void Sloong::CControlHub::RegisterEventHook(EVENT_TYPE t, MsgHookFunc func)
{
	m_listMsgHook[t] = func;
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

		if (m_listMsgHook[event->GetEvent()] != nullptr)
			m_listMsgHook[event->GetEvent()](std::move(event));

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
				//usleep(990000);
				continue;
			}
			if (m_oMsgList.empty())
			{
				m_oSync.wait_for(1);
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
