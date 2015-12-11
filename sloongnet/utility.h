#ifndef UTILITY_H
#define UTILITY_H

#include <univ/lua.h>

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

		int GetCpuUsed(int nWaitTime = 10);
		int GetMemory(int& total, int& free);

	};

}

#endif // UTILITY_H
