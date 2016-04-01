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
}


#endif // !STRUCTS_H
