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
	typedef struct ObjectData
	{
		void *ptr = nullptr;
		int size = 0;
	} ObjectData;
	typedef struct BytesData
	{
		unique_ptr<char[]> ptr = nullptr;
		int size = 0;
	} BytesData;
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
		void SendMessage(UniqueEvent);

		void CallMessage(UniqueEvent);

		inline void RegisterEvent(EVENT_TYPE t)
		{
			if( !m_oMsgHandlerList.exist(t))
				m_oMsgHandlerList[t] = vector<MsgHandlerFunc>();
		}
		void RegisterEventHandler(EVENT_TYPE, MsgHandlerFunc);
		inline void RegisterEventHook(EVENT_TYPE t, MsgHookFunc func)
		{
			m_listMsgHook[t] = func;
		}

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
			m_oTempStringList[key] = value;
		}
		string GetTempString(const string &);
		inline bool ExistTempString(const string &key) { return m_oTempStringList.exist(key); }

		void AddTempObject(const string &key, const void *object, int size) { m_oTempObjectList[key] = ObjectData{ptr : const_cast<void *>(object), size : size}; }
		void *GetTempObject(const string &, int *);
		inline bool ExistTempObject(const string &key) { return m_oTempObjectList.exist(key); }

		void AddTempBytes(const string &key, unique_ptr<char[]> &bytes, int size)
		{
			m_oTempBytesList[key] = BytesData();
			m_oTempBytesList[key].ptr = std::move(bytes);
			m_oTempBytesList[key].size = size;
		}
		unique_ptr<char[]> GetTempBytes(const string &, int *);
		inline bool ExistTempBytes(const string &key)
		{
			if (m_oTempBytesList.find(key) == m_oTempBytesList.end())
				return false;
			return true;
		}

	protected:
		// Data
		map<DATA_ITEM, void *> m_oDataList;
		map_ex<string, string> m_oTempStringList;
		map_ex<string, ObjectData> m_oTempObjectList;
		map<string, BytesData> m_oTempBytesList;
		// Message
		map_ex<EVENT_TYPE, vector<MsgHandlerFunc>> m_oMsgHandlerList;
		map_ex<EVENT_TYPE, MsgHookFunc> m_listMsgHook;
		queue_ex<UniqueEvent> m_oMsgList;
		RUN_STATUS m_emStatus = RUN_STATUS::Created;
		CEasySync m_oSync;
	};
} // namespace Sloong

#endif