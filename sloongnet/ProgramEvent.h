#pragma once
#include "NormalEvent.h"

namespace Sloong
{
	namespace Events
	{
		class CProgramStartEvent : public CNormalEvent
		{
		public:
			CProgramStartEvent() {}
			virtual ~CProgramStartEvent();
		};
	}
}
