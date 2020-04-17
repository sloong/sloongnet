/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-17 17:12:06
 * @Description: file content
 */

#ifndef SLOONGNET_MODULE_CORE_EXPORT_H
#define SLOONGNET_MODULE_CORE_EXPORT_H

#include "protocol/protocol.pb.h"
using namespace Protocol;
#include "DataTransPackage.h"
#include "sockinfo.h"
typedef CResult (*MessagePackageProcesserFunction)(CDataTransPackage*);
typedef CResult (*NewConnectAcceptProcesserFunction)(CSockInfo*);
typedef CResult (*ModuleInitializationFunction)(GLOBAL_CONFIG*);
typedef CResult (*ModuleInitializedFunction)(IControl*);

#endif //SLOONGNET_MODULE_CORE_EXPORT_H