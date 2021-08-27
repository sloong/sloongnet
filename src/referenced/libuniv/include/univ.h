/*** 
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-03-25 15:52:09
 * @LastEditTime: 2021-08-27 10:32:37
 * @LastEditors: Chuanbin Wang
 * @FilePath: /libuniv/src/univ.h
 * Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */
#ifdef _WINDOWS
#ifdef SLOONGUNIVERSAL_EXPORTS
#define UNIVERSAL_API __declspec(dllexport)
#else
#define UNIVERSAL_API __declspec(dllimport)
#endif
#else
#define UNIVERSAL_API
#endif

#pragma once

#include "helper.hpp"
#include "defines.h"
#include "univ.h"
#include "log.h"
#include "threadpool.h"
#include "taskpool.h"
#include "hash.h"
#include "base64.h"
#include "CRC.hpp"
