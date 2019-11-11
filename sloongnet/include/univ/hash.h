#pragma once
#include "univ.h"

namespace Sloong
{
	namespace Universal
	{
		const int MD5_LENGTH = 16;
		class UNIVERSAL_API CMD5
		{
		public:
			CMD5() {}
			~CMD5() {}

			static void Binary_Encode(string str, unsigned char(&md)[Sloong::Universal::MD5_LENGTH], bool bFile = false);
			static string Encode(string str, bool bFile = false);
		};
		
		const int SHA1_LENGTH = 32;
		class UNIVERSAL_API CSHA1
		{
		public:
			CSHA1() {}
			~CSHA1() {}

			// compare string or file hash value.
			static string Encode(string str_src, bool file = false);
			static void Binary_Encoding( const string& str_src, unsigned char (&md)[Sloong::Universal::SHA1_LENGTH], bool bFile = false);
		};

		const int SHA256_LENGTH = 32;
		const int SHA512_LENGTH = 64;
		class UNIVERSAL_API CSHA256
		{
		public:
			CSHA256() {}
			~CSHA256() {}

			// compare string or file hash value.
			static string Encode(string str_src, bool file = false);
			static void Binary_Encoding( const string& str_src, unsigned char (&md)[Sloong::Universal::SHA256_LENGTH], bool bFile = false);
		};

		class UNIVERSAL_API CSHA512
		{
		public:
			CSHA512() {}
			~CSHA512() {}

			// compare string or file hash value.
			static string Encode(string str_src, bool file = false);
			static void Binary_Encoding(const string& str_src, unsigned char(&md)[Sloong::Universal::SHA512_LENGTH], bool bFile = false);
		};
	}
}

