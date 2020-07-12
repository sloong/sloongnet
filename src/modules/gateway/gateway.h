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
	class SloongNetGateway : public IObject
	{
	public:
		SloongNetGateway() {}

		CResult Initialization(GLOBAL_CONFIG *);
		CResult Initialized(SOCKET, IControl *);

		CResult ResponsePackageProcesser(CDataTransPackage *);

		void QueryReferenceInfo();
		CResult QueryReferenceInfoResponseHandler(IEvent*, CDataTransPackage *);

		inline CResult CreateProcessEnvironmentHandler(void **);
		void EventPackageProcesser(CDataTransPackage *);

		// Event handler
		void OnStart(SharedEvent);
		void OnSocketClose(SharedEvent);

		void OnReferenceModuleOnlineEvent(const string &, CDataTransPackage *);
		void OnReferenceModuleOfflineEvent(const string &, CDataTransPackage *);

	private:
		inline int ParseFunctionValue(const string &);
		list<int> ProcessProviedFunction(const string &);

		void AddConnection(uint64_t, const string &, int);

	public:
		SOCKET GetPorcessConnect(int function);
		map_ex<int, list_ex<int>> m_mapFuncToTemplateIDs;
		map_ex<int, list_ex<uint64_t>> m_mapTempteIDToUUIDs;
		map_ex<uint64_t, NodeItem> m_mapUUIDToNode;
		map_ex<uint64_t, SOCKET> m_mapUUIDToConnect;
		map_ex<uint64_t, RequestInfo> m_mapSerialToRequest;

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