#pragma once
#include "univ.h"
namespace Sloong
{
	namespace Universal
	{
		class UNIVERSAL_API CDES
		{
		public:
			CDES();
			~CDES();

			static int Get_Length(const char* datain);
			static int Encrypt(const char* datain, unsigned char* dataout, const unsigned char* keyin);
		};


	}
}
