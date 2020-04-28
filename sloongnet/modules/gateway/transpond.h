/*
 * @Author: WCB
 * @Date: 2020-04-28 14:43:07
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-28 14:52:01
 * @Description: file content
 */

#ifndef SLOONGNET_GATEWAY_TRANSPOND_H
#define SLOONGNET_GATEWAY_TRANSPOND_H


#include "main.h"
namespace Sloong
{
	class GatewayTranspond
	{
	public:
		GatewayTranspond(){}

		CResult Initialize(IControl*);

		CResult PackageProcesser(CDataTransPackage*);

	private:
        void AcceptConnectProcesser(CSockInfo* info);
		void MessageToProcesser(CDataTransPackage* pack);
		void MessageToClient(CDataTransPackage* pack);

	protected:
		// 
		map<int,int>	m_mapProcessLoadList;
		map<int,SmartConnect> 	m_mapProcessList;
		map<int,string>		m_mapUUIDList;
		map<u_int64_t,CDataTransPackage*> m_mapPackageList;
		int m_nSerialNumber = 0;
    protected:
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;
	};

}

#endif