/*
 * @Author: WCB
 * @Date: 2020-03-18 20:39:26
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 19:19:43
 * @Description: file content
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
	};
} // namespace Sloong
