// mian.h files
#ifndef MAIN_H
#define MAIN_H

// std c head file
#include <errno.h> // for errno
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// linux head file
#include <sys/times.h> 

// std c++ file
#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <mutex>
using namespace std;

// openssl head file
#include <openssl/ssl.h>
#include <openssl/err.h>

// univ head file
#include <univ/defines.h>
#include <univ/univ.h>
#include <univ/log.h>
#include <univ/exception.h>
#include <univ/threadpool.h>
#include <univ/hash.h>
#include <univ/lua.h>
#include <univ/luapacket.h>
using namespace Sloong;
using namespace Sloong::Universal;

#include "defines.h"
#include "SmartSync.h"

#endif //MAIN_H
