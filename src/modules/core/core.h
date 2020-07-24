/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-05-13 16:01:24
 * @Description: file content
 */
#pragma once

// std c head file
#include <errno.h> // for errno
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// linux head file
#include <sys/times.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <execinfo.h>
#include <fcntl.h>

// std c++ file
#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <mutex>
using namespace std;

#include "protocol/base.pb.h"
using namespace Base;

#include "protocol/core.pb.h"
using namespace Core;

#include <jsoncpp/json/json.h>

#include "defines.h"
#include "result.hpp"
#include "queue_ex.hpp"
#include "map_ex.hpp"
#include "list_ex.hpp"
#include "package.hpp"

using namespace Sloong;
