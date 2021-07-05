/*** 
 * @......................................&&.........................
 * @....................................&&&..........................
 * @.................................&&&&............................
 * @...............................&&&&..............................
 * @.............................&&&&&&..............................
 * @...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * @................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * @.......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * @..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * @..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * @..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * @........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * @.......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * @........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * @..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * @................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * @..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * @..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * @...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * @.........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * @......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * @.....&&&&&&&&&&&&&&&&............................&&..............
 * @....&&&&&&&&&&&&&&&.................&&...........................
 * @...&&&&&&&&&&&&&&&.....................&&&&......................
 * @...&&&&&&&&&&.&&&........................&&&&&...................
 * @..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * @..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * @..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * @..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * @..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * @...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * @....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * @.......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * @.......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * @........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * @.......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * @.......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * @........&&...................&&&&&&.........................&&&..
 * @.........&.....................&&&&........................&&....
 * @...............................&&&.......................&&......
 * @................................&&......................&&.......
 * @.................................&&..............................
 * @..................................&..............................
 */

/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-04-24 20:40:22
 * @LastEditTime: 2020-07-24 15:03:09
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/middleLayer/lua/luaMiddleLayer.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */

#pragma once

#include "core.h"
#include "export.h"
#include "LuaProcessCenter.h"
#include "IObject.h"

#include "protocol/manager.pb.h"
using namespace Manager;

extern "C"
{
	PackageResult RequestPackageProcesser(void *, Package *);
	PackageResult ResponsePackageProcesser(void *, Package *);
	CResult EventPackageProcesser(Package *);
	CResult NewConnectAcceptProcesser(SOCKET);
	CResult ModuleInitialization(IControl*);
	CResult ModuleInitialized();
	CResult CreateProcessEnvironment(void **);
}

enum LUA_EVENT_TYPE
{
	OnReferenceModuleOnline = EVENT_TYPE::CustomEventMix +1,
	OnReferenceModuleOffline,
	ProcessLuaEvent,
};

namespace Sloong
{
	class LuaMiddleLayer : public IObject
	{
	public:
		LuaMiddleLayer() {  }
		~LuaMiddleLayer() {}

		CResult Initialization(IControl*);
		CResult Initialized();

		PackageResult RequestPackageProcesser(CLuaProcessCenter *, Package *);
		PackageResult ResponsePackageProcesser(CLuaProcessCenter *, Package *);
		inline CResult CreateProcessEnvironmentHandler(void **);

		void EventPackageProcesser(Package *);

		void OnReferenceModuleOnlineEvent(const string &, Package *);
		void OnReferenceModuleOfflineEvent(const string &, Package *);

		void OnConnectionBreaked(SharedEvent);

		void SetReloadScriptFlag();

		inline CLog* GetLog(){ return m_pLog; }

	protected:
		list_ex<shared_ptr<CLuaProcessCenter>> m_listProcess;
		map_ex<uint64_t, unique_ptr<CLuaPacket>> m_mapUserInfoList;
		GLOBAL_CONFIG *m_pConfig;
		Json::Value *m_pModuleConfig;

	public:
		static unique_ptr<LuaMiddleLayer> Instance;
	};

} // namespace Sloong
