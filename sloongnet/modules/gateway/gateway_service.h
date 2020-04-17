/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 18:53:06
 * @Description: file content
 */
#ifndef SLOONGNET_GATEWAY_SERVICE_H
#define SLOONGNET_GATEWAY_SERVICE_H


#include "sockinfo.h"
#include "core.h"
#include "export.h"

extern "C" {
	CResult MessagePackageProcesser(CDataTransPackage*);
	CResult NewConnectAcceptProcesser(CSockInfo*);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(IControl*);
}

namespace Sloong
{
	typedef std::function<bool(Functions, string, CDataTransPackage*)> FuncHandler;
	class SloongNetGateway
	{
	public:
		SloongNetGateway(){}

		CResult Initialization(GLOBAL_CONFIG*);
		CResult Initialized(IControl*);
		
		bool ConnectToProcess();

		CResult MessagePackageProcesser(CDataTransPackage*);

		// Event handler
		void OnStart(SmartEvent);
		void OnSocketClose(SmartEvent);

		// Network processer
		bool ProcessMessageHanlder(Functions func, string sender, SmartPackage pack);
		void AcceptConnectProcesser(shared_ptr<CSockInfo>);
	private:
		void MessageToProcesser(SmartPackage pack);
		void MessageToClient(SmartPackage pack);
		void RegistFunctionHandler(Functions func, FuncHandler handler);
	protected:
		// 
		map<int,int>	m_mapProcessLoadList;
		map<int,SmartConnect> 	m_mapProcessList;
		map<int,string>		m_mapUUIDList;
		map<u_int64_t,SmartPackage> m_mapPackageList;
		unique_ptr<CServerManage>	m_pServer = make_unique<CServerManage>();
		map_ex<Functions, FuncHandler> m_oFunctionHandles;
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;
		
		GLOBAL_CONFIG* m_pConfig;
	public:
		static unique_ptr<SloongNetGateway> Instance;
	};

}

#endif