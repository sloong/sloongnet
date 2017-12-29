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

	struct LogConfigInfo
	{
		bool	DebugMode;
		bool	ShowSendMessage;
		bool	ShowReceiveMessage;
		bool	LogWriteToOneFile;
		bool	ShowSQLCmd;
		bool	ShowSQLResult;
		int		LogLevel;
		string	LogPath;
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
