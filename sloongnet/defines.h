#pragma once

#include "main.h"

template<typename T> inline
T TYPE_TRANS(LPVOID p)
{
	T tmp = static_cast<T>(p);
	assert(tmp);
	return tmp;
}


const int s_llLen = 8;

typedef enum g_DataCenter_MsgType
{
	ProgramStart,
	ProgramExit,

	//////////////////////////////////////////////////////////////////////////
	// 由 * EPollEx * 模块提供的消息
	//////////////////////////////////////////////////////////////////////////
	// 当接收到消息包之后，会发送该消息
	ReveivePackage,

	// 当连接关闭时会发送该消息
	// 需要在处理完成后调用回调函数以清除连接信息。
	// 参数类型为CNetworkEvent
	SocketClose,

	// 需要发送数据给客户端时，使用该消息
	SendMessage,

	//////////////////////////////////////////////////////////////////////////
	// 由 * LuaProcessCenter * 提供的消息
	//////////////////////////////////////////////////////////////////////////
	// 请求调用lua来处理消息。
	// 在处理完毕将会调用回调函数，回调参数为
	ProcessMessage,
	
	// 请求重新载入Lua环境
	// 当需要重新载入Lua Context的时候发送该请求。
	// 请求类型为CNormalEvent
	ReloadLuaContext,
	

}MSG_TYPE;

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
	int		LogLevel;
	string	LogPath;
	int		NetworkPort;
};

enum RecvStatus
{
	Wait=0,
	Receiving=1,
	Saveing=2,
	Done=3,
	VerificationError=4,
	OtherError=5,
};

// Receive file struce for GFunc
struct RecvDataPackage
{
	string strMD5 = "";
	RecvStatus emStatus = RecvStatus::Wait;
	string strName = "";
	string strPath = "";
};

enum HashType
{
	MD5 = 0,
	SHA_1 = 1,
	SHA_256 = 2,
	SHA_512 = 3,
};

enum NetworkResult
{
	Succeed = 1,
	Retry = 0,
	Error = -1,
};


typedef enum g_em_DataItem
{
	Configuation,
	Logger,
}DATA_ITEM;