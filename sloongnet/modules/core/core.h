/*
 * @Author: WCB
 * @Date: 1970-01-01 08:00:00
 * @LastEditors: WCB
 * @LastEditTime: 2020-04-16 16:34:18
 * @Description: file content
 */
#ifndef SLOONGNET_MODULE_CORE_CORE_H
#define SLOONGNET_MODULE_CORE_CORE_H


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

// Boost
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

// std c++ file
#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <mutex>


// openssl head file
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

#include "defines.h"
#include "result.hpp"
#include "EasySync.hpp"
#include "queue_ex.hpp"
#include "map_ex.hpp"
#include "list_ex.hpp"

using namespace Sloong;


#endif //SLOONGNET_MODULE_CORE_CORE_H
