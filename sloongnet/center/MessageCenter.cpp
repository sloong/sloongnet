#include "MessageCenter.h"
#include "NormalEvent.h"

using namespace Sloong;
using namespace Sloong::Events;

CMessageCenter::CMessageCenter()
{
}


CMessageCenter::~CMessageCenter()
{
}

void CMessageCenter::Initialize(IData* iData, int nWorkThreadNum)
{
	m_iData = iData;
	CThreadPool::AddWorkThread(std::bind(&CMessageCenter::MessageWorkLoop, this, std::placeholders::_1), nullptr, nWorkThreadNum);
}

void CMessageCenter::SendMessage(MSG_TYPE msgType)
{
	auto evt = make_shared<CNormalEvent>();
	evt->SetEvent(msgType);
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oSync.notify_one();
}

void CMessageCenter::SendMessage(SmartEvent evt)
{
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oSync.notify_one();
}


void CMessageCenter::RegisterEvent(MSG_TYPE t)
{
	m_oMsgHandlerList[t] = vector<MsgHandlerFunc>();
}

/**
 * @Remarks: One message only have one handler. so cannot register handled message again.
 * @Params: 
 * @Return: 
 */
void CMessageCenter::RegisterEventHandler(MSG_TYPE t, MsgHandlerFunc func)
{
	if (m_oMsgHandlerList.find(t) == m_oMsgHandlerList.end())
	{
		throw normal_except("Target event is not regist.");
	}
	else{
		m_oMsgHandlerList[t].push_back(func);
	}
}

void CMessageCenter::Run()
{
	m_emStatus = RUN_STATUS::Running;
}

void CMessageCenter::Exit()
{
	m_emStatus = RUN_STATUS::Exit;
}

void CMessageCenter::MessageWorkLoop(SMARTER param)
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


