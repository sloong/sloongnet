#include <list>

namespace Sloong
{
	template<typename T>
	class list_ex :public std::list<T>
	{
	public:
		bool unique_insert(const T& v) {
			if (!this->exist(v))
			{
				this->push_back(v);
				return true;
			}
			return false;
		}

		bool exist(const T& v) {
			auto it = std::find(this->begin(), this->end(), v);
			// Check if iterator points to end or not
			if (it == this->end())
				return false;
			else
				return true;
		}
	};
}