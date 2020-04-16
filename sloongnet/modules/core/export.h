/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-16 16:30:51
 * @Description: file content
 */

#ifndef SLOONGNET_MODULE_CORE_EXPORT_H
#define SLOONGNET_MODULE_CORE_EXPORT_H

#include "DataTransPackage.h"
#include "sockinfo.h"
typedef CResult (*MessagePackageProcesser)(CDataTransPackage*);
typedef CResult (*NewConnectAcceptProcesser)(CSockInfo*);
typedef CResult (*ModuleInitialize)(IControl*);

#endif //SLOONGNET_MODULE_CORE_EXPORT_H