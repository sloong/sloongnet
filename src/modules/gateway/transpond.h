/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2020-04-28 14:43:07
 * @LastEditTime: 2021-09-22 14:44:34
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/gateway/transpond.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */

#pragma once


#include "IObject.h"

namespace Sloong
{
	class GatewayTranspond : public IObject
	{
	public:
		GatewayTranspond(){}

		CResult Initialize(IControl*);

        PackageResult RequestPackageProcesser( Package *);
        PackageResult ResponsePackageProcesser( UniquePackage, Package *);
	private:
        PackageResult MessageToProcesser(Package *);
		PackageResult MessageToClient(UniquePackage, Package *);

    protected:
		// map_ex<int,>
	};

}
