#ifndef STRUCTS_H
#define STRUCTS_H

#include "main.h"

namespace Sloong
{
	struct MySQLConnectInfo
	{
		string Address;
		int Port;
		string User;
		string Password;
		string Database;
	};

	struct LuaScriptConfigInfo
	{
		string EntryFile;
		string EntryFunction;
		string ProcessFunction;
		string ScriptFolder;
	};
}


#endif // !STRUCTS_H
