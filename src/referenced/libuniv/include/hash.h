/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-08-20 15:19:54
 * @LastEditTime: 2020-08-21 13:20:36
 * @LastEditors: Chuanbin Wang
 * @FilePath: /libuniv/src/hash.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#pragma once
#include "univ.h"

namespace Sloong
{
	namespace Universal
	{
		// Same as MD5_DIGEST_LENGTH
		const int MD5_LENGTH = 16;
		class UNIVERSAL_API CMD5
		{
		public:
			static void Binary_Encode(const string& str, unsigned char(&md)[Sloong::Universal::MD5_LENGTH], bool bFile = false);
			static string Encode(const string& str, bool bFile = false);
		};

		class UNIVERSAL_API CCity
		{
		public:
			static int64_t Encode64(const string& str);
		};
		
		// Same as SHA_DIGEST_LENGTH
		const int SHA1_LENGTH = 20; 
		class UNIVERSAL_API CSHA1
		{
		public:
			// compare string or file hash value.
			static string Encode(const string& str_src, bool file = false);
			static void Binary_Encoding( const string& str_src, unsigned char (&md)[Sloong::Universal::SHA1_LENGTH], bool bFile = false);
		};

		// Same as SHA256_DIGEST_LENGTH
		const int SHA256_LENGTH = 32;
		class UNIVERSAL_API CSHA256
		{
		public:
			// compare string or file hash value.
			static string Encode(const string& str_src, bool file = false);
			static void Binary_Encoding( const string& str_src, unsigned char (&md)[Sloong::Universal::SHA256_LENGTH], bool bFile = false);
		};

		// Same as SHA512_DIGEST_LENGTH
		const int SHA512_LENGTH = 64;
		class UNIVERSAL_API CSHA512
		{
		public:
			// compare string or file hash value.
			static string Encode(const string& str_src, bool file = false);
			static void Binary_Encoding(const string& str_src, unsigned char(&md)[Sloong::Universal::SHA512_LENGTH], bool bFile = false);
		};
	}
}

