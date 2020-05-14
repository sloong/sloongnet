/*
 * @Author: WCB
 * @Date: 2020-04-24 20:40:22
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 19:10:09
 * @Description: file content
 */
#ifndef SLOONGNET_PROCESS_SERVICE_H
#define SLOONGNET_PROCESS_SERVICE_H


#include "sockinfo.h"
#include "core.h"
#include "export.h"
#include <jsoncpp/json/json.h>
#include "LuaProcessCenter.h"

#include "protocol/manager.pb.h"
using namespace Manager;

extern "C" {
	CResult RequestPackageProcesser(void*,CDataTransPackage*);
	CResult ResponsePackageProcesser(void*,CDataTransPackage*);
	CResult EventPackageProcesser(CDataTransPackage*);
	CResult NewConnectAcceptProcesser(CSockInfo*);
	CResult ModuleInitialization(GLOBAL_CONFIG*);
	CResult ModuleInitialized(SOCKET,IControl*);
	CResult CreateProcessEnvironment(void**);
}


namespace Sloong
{
	class SloongNetProcess
	{
	public:
		SloongNetProcess() {}
		~SloongNetProcess() {}

		CResult Initialization(GLOBAL_CONFIG*);
		CResult Initialized(SOCKET,IControl*);
		
		CResult RequestPackageProcesser(CLuaProcessCenter*,CDataTransPackage*);
		CResult ResponsePackageProcesser(CLuaProcessCenter*,CDataTransPackage*);
		inline CResult CreateProcessEnvironmentHandler(void**);
		
		void EventPackageProcesser(CDataTransPackage*);

		void OnSocketClose(SmartEvent evt);
		
	protected:
		list<shared_ptr<CLuaProcessCenter>> m_listProcess;
		map<string, unique_ptr<CLuaPacket>> m_mapUserInfoList;
		
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;

		int 		m_nSerialNumber = 0;
		SOCKET   m_nManagerConnection = -1;
		GLOBAL_CONFIG* m_pConfig;
		Json::Value    m_oExConfig;
	public:
		static unique_ptr<SloongNetProcess> Instance;
	};

}


#endif