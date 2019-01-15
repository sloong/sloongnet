#ifndef CMDPROCESS_H
#define CMDPROCESS_H

#include "main.h"

namespace Sloong
{
	class CCmdProcess
	{
	public:
		static void PrintVersion();
		static void PrintHelp();
		static bool Parser(int argc, char** args, CServerConfig*);
	};
}
#endif // !CMDPROCESS_H

