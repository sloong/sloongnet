/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 18:44:07
 * @Description: file content
 */ 
#pragma once

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

		void erase(const T& v){
			auto it = std::find(this->begin(), this->end(), v);
			if (it != this->end())
				list<T>::erase(it);
		}

		bool exist(const T& v) {
			auto it = std::find(this->begin(), this->end(), v);
			// Check if iterator points to end or not
			if (it == this->end())
				return false;
			else
				return true;
		}

		void copyfrom(const list_ex<T>& src){
			for( auto i:src)
				this->push_back(i);
		}
	};
}