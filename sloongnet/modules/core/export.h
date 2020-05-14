/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-14 13:45:30
 * @Description: file content
 */

#ifndef SLOONGNET_MODULE_CORE_EXPORT_H
#define SLOONGNET_MODULE_CORE_EXPORT_H

#include "protocol/core.pb.h"
using namespace Core;
#include "DataTransPackage.h"
#include "sockinfo.h"
typedef CResult (*CreateProcessEnvironmentFunction)(void**);
typedef CResult (*RequestPackageProcessFunction)(void*,CDataTransPackage*);
typedef CResult (*ResponsePackageProcessFunction)(void*,CDataTransPackage*);
typedef CResult (*EventPackageProcessFunction)(CDataTransPackage*);
typedef CResult (*NewConnectAcceptProcessFunction)(CSockInfo*);
typedef CResult (*ModuleInitializationFunction)(GLOBAL_CONFIG*);
typedef CResult (*ModuleInitializedFunction)(SOCKET,IControl*);
#endif //SLOONGNET_MODULE_CORE_EXPORT_H