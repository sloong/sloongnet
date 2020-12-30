/*
 * @Author: WCB
 * @Date: 2020-04-28 14:43:07
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-28 14:52:01
 * @Description: file content
 */

#ifndef SLOONGNET_GATEWAY_TRANSPOND_H
#define SLOONGNET_GATEWAY_TRANSPOND_H


#include "IObject.h"

namespace Sloong
{
	class GatewayTranspond
	{
	public:
		GatewayTranspond(){}

		CResult Initialize(IControl*);

        PackageResult RequestPackageProcesser( DataPackage *);
        PackageResult ResponsePackageProcesser( UniquePackage, DataPackage *);
	private:
        PackageResult MessageToProcesser(DataPackage *);
		PackageResult MessageToClient(UniquePackage, DataPackage *);

    protected:
		IControl* 	m_iC = nullptr;
		CLog*		m_pLog =nullptr;
	};

}

#endif