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
		static string MD5_Encoding(string str, bool bFile = false);

		static string Base64_Encoding(string str);
		static string Base64_Decoding(string str);

		static void tolower(string str)
		{
			transform(str.begin(), str.end(), str.begin(), ::tolower);
		}

		static void touper(string str)
		{
			transform(str.begin(), str.end(), str.begin(), ::toupper);
		}

	};



}

#endif // UTILITY_H
