#include <map>
#include <memory>

namespace Sloong
{
	template<typename K, typename V>
	class map_ex:public std::map<K,V>
	{
	public:
		std::shared_ptr<V> try_get(K key)
		{
			auto it = this->find(key);
			if (it == this->end())
				return nullptr;
			return make_shared<V>(it->second);
		}

		V try_get(K key, V def)
		{
			auto it = this->find(key);
			if (it == this->end())
				return def;
			else
				return it->second;
		}

		bool exist(K key)
		{
			if (this->find(key) == this->end())
				return false;
			return true;
		}
	};
}
