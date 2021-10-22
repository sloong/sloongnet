/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-10-20 14:55:32
 * @LastEditTime: 2021-10-20 14:55:32
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/std.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description:
 */
/***
 * @Author: Chuanbin Wang - wcb@sloong.com
 * @Date: 2021-10-09 18:00:45
 * @LastEditTime: 2021-10-09 18:00:52
 * @LastEditors: Chuanbin Wang
 * @FilePath: /engine/src/modules/core/std.h
 * @Copyright 2015-2020 Sloong.com. All Rights Reserved
 * @Description:
 */

#pragma once

// std c head file
#include <errno.h> // for errno
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// linux head file
#include <execinfo.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <sys/types.h>

// std c++ file
#include <atomic>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
using namespace std;

#include <fmt/chrono.h>
#include <fmt/format.h>
using fmt::format;

#include <spdlog/spdlog.h>

#include <jsoncpp/json/json.h>
