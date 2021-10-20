/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2018-02-28 10:55:37
 * @LastEditTime: 2021-10-20 14:36:34
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

#pragma once

#include "IControl.h"

#include "core.h"
namespace Sloong
{
enum DataItemType
{
    String,
    Object,
    Bytes,
    SharedPtr,
};
class DataItem
{
  public:
    virtual ~DataItem() {}
    DataItemType Type;
};
class StringData : public DataItem
{
  public:
    StringData(const string &data)
    {
        Type = DataItemType::String;
        Data = data;
    }
    string Data;
};
class ObjectData : public DataItem
{
  public:
    ObjectData(void *p, int s)
    {
        Type = DataItemType::Object;
        Ptr = p;
        Size = s;
    }
    void *Ptr = nullptr;
    int Size = 0;
};
class BytesData : public DataItem
{
  public:
    BytesData(unique_ptr<char[]> &p, int s)
    {
        Type = DataItemType::Bytes;
        Ptr = std::move(p);
        Size = s;
    }
    unique_ptr<char[]> Ptr = nullptr;
    int Size = 0;
};
class SharedPtrData : public DataItem
{
  public:
    SharedPtrData(shared_ptr<void> p)
    {
        Type = DataItemType::SharedPtr;
        Ptr = p;
    }
    shared_ptr<void> Ptr = nullptr;
};
class CControlHub : public IControl
{
  public:
    virtual ~CControlHub()
    {
        Exit();
        m_oMsgHandlerList.clear();
        ThreadPool::Exit();
        TaskPool::Exit();
    }
    // Always return true
    CResult Initialize(int, spdlog::logger *);

    void Run() { m_emStatus = RUN_STATUS::Running; }
    void Exit();

    // Message
    void SendMessage(int32_t);
    void SendMessage(SharedEvent);

    void CallMessage(SharedEvent);

    void RegisterEventHandler(int32_t, MsgHandlerFunc);

    void MessageWorkLoop();

    // Data
    void Add(uint64_t item, void *object)
    {
        m_oDataList[item] = make_shared<ObjectData>(const_cast<void *>(object), 0);
    }
    void *Get(uint64_t);

    template <typename T> T GetAs(uint64_t item)
    {
        T tmp = static_cast<T>(Get(item));
        assert(tmp);
        return tmp;
    }

    void Remove(uint64_t item) { m_oDataList.erase(item); }

    void AddSharedPtr(uint64_t item, shared_ptr<void> object)
    {
        m_oDataList[item] = make_shared<SharedPtrData>(object);
    }
    shared_ptr<void> GetSharedPtr(uint64_t key)
    {
        auto baseitem = m_oDataList.try_get(key);
        if (baseitem == nullptr || (*baseitem)->Type != DataItemType::SharedPtr)
            return nullptr;

        auto item = dynamic_pointer_cast<SharedPtrData>(*baseitem);

        return item->Ptr;
    }

    template <typename T> shared_ptr<T> GetSharedPtrAs(uint64_t key)
    {
        return dynamic_pointer_cast<T>(GetSharedPtr(key));
    }

    void AddTempString(const string &key, const string &value)
    {
        m_oTempDataList[key] = make_shared<StringData>(value);
    }
    string GetTempString(const string &, bool = true);
    inline bool ExistTempString(const string &key)
    {
        auto i = m_oTempDataList.try_get(key);
        if (i == nullptr || (*i)->Type != DataItemType::String)
            return false;
        else
            return true;
    }

    void AddTempObject(const string &key, const void *object, int size)
    {
        m_oTempDataList[key] = make_shared<ObjectData>(const_cast<void *>(object), size);
    }
    void *GetTempObject(const string &, int *, bool = true);
    inline bool ExistTempObject(const string &key)
    {
        auto i = m_oTempDataList.try_get(key);
        if (i == nullptr || (*i)->Type != DataItemType::Object)
            return false;
        else
            return true;
    }

    void AddTempBytes(const string &key, unique_ptr<char[]> &bytes, int size)
    {
        m_oTempDataList[key] = make_shared<BytesData>(bytes, size);
    }

    unique_ptr<char[]> GetTempBytes(const string &, int *);
    inline bool ExistTempBytes(const string &key)
    {
        auto i = m_oTempDataList.try_get(key);
        if (i == nullptr || (*i)->Type != DataItemType::Bytes)
            return false;
        else
            return true;
    }

    void AddTempSharedPtr(const string &key, shared_ptr<void> ptr)
    {
        m_oTempDataList[key] = make_shared<SharedPtrData>(ptr);
    }
    shared_ptr<void> GetTempSharedPtr(const string &, bool = true);
    inline bool ExistTempSharedPtr(const string &key)
    {
        auto i = m_oTempDataList.try_get(key);
        if (i == nullptr || (*i)->Type != DataItemType::SharedPtr)
            return false;
        else
            return true;
    }

  protected:
    // Data
    map_ex<uint64_t, shared_ptr<DataItem>> m_oDataList;
    map_ex<string, shared_ptr<DataItem>> m_oTempDataList;

    // Message
    map_ex<int32_t, vector<MsgHandlerFunc>> m_oMsgHandlerList;
    queue_safety<SharedEvent> m_oMsgList;
    RUN_STATUS m_emStatus = RUN_STATUS::Created;
    EasySync m_oSync;

    spdlog::logger *m_pLog = nullptr;
};
} // namespace Sloong
