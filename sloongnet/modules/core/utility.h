#ifndef UTILITY_H
#define UTILITY_H

#include "core.h"

typedef unsigned char byte;
namespace Sloong
{
	class CUtility
	{
	public:
		CUtility();

		// Initialize the global variable. and register the function to lua.

		int GetCpuUsed(double nWaitTime = 0.1);
		int GetMemory(int& total, int& free);

		static string GetSocketIP(int socket);
		static int GetSocketPort(int socket);
		static string GetSocketAddress(int socket);

		static int ReadFile(const string& filepath, char*& bBuffer);
		static string GenUUID();

		static void write_call_stack();
	};



}

#endif // UTILITY_H
