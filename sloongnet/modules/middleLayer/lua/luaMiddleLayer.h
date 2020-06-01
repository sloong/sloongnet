/*
 * @Author: WCB
 * @Date: 2020-04-24 20:40:22
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-19 19:28:21
 * @Description: file content
 */
#ifndef SLOONGNET_MODULE_MIDDLE_LAYER_LUA_H
#define SLOONGNET_MODULE_MIDDLE_LAYER_LUA_H

#include "sockinfo.h"
#include "core.h"
#include "export.h"
#include <jsoncpp/json/json.h>
#include "LuaProcessCenter.h"

#include "protocol/manager.pb.h"
using namespace Manager;

extern "C"
{
	CResult RequestPackageProcesser(void *, CDataTransPackage *);
	CResult ResponsePackageProcesser(void *, CDataTransPackage *);
	CResult EventPackageProcesser(CDataTransPackage *);
	CResult NewConnectAcceptProcesser(CSockInfo *);
	CResult ModuleInitialization(GLOBAL_CONFIG *);
	CResult ModuleInitialized(SOCKET, IControl *);
	CResult CreateProcessEnvironment(void **);
}

namespace Sloong
{
	class LuaMiddleLayer
	{
	public:
		LuaMiddleLayer() {}
		~LuaMiddleLayer() {}

		CResult Initialization(GLOBAL_CONFIG *);
		CResult Initialized(SOCKET, IControl *);

		CResult RequestPackageProcesser(CLuaProcessCenter *, CDataTransPackage *);
		CResult ResponsePackageProcesser(CLuaProcessCenter *, CDataTransPackage *);
		inline CResult CreateProcessEnvironmentHandler(void **);

		void EventPackageProcesser(CDataTransPackage *);

		void OnSocketClose(IEvent *evt);

	protected:
		list_ex<shared_ptr<CLuaProcessCenter>> m_listProcess;
		map_ex<uint64_t, unique_ptr<CLuaPacket>> m_mapUserInfoList;

		IControl *m_pControl = nullptr;

		int m_nSerialNumber = 0;
		SOCKET m_nManagerConnection = -1;
		GLOBAL_CONFIG *m_pConfig;
		Json::Value *m_pModuleConfig;

	public:
		CLog *m_pLog = nullptr;
		static unique_ptr<LuaMiddleLayer> Instance;
	};

} // namespace Sloong

#endif // SLOONGNET_MODULE_MIDDLE_LAYER_LUA_H