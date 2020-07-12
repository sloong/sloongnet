/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-16 16:26:29
 * @Description: file content
 */
#ifndef CONTROL_HUB_H
#define CONTROL_HUB_H

#include "IEvent.h"
#include "IControl.h"
#include "core.h"
namespace Sloong
{
	enum TempDataItemType
	{
		String,
		Object,
		Bytes,
		SharedPtr,
	};
	class TempDataItem
	{
	public:
		virtual ~TempDataItem(){}
		TempDataItemType Type;
	};
	class StringData : public TempDataItem
	{
	public:
		StringData( const string &data)
		{
			Type = TempDataItemType::String;
			Data = data;
		}
		string Data;
	};
	class ObjectData : public TempDataItem
	{
	public:
		ObjectData( void *p, int s)
		{
			Type = TempDataItemType::Object;
			Ptr = p;
			Size = s;
		}
		void *Ptr = nullptr;
		int Size = 0;
	};
	class BytesData : public TempDataItem
	{
	public:
		BytesData( unique_ptr<char[]> &p, int s)
		{
			Type = TempDataItemType::Bytes;
			Ptr = std::move(p);
			Size = s;
		}
		unique_ptr<char[]> Ptr = nullptr;
		int Size = 0;
	};
	class SharedPtrData : public TempDataItem
	{
	public:
		SharedPtrData( shared_ptr<void> p)
		{
			Type = TempDataItemType::SharedPtr;
			Ptr = p;
		}
		shared_ptr<void> Ptr = nullptr;
	};
	class CControlHub : public IControl
	{
	public:
		// Always return true
		CResult Initialize(int);

		void Run()
		{
			m_emStatus = RUN_STATUS::Running;
		}
		void Exit();

		// Message
		void SendMessage(EVENT_TYPE);
		void SendMessage(SharedEvent);

		void CallMessage(SharedEvent);

		void RegisterEventHandler(EVENT_TYPE, MsgHandlerFunc);

		void MessageWorkLoop();

		// Data
		void Add(DATA_ITEM item, void *object)
		{
			m_oDataList[item] = object;
		}
		void *Get(DATA_ITEM);

		template <typename T>
		T GetAs(DATA_ITEM item)
		{
			T tmp = static_cast<T>(Get(item));
			assert(tmp);
			return tmp;
		}

		void Remove(DATA_ITEM item)
		{
			m_oDataList.erase(item);
		}

		void AddTempString(const string &key, const string &value)
		{
			m_oTempDataList[key] = new StringData( value);
		}
		string GetTempString(const string &);
		inline bool ExistTempString(const string &key)
		{
			auto i = m_oTempDataList.try_get(key);
			if (i == nullptr || (*i)->Type != TempDataItemType::String)
				return false;
			else
				return true;
		}

		void AddTempObject(const string &key, const void *object, int size)
		{
			m_oTempDataList[key] = new ObjectData(const_cast<void *>(object), size);
		}
		void *GetTempObject(const string &, int *);
		inline bool ExistTempObject(const string &key) 
		{
			auto i = m_oTempDataList.try_get(key);
			if (i == nullptr || (*i)->Type != TempDataItemType::Object)
				return false;
			else
				return true;
		}

		void AddTempBytes(const string &key, unique_ptr<char[]> &bytes, int size)
		{
			m_oTempDataList[key] = new BytesData( bytes, size);
		}

		unique_ptr<char[]> GetTempBytes(const string &, int *);
		inline bool ExistTempBytes(const string &key)
		{
			auto i = m_oTempDataList.try_get(key);
			if (i == nullptr || (*i)->Type != TempDataItemType::Bytes)
				return false;
			else
				return true;
		}

		void AddTempSharedPtr(const string &key, shared_ptr<void> ptr)
		{
			m_oTempDataList[key] = new SharedPtrData( ptr);
		}
		shared_ptr<void> GetTempSharedPtr(const string &);
		inline bool ExistTempSharedPtr(const string &key)
		{
			auto i = m_oTempDataList.try_get(key);
			if (i == nullptr || (*i)->Type != TempDataItemType::SharedPtr)
				return false;
			else
				return true;
		}

	protected:
		// Data
		map<DATA_ITEM, void *> m_oDataList;
		map_ex<string, TempDataItem *> m_oTempDataList;

		// Message
		map_ex<EVENT_TYPE, vector<MsgHandlerFunc>> m_oMsgHandlerList;
		queue_ex<SharedEvent> m_oMsgList;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;
		CEasySync m_oSync;
	};
} // namespace Sloong

#endif