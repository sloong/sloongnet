/*
 * @Author: WCB
 * @Date: 2020-03-18 20:39:26
 * @LastEditors: WCB
 * @LastEditTime: 2020-03-18 20:39:38
 * @Description: file content
 */
#include <map>
#include <memory>

namespace Sloong
{
	template<typename K, typename V>
	class map_ex:public std::map<K,V>
	{
	public:
		std::shared_ptr<V> try_get(const K& key)
		{
			auto it = this->find(key);
			if (it == this->end())
				return nullptr;
			return make_shared<V>(it->second);
		}

		V try_get(const K& key, const K& def)
		{
			auto it = this->find(key);
			if (it == this->end())
				return def;
			else
				return it->second;
		}

		bool exist(const K& key)
		{
			if (this->find(key) == this->end())
				return false;
			return true;
		}
	};
}
