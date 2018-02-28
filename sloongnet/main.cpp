#include <string.h>
#include <stdarg.h>
#include "main.h"
#include "userv.h"
#include "univ/log.h"
#include "serverconfig.h"
#include "version.h"
#include "jpeg.h"
#include "univ/exception.h"
#include "CmdProcess.h"
#include <execinfo.h>
#include <stdio.h>
#include <fcntl.h>
using namespace std;
using namespace Sloong;

#define MAX_STACK_LAYERS 256

void write_call_stack()
{
	/*int fd = open("/usr/local/info.dat", O_RDWR | O_CREAT | O_APPEND);
	void *array[MAX_STACK_LAYERS];
	size_t size;
	char **strings;*/
	/*//string strRet("");
	size = backtrace(array, MAX_STACK_LAYERS);
	strings = backtrace_symbols(array, size);
	for (int i = 0; i < size; i++)
	{
	if (fd > 0)
	{
	write(fd, strings[i], strlen(strings[i]));
	write(fd, "\n", 1);
	}
	//strRet.append(strings[i]);
	//if (i < size - 1)
	//strRet.append(1, '\n');
	}
	if (fd > 0)
	write(fd, "\n", 1);

	free(strings);*/
	//backtrace_symbols_fd(array, size, fd);
}


void sloong_terminator() 
{
	cout << "Unkonw error happened, system will shutdown. " << endl;
	write_call_stack();
	exit(0);
}

void on_sigint(int signal)
{
	cout << "Unhandle signal happened, system will shutdown. signal:" << signal<< endl;
	write_call_stack();
	exit(0);
}

int main( int argc, char** args )
{
	set_terminate(sloong_terminator);
	set_unexpected(sloong_terminator);

	//SIG_IGN:忽略信号的处理程序
	//SIGPIPE:在reader终止之后写pipe的时候发生
	signal(SIGPIPE, SIG_IGN); // this signal should call the socket check function. and remove the timeout socket.
	//SIGCHLD: 进程Terminate或Stop的时候,SIGPIPE会发送给进程的父进程,缺省情况下该Signal会被忽略
	signal(SIGCHLD, SIG_IGN);
	//SIGINT:由Interrupt Key产生,通常是Ctrl+c或者Delete,发送给所有的ForeGroundGroup进程.
	signal(SIGINT, &on_sigint);
	signal(SIGSEGV, &on_sigint);

	try
	{
		CServerConfig config;
		// CmdProcess会根据参数来加载正确的配置信息。成功返回true。
		if (CCmdProcess::Parser(argc, args, &config))
		{
			// 成功加载后即创建UServer对象，并开始运行。
			SloongWallUS us;
			us.Initialize(&config);
			// Run函数会阻塞运行。
			us.Run();
		}
	}
	catch (exception& e)
	{
		cout << "exception happened, system will shutdown. message:" << e.what() << endl;
	}
	catch(normal_except& e)
    {
        cout << "exception happened, system will shutdown. message:" << e.what() << endl;
    }
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. "<< endl;
		write_call_stack();
	}
	
	return 0;
}

