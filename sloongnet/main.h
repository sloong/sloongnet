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

#include "defines.h"
#include "EasySync.h"

#include "serverconfig.h"

// define global variables
extern CServerConfig* g_pConfig;

#endif //MAIN_H
