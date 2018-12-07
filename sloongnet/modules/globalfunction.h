#ifndef CGLOBALFUNCTION_H
#define CGLOBALFUNCTION_H

#include <univ/lua.h>
#include "IData.h"
#include "IMessage.h"
namespace Sloong
{
	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;
	class CUtility;
	using namespace Interface;
	class CGlobalFunction
	{
	public:
        CGlobalFunction();
		~CGlobalFunction();

		void Initialize(IMessage* iMsg,IData* iData);
		void Exit();
		void RegistFuncToLua(CLua* pLua);
		void EnableDataReceive(int port);

	protected:
		void ClearReceiveInfoByUUID(string uuid);

	private:
		static void* RecvDataConnFunc(void* pParam);
		static void* RecvFileFunc(void* pParam);
		
	protected:
        CUtility * m_pUtility;
		CLog*	m_pLog;
		IMessage* m_iMsg;
		IData*		m_iData;
		int 		m_ListenSock;
		bool		m_bIsRunning=true;
	public:


		static int Lua_ShowLog(lua_State* l);
		static int Lua_GetEngineVer(lua_State* l);
		static int Lua_Base64_Encode(lua_State* l);
		static int Lua_Base64_Decode(lua_State* l);
		static int Lua_Hash_Encode(lua_State* l);
		static int Lua_SendFile(lua_State* l);
		static int Lua_ReloadScript(lua_State* l);
		static int Lua_GetConfig(lua_State* l);
		static int Lua_MoveFile(lua_State* l);
		static int Lua_GenUUID(lua_State* l);
		static int Lua_ReceiveFile(lua_State* l);
		static int Lua_CheckRecvStatus(lua_State* l);
		static int Lua_SetCommData(lua_State* l);
		static int Lua_GetCommData(lua_State* l);
		static int Lua_GetLogObject(lua_State* l);
	public:
		static CGlobalFunction* g_pThis;
	};
}
#endif // !CGLOBALFUNCTION_H


