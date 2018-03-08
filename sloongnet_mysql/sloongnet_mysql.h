#pragma once

#define SLOONGNET_MYSQL_METHOD_NAME "class_sloongnet_mysql"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <string>
#include <univ/exception.h>
#include <univ/log.h>
using namespace std;
using namespace Sloong::Universal;