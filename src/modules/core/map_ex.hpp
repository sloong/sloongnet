/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-03-18 20:39:26
 * @LastEditTime: 2020-08-11 19:20:24
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/map_ex.hpp
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

#include <map>
#include <memory>
#include <shared_mutex>
using std::shared_lock;
using std::shared_mutex;
using std::unique_lock;

namespace Sloong
{
	template <typename K, typename V>
	class map_ex : public std::map<K, V>
	{
	private:
		shared_mutex m_mut;

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

		V &operator[](const K &key)
		{
			unique_lock<shared_mutex> lock(m_mut);
			return map<K, V>::operator[](key);
		}

		V &operator[](K &&key)
		{
			unique_lock<shared_mutex> lock(m_mut);
			return map<K, V>::operator[](key);
		}

		bool exist(const K &key)
		{
			shared_lock<shared_mutex> lock(m_mut);
			if (this->find(key) == this->end())
				return false;
			return true;
		}

		void erase( const K& key)
		{
			unique_lock<shared_mutex> lock(m_mut);
			map<K, V>::erase(key);
		}

		V remove(const K &key)
		{
			unique_lock<shared_mutex> lock(m_mut);
			V value= move(map<K, V>::operator[](key));
			map<K, V>::erase(key);
			return move(value);
		}
	};
} // namespace Sloong
