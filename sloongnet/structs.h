#ifndef STRUCTS_H
#define STRUCTS_H

#include "main.h"

namespace Sloong
{
	struct MySQLConnectInfo
	{
		bool Enable;
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
		string SocketCloseFunction;
		string ScriptFolder;
	};

	enum EventType
	{
		ReceivedData,
		SocketClose,
	};

	struct EventListItem
	{
		EventType emType;
		int nSocket;
	};
}


#endif // !STRUCTS_H
