/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-03-18 20:39:26
 * @LastEditTime: 2021-09-23 17:02:16
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/map_ex.hpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: This is inherit for std::map and disabled the operation[] functions. so you must use insert function to
 * insert value to map. and use get/try_get to get value from map. 
 * The purpose of this is to ensure the value is you want get. not you want get but it's do insert.
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

#include <map>
#include <memory>
#include <shared_mutex>
using std::shared_lock;
using std::shared_mutex;
using std::unique_lock;

namespace Sloong
{
template <typename K, typename V> class map_ex : public std::map<K, V>
{
  private:
    shared_mutex m_mut;
    V &operator[](const K &key);

  public:
    V *try_get(const K &key)
    {
        shared_lock<shared_mutex> lock(m_mut);
        auto it = this->find(key);
        if (it == this->end())
            return nullptr;
        return &it->second;
    }

    V &try_get(const K &key, V &def)
    {
        shared_lock<shared_mutex> lock(m_mut);
        auto it = this->find(key);
        if (it == this->end())
            return def;
        else
            return it->second;
    }

    void insert(K key, const V &value)
    {
        unique_lock<shared_mutex> lock(m_mut);
        map<K, V>::operator[](key) = value;
    }

    void insert(K key, V &&value)
    {
        unique_lock<shared_mutex> lock(m_mut);
        map<K, V>::operator[](key) = move(value);
    }

    V &get(const K &key)
    {
        shared_lock<shared_mutex> lock(m_mut);
        auto it = this->find(key);
        if(  it == this->end())
        {
            stringstream ss;
            ss << key;
            throw std::runtime_error(format("map_ex: not found key {}",  ss.str()));
        }            

        return it->second;
    }

    bool exist(const K &key)
    {
        shared_lock<shared_mutex> lock(m_mut);
        if (this->find(key) == this->end())
            return false;
        return true;
    }

    void erase(const K &key)
    {
        unique_lock<shared_mutex> lock(m_mut);
        map<K, V>::erase(key);
    }

    V remove(const K &key)
    {
        unique_lock<shared_mutex> lock(m_mut);
        V value = move(map<K, V>::operator[](key));
        map<K, V>::erase(key);
        return move(value);
    }
};
} // namespace Sloong
