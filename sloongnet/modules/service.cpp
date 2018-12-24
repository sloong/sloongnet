/* File Name: server.c */
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
using namespace std;
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "serverconfig.h"
#include "CmdProcess.h"
#include "service.h"
#include "ControlCenter.h"
#include "NormalEvent.h"
#include "IData.h"

using namespace Sloong::Events;

IControl* Sloong::IData::m_iC = nullptr;

SloongNetService::SloongNetService()
{
	m_pLog = make_unique<CLog>();
    m_pCC = make_unique<CControlCenter>();
}

SloongNetService::~SloongNetService()
{
	Exit();
	CThreadPool::Exit();
    m_pLog->End();
}


void SloongNetService::Initialize(CServerConfig* config)
{
	LOGTYPE oType = LOGTYPE::ONEFILE;
	if (!config->m_oLogInfo.LogWriteToOneFile)
	{
		oType = LOGTYPE::DAY;
	}
	m_pLog->Initialize(config->m_oLogInfo.LogPath, config->m_oLogInfo.DebugMode, LOGLEVEL(config->m_oLogInfo.LogLevel), oType);
	if (config->m_oLogInfo.NetworkPort != 0)
		m_pLog->EnableNetworkLog(config->m_oLogInfo.NetworkPort);
		
	Add(Configuation, config);
	Add(Logger, m_pLog.get());
	
	CThreadPool::AddWorkThread(std::bind(&SloongNetService::MessageWorkLoop, this, std::placeholders::_1), nullptr, config->m_nMessageCenterThreadQuantity);
	
	RegisterEvent(ProgramExit);
	RegisterEvent(ProgramStart);
	RegisterEventHandler(MSG_TYPE::ProgramExit, std::bind(&SloongNetService::ExitEventHandler, this, std::placeholders::_1));

	try{
		IData::Initialize(this);
		m_pCC->Initialize(this);
	}
	catch(exception e)
	{
		m_pLog->Error(string("Excepiton happened in initialize for ControlCenter. Message:")+  string(e.what()));
	}
}

void SloongNetService::Run()
{
	m_emStatus = RUN_STATUS::Running;
	CThreadPool::Run();
	SendMessage(MSG_TYPE::ProgramStart);
	while (m_emStatus != RUN_STATUS::Exit)
	{
		m_oSync.wait();
	}
}

void Sloong::SloongNetService::Exit()
{
	SendMessage(MSG_TYPE::ProgramExit);
}

void Sloong::SloongNetService::ExitEventHandler(SmartEvent ev)
{
	m_emStatus = RUN_STATUS::Exit;
	m_oSync.notify_all();
}

bool Sloong::SloongNetService::Add(DATA_ITEM item, void * object)
{
	auto it = m_oDataList.find(item);
	if (it != m_oDataList.end())
	{
		return false;
	}
	m_oDataList.insert(make_pair(item, object));
	return false;
}

void * Sloong::SloongNetService::Get(DATA_ITEM item)
{
	auto data = m_oDataList.find(item);
	if (data == m_oDataList.end())
	{
		return nullptr;
	}
	return (*data).second;
}

bool Sloong::SloongNetService::Remove(DATA_ITEM item)
{
	m_oDataList.erase(item);
}

bool Sloong::SloongNetService::AddTemp(string name, void * object)
{
	m_oTempDataList[name] = object;
	return true;
}

void * Sloong::SloongNetService::GetTemp(string name)
{
	auto item = m_oTempDataList.find(name);
	if (item == m_oTempDataList.end())
	{
		return nullptr;
	}
	m_oTempDataList.erase(name);
	return (*item).second;
}



void Sloong::SloongNetService::SendMessage(MSG_TYPE msgType)
{
	auto evt = make_shared<CNormalEvent>();
	evt->SetEvent(msgType);
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oSync.notify_one();
}

void Sloong::SloongNetService::SendMessage(SmartEvent evt)
{
	unique_lock<mutex> lck(m_oMsgListMutex);
	m_oMsgList.push(evt);
	m_oSync.notify_one();
}


void Sloong::SloongNetService::RegisterEvent(MSG_TYPE t)
{
	m_oMsgHandlerList[t] = vector<MsgHandlerFunc>();
}

/**
 * @Remarks: One message only have one handler. so cannot register handled message again.
 * @Params: 
 * @Return: 
 */
void Sloong::SloongNetService::RegisterEventHandler(MSG_TYPE t, MsgHandlerFunc func)
{
	if (m_oMsgHandlerList.find(t) == m_oMsgHandlerList.end())
	{
		throw normal_except("Target event is not regist.");
	}
	else{
		m_oMsgHandlerList[t].push_back(func);
	}
}


void Sloong::SloongNetService::MessageWorkLoop(SMARTER param)
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


