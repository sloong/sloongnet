#ifndef LUA_H
#define LUA_H

#define LuaRes extern "C" int

#include <mutex>
#include <string>
using std::mutex;
using std::string;

#include "univ.h"
#include "luapacket.h"

struct lua_State;
namespace Sloong
{
	namespace Universal
	{
		typedef int (*LuaFunctionType)(lua_State *pLuaState);
		typedef void (*ErrorHandleType)(const string &strError);

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
			CLua(lua_State *l) { m_pScriptContext = l; }
			virtual ~CLua();

			bool LoadScript(const string &strFileName);
			bool RunScript(const string &strFileName);
			bool RunBuffer(LPCSTR pBuffer, size_t sz);
			bool RunString(const string &strCommand);
			void PushPacket(CLuaPacket *pData);
			bool RunFunction(string strFunctionName, CLuaPacket *pUserInfo, CLuaPacket *pRequest, CLuaPacket *pResponse);
			int RunFunction(string strFunctionName, CLuaPacket *pUserInfo, string &strRequest, string &strResponse);
			void RunFunction(string strFunctionName, CLuaPacket *pUserInfo);
			string GetErrorString();
			bool AddFunction(const string &strFunctionName, LuaFunctionType pFunction);
			bool PushFunction(int nFuncRef);
			bool GetLuaFuncRef(int &nFunc, const string &strFuncName);
			LuaType CheckType(int index);

			// Inline functions.
		public:
			inline bool RunFunction(const string &strFunctionName, const string &args) { return RunString(Helper::Format("%s(%s)", strFunctionName.c_str(), args.c_str())); }
			inline void HandlerError(const string &strErrorType, const string &strCmd) { HandlerError(strErrorType, strCmd.c_str()); }
			inline string GetString(int nNum, const string &strDefault = "") { return GetString(m_pScriptContext, nNum, strDefault); }
			double GetDouble(int nNum, double dDefault = -1.0f) { return GetDouble(m_pScriptContext, nNum, dDefault); }
			inline int GetInteger(int nNum, int nDef = -1) { return GetInteger(m_pScriptContext, nNum, nDef); }
			inline bool GetBoolen(int nNum) { return GetBoolen(m_pScriptContext, nNum); }
			inline void *GetPointer(int nNum) { return GetPointer(m_pScriptContext, nNum); }
			inline void PushString(const string &strString) { PushString(m_pScriptContext, strString); }
			inline void PushDouble(double dValue) { CLua::PushDouble(m_pScriptContext, dValue); }
			inline void PushInteger(int nValue) { PushInteger(m_pScriptContext, nValue); }
			inline void SetErrorHandle(ErrorHandleType pErr) { m_pErrorHandler = pErr; }
			inline lua_State *GetScriptContext() { return m_pScriptContext; }
			inline unique_ptr<map<string, string>> GetTableParam(int index) { return GetTableParam(m_pScriptContext, index); }
			inline void PushPointer(void *pPointer) { return CLua::PushPointer(m_pScriptContext, pPointer); }
			inline void HandlerError(const string &strErrorType, const char *strCmd)
			{
				if (m_pErrorHandler)
					m_pErrorHandler(Helper::Format("\n Error - %s:\n %s\n Error Message:%s%", strErrorType.c_str(), strCmd, GetErrorString().c_str()));
			}
			inline void AddFunctions(vector<LuaFunctionRegistr> *pFuncList)
			{
				for (auto item : *pFuncList)
					AddFunction(item.strFunctionName, item.pFunction);
			}
			
			
			inline bool PushFunction(const string &strFuncName)
			{
				int nFunc = 0;
				if (!GetLuaFuncRef(nFunc, strFuncName))
					return false;

				return PushFunction(nFunc);
			}
			inline void SetScriptFolder(const string &folder)
			{
				m_strScriptFolder = folder;
				char tag = m_strScriptFolder[m_strScriptFolder.length() - 1];
				if (tag != '/' && tag != '\\')
				{
					m_strScriptFolder += '/';
				}
			}
			inline vector<string>* GetSearchRouteList(){ return &m_listSearchRoute; }

			// Static functions.
		public:
			static string GetString(lua_State *l, int nNum, const string &strDefault = "");
			static double GetDouble(lua_State *l, int nNum, double dDefault = -1.0f);
			static int GetInteger(lua_State *l, int nNum, int nDef = -1);
			static bool GetBoolen(lua_State *l, int nNum);
			static void *GetPointer(lua_State *l, int nNum);
			static void PushString(lua_State *l, const string &strString);
			static void PushDouble(lua_State *l, double dValue);
			static void PushNil(lua_State *l);
			static void PushInteger(lua_State *l, int nValue);
			static void PushBoolen(lua_State *l, bool b);
			static void PushPointer(lua_State *l, void *pPointer);
			static unique_ptr<map<string, string>> GetTableParam(lua_State *l, int index);

		protected:
			string findScript(const string &strFullName);

		private:
			lua_State *m_pScriptContext = nullptr;
			ErrorHandleType m_pErrorHandler = nullptr;
			string m_strScriptFolder = "./";
			vector<string> m_listSearchRoute = {
				"%pathdir%%filename%.lua",
				"%pathdir%%filename%",
			};
		};

	} // namespace Universal

} // namespace Sloong

#endif // !LUA_H
