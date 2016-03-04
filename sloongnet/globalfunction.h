#ifndef CGLOBALFUNCTION_H
#define CGLOBALFUNCTION_H

#include <univ/lua.h>
namespace Sloong
{
	struct SendExDataInfo
	{
		int m_nDataSize;
		char* m_pData;
		bool m_bIsEmpty;
	};

	namespace Universal
	{
		class CLog;
	}
	using namespace Universal;
	class CUtility;
	class CDBProc;
	class CGlobalFunction
	{
	public:
        CGlobalFunction();
		~CGlobalFunction();

		void Initialize(CLog* plog);
		void InitLua(CLua* pLua);
	protected:
        CUtility * m_pUtility;
		CDBProc* m_pDBProc;
		CLog*	m_pLog;
	public:
		static void HandleError(string err);

		static int Lua_showLog(lua_State* l);
		static int Lua_querySql(lua_State* l);
		static int Lua_modifySql(lua_State* l);
        static int Lua_getSqlError(lua_State* l);
		static int Lua_getThumbImage(lua_State* l);
		static int Lua_getEngineVer(lua_State* l);
		static int Lua_Base64_Encode(lua_State* l);
		static int Lua_Base64_Decode(lua_State* l);
		static int Lua_MD5_Encode(lua_State* l);
		static int Lua_SendFile(lua_State* l);
		static int Lua_ReloadScript(lua_State* l);
		
	public:
		static CGlobalFunction* g_pThis;
		map<int,SendExDataInfo> m_oSendExMapList;
		mutex		m_oListMutex;
		bool*		m_pReloadTagList;
		int			m_nTagSize;
	};
}
#endif // !CGLOBALFUNCTION_H



