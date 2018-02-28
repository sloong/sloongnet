#include "MessageCenter.h"

#include "NormalEvent.h"

using namespace Sloong;
using namespace Sloong::Universal;
using namespace Sloong::Events;


CMessageCenter::CMessageCenter()
{
}


CMessageCenter::~CMessageCenter()
{
}

void CMessageCenter::Initialize(int nWorkLoopNum, int ThreadPoolNum)
{
	CThreadPool::Initialize(ThreadPoolNum);
	CThreadPool::AddWorkThread(MessageWorkLoop, this, nWorkLoopNum);
}

void CMessageCenter::SendMessage(MSG_TYPE msgType)
{
	CNormalEvent* evt = new CNormalEvent();
	evt->SetEvent(msgType);
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oWrokLoopCV.notify_one();
}

void CMessageCenter::SendMessage(IEvent * evt)
{
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oWrokLoopCV.notify_one();
}

void CMessageCenter::CallMessage(MSG_TYPE msgType, void * msgParams)
{
}

void CMessageCenter::RegisterEvent(MSG_TYPE t)
{
	m_oMsgHandlerList[t] = vector<HandlerItem>();
}

void CMessageCenter::RegisterEventHandler(MSG_TYPE t, void* object, LPCALLBACK2FUNC func)
{
	if (m_oMsgHandlerList.find(t) == m_oMsgHandlerList.end())
	{
		throw normal_except("Target event is not regist.");
	}
	else
	{
		HandlerItem item;
		item.handler = func;
		item.object = object;
		m_oMsgHandlerList[t].push_back(item);
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

void * CMessageCenter::MessageWorkLoop(void * param)
{
	CMessageCenter* pThis = (CMessageCenter*)param;
	unique_lock<mutex> work_lck(pThis->m_oWorkLoopMutex);
	while (pThis->m_emStatus != RUN_STATUS::Exit)
	{
		try
		{
			if (pThis->m_emStatus == RUN_STATUS::Created)
			{
				SLEEP(100);
				continue;
			}
			if (pThis->m_oMsgList.empty())
			{
				pThis->m_oWrokLoopCV.wait(work_lck);
				continue;
			}
			if (!pThis->m_oMsgList.empty())
			{
				unique_lock<mutex> lck(pThis->m_oMsgListMutex);
				if (pThis->m_oMsgList.empty())
				{
					lck.unlock();
					continue;
				}

				auto p = pThis->m_oMsgList.front();
				pThis->m_oMsgList.pop();
				lck.unlock();

				// Get the message handler list.
				auto evt_type = p->GetEvent();
				auto handler_list = pThis->m_oMsgHandlerList[evt_type];
				int handler_num = handler_list.size();
				if ( handler_num == 0 )
					continue;

				// 由于涉及到多处理函数，而且释放必须由最后一个使用者来释放
				// 所以在这里，将其加入到处理队列前，自动调用AddRef函数来增加计数。
				// 由于担心处理函数运行太快，导致下次尚未加入到队列中，前一次已经运行结束直接将Event对象删除。
				// 所以在这里一次性将引用计数根据队列大小增加完毕。
				p->AddRef(handler_num);
				for (int i = 0; i < handler_num; i++)
				{
					auto item = handler_list[i];
					// Use the threadpool to do the message 	
					CThreadPool::EnqueTask2(item.handler, p, item.object);
				}
			}
		}
		catch (...)
		{
			cerr << "Unhandle exception in MessageCenter work loop." << endl;
		}
		
	}

	return nullptr;
}
