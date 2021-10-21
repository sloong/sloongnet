/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-09-24 11:09:49
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/utility.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#ifndef UTILITY_H
#define UTILITY_H

#include "core.h"

typedef unsigned char byte;
namespace Sloong
{

	class CUtility
	{
	public:
		static string GetSocketIP(int socket);
		static int GetSocketPort(int socket);
		static string GetSocketAddress(int socket);

		static unique_ptr<char[]> ReadFile(const string &filepath, int *out_size);
		static CResult WriteFile(const string &filepath, const char *buf, int size);
		static uint64_t CityEncodeFile(const string &path);
		static uint32_t CRC32EncodeFile(const string &path);

		static string SHA1EncodeFile(const string &path);
		static string SHA256EncodeFile(const string &path);

		static string GenUUID();

		static VStrResult HostnameToIP(const string &);
		static VStrResult IPToHostName(const string &);

		static string GetCallStack();

		static bool FileExist(const string &file)
		{
			// 		On success (all requested permissions granted, or mode is F_OK
			//    and the file exists), zero is returned.  On error (at least one
			//    bit in mode asked for a permission that is denied, or mode is
			//    F_OK and the file does not exist, or some other error occurred),
			//    -1 is returned, and errno is set to indicate the error.
			if (0 == access(file.c_str(), F_OK))
				return true;
			else
				return false;
		}
	};

} // namespace Sloong

#endif // UTILITY_H
