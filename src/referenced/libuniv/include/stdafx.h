// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//
#ifndef STDAFX_H
#define STDAFX_H

#include "targetver.h"

#include <string>
using namespace std;
#ifdef _WINDOWS
	#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
	// Windows 头文件: 
	#include <windows.h>
	#include <tchar.h>
	#include <assert.h>
	#include <time.h>
	#include <stdio.h>
	#include <fstream>
	#include <windows.h>
	#include <winerror.h>
	#include <vector>
	#pragma comment(lib,"libeay32.lib")
	#pragma comment(lib,"lua52.lib")
#else
	#include <stdarg.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <wchar.h>
	#include <vector>
    #include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <semaphore.h>
#endif
#include "univ.h"
#endif //STDAFX_H

