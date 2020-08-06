/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2018-02-28 10:55:37
 * @LastEditTime: 2020-08-06 14:24:33
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/ControlHub.h
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
		virtual ~CControlHub()
		{
			m_oMsgHandlerList.clear();
			CThreadPool::Exit();
		}
		// Always return true
		CResult Initialize(int, CLog*);

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
		void Add(int64_t item, void *object)
		{
			m_oDataList[item] = object;
		}
		void *Get(int64_t);

		template <typename T>
		T GetAs(int64_t item)
		{
			T tmp = static_cast<T>(Get(item));
			assert(tmp);
			return tmp;
		}

		void Remove(int64_t item)
		{
			m_oDataList.erase(item);
		}

		void AddTempString(const string &key, const string &value)
		{
			m_oTempDataList[key] = make_shared<StringData>(value);
		}
		string GetTempString(const string &,bool = true);
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
			m_oTempDataList[key] = make_shared<ObjectData>(const_cast<void *>(object), size);
		}
		void *GetTempObject(const string &, int *,bool = true);
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
			m_oTempDataList[key] = make_shared<BytesData>( bytes, size);
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
			m_oTempDataList[key] = make_shared<SharedPtrData>( ptr);
		}
		shared_ptr<void> GetTempSharedPtr(const string &,bool = true);
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
		map<int64_t, void *> m_oDataList;
		map_ex<string, shared_ptr<TempDataItem>> m_oTempDataList;

		// Message
		map_ex<EVENT_TYPE, vector<MsgHandlerFunc>> m_oMsgHandlerList;
		queue_ex<SharedEvent> m_oMsgList;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;
		EasySync m_oSync;

		CLog* m_pLog = nullptr;
	};
} // namespace Sloong

#endif