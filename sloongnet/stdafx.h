#ifndef SLOONGNET_STD_AFX_H
#define SLOONGNET_STD_AFX_H

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

#endif