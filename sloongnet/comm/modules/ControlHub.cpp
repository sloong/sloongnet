
#include "ControlHub.h"
#include "NormalEvent.hpp"
using namespace Sloong::Events;


bool Sloong::CControlHub::Initialize(int quantity)
{
	CThreadPool::AddWorkThread(std::bind(&CControlHub::MessageWorkLoop, this, std::placeholders::_1), nullptr, quantity);
	CThreadPool::Run();
	Run();
    return true;
}

void Sloong::CControlHub::Run()
{
	m_emStatus = RUN_STATUS::Running;
}

void Sloong::CControlHub::Exit()
{
	m_emStatus = RUN_STATUS::Exit;
}

bool Sloong::CControlHub::Add(DATA_ITEM item, void * object)
{
	auto it = m_oDataList.find(item);
	if (it != m_oDataList.end())
	{
		return false;
	}
	m_oDataList.insert(make_pair(item, object));
	return false;
}

void * Sloong::CControlHub::Get(DATA_ITEM item)
{
	auto data = m_oDataList.find(item);
	if (data == m_oDataList.end())
	{
		return nullptr;
	}
	return (*data).second;
}

bool Sloong::CControlHub::Remove(DATA_ITEM item)
{
	m_oDataList.erase(item);
}

bool Sloong::CControlHub::AddTemp(string name, void * object)
{
	m_oTempDataList[name] = object;
	return true;
}

void * Sloong::CControlHub::GetTemp(string name)
{
	auto item = m_oTempDataList.find(name);
	if (item == m_oTempDataList.end())
	{
		return nullptr;
	}
	m_oTempDataList.erase(name);
	return (*item).second;
}



void Sloong::CControlHub::SendMessage(EVENT_TYPE msgType)
{
	auto evt = make_shared<CNormalEvent>();
	evt->SetEvent(msgType);
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oSync.notify_one();
}

void Sloong::CControlHub::SendMessage(SmartEvent evt)
{
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
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
	else{
		m_oMsgHandlerList[t].push_back(func);
	}
}


void Sloong::CControlHub::MessageWorkLoop(SMARTER param)
{
	while (m_emStatus != RUN_STATUS::Exit)
	{
		try
		{
			if (m_emStatus == RUN_STATUS::Created)
			{
				SLEEP(100);
				continue;
			}
			if (m_oMsgList.empty())
			{
				m_oSync.wait_for(1);
				continue;
			}
			if (!m_oMsgList.empty())
			{
				unique_lock<mutex> lck(m_oMsgListMutex);
				if (m_oMsgList.empty())
				{
					lck.unlock();
					continue;
				}

				auto p = m_oMsgList.front();
				m_oMsgList.pop();
				lck.unlock();

				// Get the message handler list.
				auto evt_type = p->GetEvent();
				auto handler_list = m_oMsgHandlerList[evt_type];
				int handler_num = handler_list.size();
				if ( handler_num == 0 )
					continue;

				for (int i = 0; i < handler_num; i++)
				{
					auto func = handler_list[i];
					func(p);
				}
			}
		}
		catch (...)
		{
			cerr << "Unhandle exception in MessageCenter work loop." << endl;
		}
		
	}
}

