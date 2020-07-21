/*
 * @Author: WCB
 * @Date: 2020-04-28 14:43:07
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-28 14:52:01
 * @Description: file content
 */

#ifndef SLOONGNET_GATEWAY_TRANSPOND_H
#define SLOONGNET_GATEWAY_TRANSPOND_H

#include "DataTransPackage.h"
#include "ConnectSession.h"

namespace Sloong
{
	struct RequestInfo{
		timeval tStart;
		timeval tProcess;
		uint64_t SerialNumber;
        EasyConnect*    RequestConnect = nullptr;
    };
	class GatewayTranspond
	{
	public:
		GatewayTranspond(){}

		CResult Initialize(IControl*);

        CResult RequestPackageProcesser( CDataTransPackage *);
        CResult ResponsePackageProcesser( RequestInfo*, CDataTransPackage *);
	private:
        CResult MessageToProcesser(CDataTransPackage *);
		CResult MessageToClient(RequestInfo*, CDataTransPackage *);

    protected:
		IControl* 	m_pControl = nullptr;
		CLog*		m_pLog =nullptr;
	};

}

#endif