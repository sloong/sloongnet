#pragma once

#include "main.h"
namespace Sloong
{
	namespace Interface
	{
		class IData
		{
		public:
			virtual bool Add(DATA_ITEM item, void* object) = 0;
			virtual void* Get(DATA_ITEM item) = 0;
			virtual bool Remove(DATA_ITEM item) = 0;

			virtual bool AddTemp(string name, void* object) = 0;
			virtual void* GetTemp(string name) = 0;
		};
	}
}