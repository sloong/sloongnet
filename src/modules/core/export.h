/*** 
 * @Author: Chuanbin Wang
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-28 20:31:47
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/export.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 13:45:30
 * @Description: file content
 */

#pragma once

#include "result.h"
typedef TResult<unique_ptr<DataPackage>> PackageResult;

#include "protocol/base.pb.h"
using namespace Base;
#include "protocol/core.pb.h"
using namespace Core;
#include "IControl.h"
typedef CResult (*CreateProcessEnvironmentFunction)(void**);
typedef PackageResult (*RequestPackageProcessFunction)(void*,DataPackage*);
typedef PackageResult (*ResponsePackageProcessFunction)(void*,DataPackage*);
typedef CResult (*EventPackageProcessFunction)(DataPackage*);
typedef CResult (*NewConnectAcceptProcessFunction)(SOCKET);
typedef CResult (*PrepareInitializeFunction)(GLOBAL_CONFIG*);
typedef CResult (*ModuleInitializationFunction)(IControl*);
typedef CResult (*ModuleInitializedFunction)();
