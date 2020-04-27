#ifndef LUA_H
#define LUA_H


#define LuaRes extern "C" int

#include <mutex>
using std::mutex;

#include "univ.h"
#include "luapacket.h"

struct lua_State;
namespace Sloong
{
	namespace Universal
	{
		typedef int(*LuaFunctionType)(lua_State* pLuaState);
        typedef void(*ErrorHandleType)(std::string strError);

		struct LuaFunctionRegistr
		{
			string strFunctionName;
			LuaFunctionType pFunction;
		};

		enum LuaType
		{
			TNONE = -1,

			TNIL = 0,
			TBOOLEAN = 1,
			TLIGHTUSERDATA = 2,
			TNUMBER = 3,
			TSTRING = 4,
			TTABLE = 5,
			TFUNCTION = 6,
			TUSERDATA = 7,
			TTHREAD = 8,
			NUMTAGS = 9,
		};


		class UNIVERSAL_API CLua
		{
		public:
			CLua();
			CLua(lua_State* l);
			virtual ~CLua();

            bool    LoadScript(std::string strFileName);
			bool	RunScript(std::string strFileName);
			bool	RunBuffer(LPCSTR pBuffer, size_t sz);
			bool	RunString(std::string strCommand);
			bool	RunFunction(std::string strFunctionName, std::string args);
            bool    RunFunction(string strFunctionName,CLuaPacket* pUserInfo, CLuaPacket* pRequest, CLuaPacket* pResponse );
			int     RunFunction(string strFunctionName, CLuaPacket* pUserInfo, string& strRequest, string& strResponse);
			void    RunFunction(string strFunctionName, CLuaPacket* pUserInfo);
			std::string	GetErrorString();
			void	HandlerError(std::string strErrorType, std::string strCmd);
			bool	AddFunction(std::string strFunctionName, LuaFunctionType pFunction);
            void    AddFunctions(vector<LuaFunctionRegistr>* pFuncList);
			string	GetString(int nNum, std::string strDefault = "");
			double	GetDouble(int nNum, double dDefault = -1.0f);
			int		GetInteger(int nNum, int nDef = -1);
			bool	GetBoolen(int nNum);
			void*	GetPointer( int nNum);
			void	PushString(std::string strString);
			void	PushDouble(double dValue);
			void	PushInteger(int nValue);
            void    PushPacket( CLuaPacket* pData );
			void    PushPointer(void* pPointer);
            bool    PushFunction( int nFuncRef );
            bool    PushFunction( const string& strFuncName );
            bool    GetLuaFuncRef( int& nFunc, const string& strFuncName );
            void	SetErrorHandle(ErrorHandleType pErr);
			lua_State*	GetScriptContext();
			map<string, string> GetTableParam(int index);
			LuaType	CheckType(int index);
			void	SetScriptFolder(string folder);

		public:
			static std::string	GetString(lua_State* l, int nNum, std::string strDefault = "");
			static double GetDouble(lua_State* l, int nNum, double dDefault = -1.0f);
			static int GetInteger(lua_State*l, int nNum, int nDef = -1);
			static bool GetBoolen(lua_State* l, int nNum);
			static void* GetPointer(lua_State* l, int nNum);
			static void	PushString(lua_State* l, std::string strString);
			static void	PushDouble(lua_State* l, double dValue);
			static void PushInteger(lua_State*l, int nValue);
			static void PushBoolen(lua_State* l, bool b);
			static void PushPointer(lua_State* l, void* pPointer);
			static map<string, string> GetTableParam(lua_State*l, int index);
		protected:
			string findScript(std::string strFullName);

		private:
			lua_State *m_pScriptContext;
            ErrorHandleType m_pErrorHandler;
			string m_strScriptFolder;
		};

	}

}

#endif // !LUA_H

