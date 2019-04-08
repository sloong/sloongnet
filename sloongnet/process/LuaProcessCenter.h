#pragma once

#define LUA_INT_TYPE LUA_INT_LONG

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
		
		void ReloadContext(SmartEvent event);
	public:
		static void HandleError(string err);
	protected:
		vector<CLua*>	m_pLuaList;
		vector<bool>	m_oReloadList;
		queue<int>		m_oFreeLuaContext;
		CEasySync		m_oSSync;
		mutex			m_oLuaContextMutex;
		unique_ptr<CGlobalFunction> m_pGFunc;
		ProtobufMessage::PROCESS_CONFIG* m_pConfig;
	};

}

