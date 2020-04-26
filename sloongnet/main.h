/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-26 18:09:50
 * @Description: file conten
 */
// mian.h files
#ifndef SLOONGNET_MAIN_H
#define SLOONGNET_MAIN_H

#include "protocol/protocol.pb.h"
using namespace Protocol;

#include "core.h"
#include "export.h"
using namespace Sloong;


struct RunTimeData
{
	string ManagerAddress;
	int ManagerPort;
	SmartConnect ManagerConnect;
	int TemplateID;
    string NodeUUID;
	GLOBAL_CONFIG TemplateConfig;
	bool ManagerMode;
};


#endif //SLOONGNET_MAIN_H
