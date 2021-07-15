#ifndef LUA_H
#define LUA_H

#define LuaRes extern "C" int

#include "luapacket.h"

struct lua_State;
namespace Sloong
{
	typedef int (*LuaFunctionType)(lua_State *pLuaState);

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
		bool AddFunction(const string &strFunctionName, LuaFunctionType pFunction);
		bool GetLuaFuncRef(int &nFunc, const string &strFuncName);
		LuaType CheckType(int index);
		inline lua_State *GetScriptContext() { return m_pScriptContext; }
		inline void AddFunctions(vector<LuaFunctionRegistr> *pFuncList)
		{
			for (auto item : *pFuncList)
				AddFunction(item.strFunctionName, item.pFunction);
		}
		inline void SetScriptFolder(const string &folder)
		{
			m_strScriptFolder = folder;
			FormatFolderString(m_strScriptFolder);
		}
		inline vector<string> *GetSearchRouteList() { return &m_listSearchRoute; }
		
		// Push value 
		void PushPacket(CLuaPacket *pData);
		bool PushFunction(int nFuncRef);
		inline void PushString(const string &strString) { PushString(m_pScriptContext, strString); }
		inline void PushDouble(double dValue) { CLua::PushDouble(m_pScriptContext, dValue); }
		inline void PushInteger(int nValue) { PushInteger(m_pScriptContext, nValue); }
		inline void PushPointer(void *pPointer) { return CLua::PushPointer(m_pScriptContext, pPointer); }
		inline bool PushFunction(const string &strFuncName)
		{
			int nFunc = 0;
			if (!GetLuaFuncRef(nFunc, strFuncName))
				return false;

			return PushFunction(nFunc);
		}

		// Run lua
		CResult RunScript(const string &strFileName);
		CResult RunBuffer(LPCSTR pBuffer, size_t sz);
		CResult RunString(const string &strCommand);
		CResult RunFunction(const string &, CLuaPacket *, int = 0, const string & = "", const string & = "", string *extendDataUUID = nullptr);
		inline CResult RunFunction(const string &strFunctionName, const string &args) { return RunString(Helper::Format("%s(%s)", strFunctionName.c_str(), args.c_str())); }
		CResult RunEventFunction( const string&, int, const string&);

		// Get Valkue
		inline string GetString(int nNum, const string &strDefault = "") { return GetString(m_pScriptContext, nNum, strDefault); }
		double GetDouble(int nNum, double dDefault = -1.0f) { return GetDouble(m_pScriptContext, nNum, dDefault); }
		inline int GetInteger(int nNum, int nDef = -1) { return GetInteger(m_pScriptContext, nNum, nDef); }
		inline bool GetBoolen(int nNum) { return GetBoolen(m_pScriptContext, nNum); }
		inline void *GetPointer(int nNum) { return GetPointer(m_pScriptContext, nNum); }
		inline unique_ptr<map<string, string>> GetTableParam(int index) { return GetTableParam(m_pScriptContext, index); }
		
		// Static functions.
	public:
		static string GetString(lua_State *, int, const string & = "");
		static double GetDouble(lua_State *, int, double = -1.0f);
		static int GetInteger(lua_State *, int, int = -1);
		static bool GetBoolen(lua_State *, int);
		static void *GetPointer(lua_State *, int);
		static void PushString(lua_State *, const string &);
		static void PushDouble(lua_State *, double);
		static void PushNil(lua_State *);
		static void PushInteger(lua_State *, int);
		static void PushBoolen(lua_State *, bool);
		static void PushPointer(lua_State *, void *);
		static string GetCallStack(lua_State *);
		static void PushTable(lua_State *, const map<string, string> &);
		static void PushTable(lua_State*, const list<string>&);
		static void Push2DTable(lua_State*, const list<list<string>>&);
		static void Push2DTable(lua_State*, const list<map<string, string>>&);
		static void Push2DTable(lua_State*, const map<string,map<string, string>>&);
		static unique_ptr<map<string, string>> GetTableParam(lua_State *, int);

	protected:
		string findScript(const string &strFullName);
		inline CResult HandlerError( const string &strErrorTitle, const string &strErrorOperation, int res = -1)
		{
			auto str = Helper::Format("\n [%d]Error - %s:\n %s\n Error Message:%s", res, strErrorTitle.c_str(), strErrorOperation.c_str(), GetString(m_pScriptContext,1).c_str());
			//Helper::Format("\n Error - %s:\n %s\n Error Message:%s", strErrorType.c_str(), strCmd, GetCallStack(m_pScriptContext).c_str())
			#ifndef HIDE_LUA_ERROR
				cout << str << endl;
				return CResult::Make_Error(str);
			#else
				return CResult::Make_Error("Service internal error.");
			#endif
			
		}

	private:
		lua_State *m_pScriptContext = nullptr;
		string m_strScriptFolder = "./";
		vector<string> m_listSearchRoute = {
			"%pathdir%%filename%",
			"%pathdir%%filename%.lua",
		};
	};

} // namespace Sloong

#endif // !LUA_H
