#pragma once
#include "univ.h"

namespace Sloong
{
	namespace Universal
	{
		class UNIVERSAL_API CBase64
		{
		public:
			CBase64() {}
			~CBase64() {}

			static string Encode(const string& str);
			static string Decode(const string& str);
		};


	}
}
