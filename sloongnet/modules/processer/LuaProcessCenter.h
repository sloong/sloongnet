/*
 * @Author: WCB
 * @Date: 2019-04-14 14:41:59
 * @LastEditors: WCB
 * @LastEditTime: 2019-11-06 17:33:54
 * @Description: file content
 */
#pragma once

#define LUA_INT_TYPE LUA_INT_LONG
#include "EasySync.hpp"
#include <jsoncpp/json/json.h>
#include "IObject.h"
namespace Sloong
{
	namespace Events
	{
		class CNetworkEvent;
	}
	using namespace Events;
	class CGlobalFunction;
	class CLuaProcessCenter : IObject
	{
	public:
		CLuaProcessCenter();
		~CLuaProcessCenter();

		void Initialize(IControl* iMsg);
		int NewThreadInit();
		void InitLua(CLua* pLua, string folder);
		void CloseSocket(CLuaPacket* uinfo);
		bool MsgProcess( CLuaPacket * pUInfo, const string& msg, string& res, char*& exData, int& exSize);
		int GetFreeLuaContext();
		
		void ReloadContext(IEvent* event);
	public:
		static void HandleError(const string& err);
	protected:
		vector<CLua*>	m_pLuaList;
		vector<bool>	m_oReloadList;
		queue<int>		m_oFreeLuaContext;
		CEasySync		m_oSSync;
		mutex			m_oLuaContextMutex;
		unique_ptr<CGlobalFunction> m_pGFunc;
		Json::Value*	m_pConfig;
	};

}

