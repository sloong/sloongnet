/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-18 12:38:53
 * @Description: file content
 */
#ifndef SLOONGNET_GATEWAY_SERVICE_H
#define SLOONGNET_GATEWAY_SERVICE_H


#include "sockinfo.h"
#include "core.h"
#include "export.h"

#include <jsoncpp/json/json.h>

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
		bool ProcessMessageHanlder(Functions func, string sender, CDataTransPackage* pack);
		void AcceptConnectProcesser(CSockInfo* info);
	private:
		void MessageToProcesser(CDataTransPackage* pack);
		void MessageToClient(CDataTransPackage* pack);
		void RegistFunctionHandler(Functions func, FuncHandler handler);
	protected:
		// 
		map<int,int>	m_mapProcessLoadList;
		map<int,SmartConnect> 	m_mapProcessList;
		map<int,string>		m_mapUUIDList;
		map<u_int64_t,CDataTransPackage*> m_mapPackageList;
		map_ex<Functions, FuncHandler> m_oFunctionHandles;
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;

		int m_nSerialNumber = 0;
		
		GLOBAL_CONFIG* m_pConfig;
		Json::Value    m_oExConfig;
	public:
		static unique_ptr<SloongNetGateway> Instance;
	};

}

#endif