#pragma once

#include "IData.h"
#include <map>
namespace Sloong
{
	using Interface::IData;
	class CDataCenter : public IData
	{
	public:
		CDataCenter();
		~CDataCenter();

		bool Add(DATA_ITEM item, void* object);
		void* Get(DATA_ITEM item);

		template<typename T>
		T GetAs(DATA_ITEM item) 
		{
			T tmp = static_cast<T>(Get(item));
			assert(tmp);
			return tmp;
		}

		bool Remove(DATA_ITEM item);

		bool AddTemp(string name, void* object);
		void* GetTemp(string name);

	protected:
		map<DATA_ITEM, void*> m_oDataList;
		map<string, void*> m_oTempDataList;
	};
}

