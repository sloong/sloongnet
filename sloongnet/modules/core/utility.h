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
		static void RecordCPUStatus(CPU_OCCUPY*);
		static double CalculateCPULoad(CPU_OCCUPY* prev);
		static int GetCpuUsed(double nWaitTime = 0.1);
		static int GetMemory(int &total, int &free);

		static string GetSocketIP(int socket);
		static int GetSocketPort(int socket);
		static string GetSocketAddress(int socket);

		static int ReadFile(const string &filepath, char *&bBuffer);
		static CResult WriteFile(const string &filepath, const char *buf, int size);

		static string GenUUID();

		static string GetCallStack();
	};

} // namespace Sloong

#endif // UTILITY_H
