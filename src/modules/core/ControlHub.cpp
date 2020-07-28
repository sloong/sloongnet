
#include "ControlHub.h"
#include "univ/exception.h"
#include "events/NormalEvent.hpp"
using namespace Sloong::Events;

CResult Sloong::CControlHub::Initialize(int quantity, CLog* log)
{
	if (quantity < 1)
		return CResult::Make_Error("CControlHub work quantity must big than 0.");
	m_pLog = log;
	CThreadPool::AddWorkThread(std::bind(&CControlHub::MessageWorkLoop, this), quantity);
	CThreadPool::Initialize(3);
	CThreadPool::Run();
	Run();
	return CResult::Succeed();
}

void Sloong::CControlHub::Exit()
{
	m_emStatus = RUN_STATUS::Exit;
}

void *Sloong::CControlHub::Get(DATA_ITEM item)
{
	auto data = m_oDataList.find(item);
	if (data == m_oDataList.end())
		return nullptr;

	return (*data).second;
}

string Sloong::CControlHub::GetTempString(const string &key, bool erase)
{
	auto baseitem = m_oTempDataList.try_get(key);
	if (baseitem == nullptr || (*baseitem)->Type != TempDataItemType::String)
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
	if (baseitem == nullptr || (*baseitem)->Type != TempDataItemType::Object)
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
	if (baseitem == nullptr || (*baseitem)->Type != TempDataItemType::Bytes)
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
	if (baseitem == nullptr || (*baseitem)->Type != TempDataItemType::SharedPtr)
		return nullptr;

	auto item = dynamic_pointer_cast<SharedPtrData>(*baseitem);
	auto ptr = item->Ptr;
	if (erase)
		m_oTempDataList.erase(key);

	return ptr;
}

void Sloong::CControlHub::SendMessage(EVENT_TYPE msgType)
{
	auto event = make_unique<NormalEvent>();
	event->SetEvent(msgType);
	m_oMsgList.push_move(std::move(event));

	m_oSync.notify_one();
}

void Sloong::CControlHub::SendMessage(SharedEvent evt)
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
	if (!m_oMsgHandlerList.exist(t))
		m_oMsgHandlerList[t] = vector<MsgHandlerFunc>();

	m_oMsgHandlerList[t].push_back(func);
}

void Sloong::CControlHub::CallMessage(SharedEvent event)
{
	try
	{
		auto evt_type = event->GetEvent();
		auto handler_list = m_oMsgHandlerList[evt_type];
		auto handler_num = handler_list.size();
		if (handler_num == 0)
			return;

		for (auto func : handler_list)
			func(event);
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
	
	m_pLog->Info("Control hub work thread is started. PID:" + spid);

	while (m_emStatus == RUN_STATUS::Created)
	{
		this_thread::sleep_for(std::chrono::microseconds(100));
	}

	m_pLog->Info("Control hub work thread is running. PID:" + spid);
	while (m_emStatus != RUN_STATUS::Exit)
	{
		try
		{
			if (m_oMsgList.empty())
			{
				m_oSync.wait_for(1000);
				continue;
			}

			auto event = m_oMsgList.TryMovePop();
			if (event != nullptr)
			{
				// Get the message handler list.
				CallMessage(event);
			}
		}
		catch (...)
		{
			cerr << "Unhandle exception in CControlHub work loop." << endl;
		}
	}
	m_pLog->Info("Control hub work thread is exit " + spid);
}
