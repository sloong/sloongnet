/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-03-24 14:57:00
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
	typedef struct PACKEDCPU
	{
		char name[20];		 //定义一个char类型的数组名name有20个元素
		unsigned int user;	 //定义一个无符号的int类型的user
		unsigned int nice;	 //定义一个无符号的int类型的nice
		unsigned int system; //定义一个无符号的int类型的system
		unsigned int idle;	 //定义一个无符号的int类型的idle
	} CPU_OCCUPY;

	class CUtility
	{
	public:
		// Initialize the global variable. and register the function to lua.
		static void RecordCPUStatus(CPU_OCCUPY *);
		static double CalculateCPULoad(CPU_OCCUPY *prev);
		static int GetCpuUsed(double nWaitTime = 0.1);
		static int GetMemory(int &total, int &free);

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
