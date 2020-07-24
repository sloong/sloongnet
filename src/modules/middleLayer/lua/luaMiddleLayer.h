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
 * @Author: Chuanbin Wang
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
#include <jsoncpp/json/json.h>
#include "LuaProcessCenter.h"
#include "IObject.h"

#include "protocol/manager.pb.h"
using namespace Manager;

extern "C"
{
	PackageResult RequestPackageProcesser(void *, DataPackage *);
	PackageResult ResponsePackageProcesser(void *, DataPackage *);
	CResult EventPackageProcesser(DataPackage *);
	CResult NewConnectAcceptProcesser(SOCKET);
	CResult ModuleInitialization(GLOBAL_CONFIG *);
	CResult ModuleInitialized( IControl *);
	CResult CreateProcessEnvironment(void **);
}

namespace Sloong
{
	class LuaMiddleLayer : public IObject
	{
	public:
		LuaMiddleLayer() {}
		~LuaMiddleLayer() {}

		CResult Initialization(GLOBAL_CONFIG *);
		CResult Initialized( IControl *);

		PackageResult RequestPackageProcesser(CLuaProcessCenter *, DataPackage *);
		PackageResult ResponsePackageProcesser(CLuaProcessCenter *, DataPackage *);
		inline CResult CreateProcessEnvironmentHandler(void **);

		void EventPackageProcesser(DataPackage *);

		void OnReferenceModuleOnlineEvent(const string &, DataPackage *);
		void OnReferenceModuleOfflineEvent(const string &, DataPackage *);

		void OnConnectionBreak(SharedEvent);

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
