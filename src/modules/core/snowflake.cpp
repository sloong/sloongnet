/*** 
 * @Author: Chuanbin Wang
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2020-07-29 16:04:20
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/snowflake.cpp
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description: 
 */

#include "snowflake.h"

using Sloong::snowflake;
using std::unique_ptr;
using std::make_unique;

unique_ptr<snowflake> Sloong::snowflake::Instance = make_unique<snowflake>();

