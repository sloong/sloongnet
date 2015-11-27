#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "userv.h"
#include "univ/log.h"
using namespace std;
int main( int argc, char** args )
{
    int nPort = 9009;
    if ( argc >= 2 )
         nPort = atoi(args[1]);
    if( argc ==3 )
         CLog::g_bDebug = true;
    CLog::showLog(INF,"Start.");
    CLog::showLog(INF,"程序开始运行,准备初始化.");
    SloongWallUS us;
    us.Initialize(nPort);
    CLog::showLog(INF,"程序开始运行.");
    us.Run();
    CLog::showLog(INF,"程序即将退出.");
}

