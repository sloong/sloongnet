/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-18 20:34:21
 * @Description: file content
 */
#ifndef SLOONGNET_GATEWAY_SERVICE_H
#define SLOONGNET_GATEWAY_SERVICE_H

#include "EasyConnect.h"
#include "core.h"
#include "export.h"

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
	struct RequestInfo{
		int SerialNumber;
        EasyConnect*    RequestConnect = nullptr;
    };
	class SloongNetGateway
	{
	public:
		SloongNetGateway() {}

		CResult Initialization(GLOBAL_CONFIG *);
		CResult Initialized(SOCKET, IControl *);

		void QueryReferenceInfo();
		CResult QueryReferenceInfoResponseHandler(IEvent *, CDataTransPackage *);

		inline CResult CreateProcessEnvironmentHandler(void **);
		void EventPackageProcesser(CDataTransPackage *);
		CResult RequestPackageProcesser(void *, CDataTransPackage *);
		CResult ResponsePackageProcesser(CDataTransPackage *);

		// Event handler
		void OnStart(SmartEvent);
		void OnSocketClose(SmartEvent);
		void SendPackageHook(SmartEvent);

		CResult PackageProcesser(CDataTransPackage *);

		void OnReferenceModuleOnlineEvent(const string&,CDataTransPackage *);
		void OnReferenceModuleOfflineEvent(const string&,CDataTransPackage *);
	private:
		CResult MessageToProcesser(CDataTransPackage *);
		CResult MessageToClient(RequestInfo*, CDataTransPackage *);

		inline int ParseFunctionValue(const string&);
		list<int> ProcessProviedFunction(const string&);

		void AddConnection(const string&, const string&, int);

	protected:
		//list<shared_ptr<GatewayTranspond>> m_listTranspond;
		inline int GetSerialNumber(){ return ++m_nSerialNumber; }
		SOCKET GetPorcessConnect(int function);


	protected:
		map_ex<int, SmartEvent> m_listSendEvent;
		map_ex<int, list_ex<int>> m_mapFuncToTemplateIDs;
		map_ex<int, list_ex<string>> m_mapTempteIDToUUIDs;
		map_ex<string, NodeItem> m_mapUUIDToNode;
		map_ex<string, SOCKET> m_mapUUIDToConnect;
		map_ex<int, RequestInfo> m_mapSerialToRequest;

		IControl *m_pControl = nullptr;
		CLog *m_pLog = nullptr;
		GLOBAL_CONFIG *m_pConfig;
		Json::Value *m_pModuleConfig;
		RuntimeDataPackage *m_pRuntimeData = nullptr;
		SOCKET m_nManagerConnection = -1;
		int m_nSerialNumber = 0;

	public:
		static unique_ptr<SloongNetGateway> Instance;
	};

} // namespace Sloong

#endif