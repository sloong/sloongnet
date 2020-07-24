/*** 
 * @Author: Chuanbin Wang
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-24 16:29:23
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/gateway/gateway.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:34:21
 * @Description: file content
 */
#ifndef SLOONGNET_MODULE_GATEWAY_H
#define SLOONGNET_MODULE_GATEWAY_H

#include "EasyConnect.h"
#include "core.h"
#include "export.h"
#include "IObject.h"
#include "transpond.h"

#include "protocol/manager.pb.h"
using namespace Manager;

extern "C"
{
	PackageResult RequestPackageProcesser(void *, DataPackage *);
	PackageResult ResponsePackageProcesser(void *, DataPackage *);
	CResult EventPackageProcesser(DataPackage *);
	CResult NewConnectAcceptProcesser(SOCKET);
	CResult ModuleInitialization(GLOBAL_CONFIG *);
	CResult ModuleInitialized(SOCKET, IControl *);
	CResult CreateProcessEnvironment(void **);
}

namespace Sloong
{
	class SloongNetGateway : public IObject
	{
	public:
		SloongNetGateway() {}

		CResult Initialization(GLOBAL_CONFIG *);
		CResult Initialized(SOCKET, IControl *);

		PackageResult ResponsePackageProcesser(DataPackage *);

		void QueryReferenceInfo();
		void QueryReferenceInfoResponseHandler(IEvent*, DataPackage *);

		inline CResult CreateProcessEnvironmentHandler(void **);
		void EventPackageProcesser(DataPackage *);

		// Event handler
		void OnStart(SharedEvent);

		void OnReferenceModuleOnlineEvent(const string &, DataPackage *);
		void OnReferenceModuleOfflineEvent(const string &, DataPackage *);

	private:
		inline int ParseFunctionValue(const string &);
		list<int> ProcessProviedFunction(const string &);

		void AddConnection(uint64_t, const string &, int);

	public:
		SOCKET GetPorcessConnect(int function);
		map_ex<int, list_ex<int>> m_mapFuncToTemplateIDs;
		map_ex<int, list_ex<uint64_t>> m_mapTempteIDToUUIDs;
		map_ex<uint64_t, NodeItem> m_mapUUIDToNode;
		map_ex<uint64_t, int64_t> m_mapUUIDToConnectionID;
		map_ex<uint64_t, UniquePackage> m_mapSerialToRequest;

	protected:
		list<unique_ptr<GatewayTranspond>> m_listTranspond;
		
		GLOBAL_CONFIG *m_pConfig;
		Json::Value *m_pModuleConfig;
		RuntimeDataPackage *m_pRuntimeData = nullptr;
		SOCKET m_nManagerConnection = -1;

	public:
		static unique_ptr<SloongNetGateway> Instance;
	};

} // namespace Sloong

#endif