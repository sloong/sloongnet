#include <map>
#include <memory>
template<typename K,typename V>
class map_ex:public std::map<K,V>
{
public:
	std::shared_ptr<V> try_get(K key)
	{
		if (this->find(key) == this->end())
			return nullptr;
		return make_shared<V>(this->find(key)->second);
	}
};