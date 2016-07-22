#ifndef UTILITY_H
#define UTILITY_H

#include <univ/lua.h>
typedef unsigned char byte;
namespace Sloong
{
	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;
	class CUtility
	{
	public:
		CUtility();

		// Initialize the global variable. and register the function to lua.

		int GetCpuUsed(double nWaitTime = 0.1);
		int GetMemory(int& total, int& free);

		static int ReadFile(string filepath, char*& bBuffer);
		static string GenUUID();
	};



}

#endif // UTILITY_H
