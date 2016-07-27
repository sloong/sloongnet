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
	int fd = open("/usr/local/info.dat", O_RDWR | O_CREAT | O_APPEND);
	void *array[MAX_STACK_LAYERS];
	size_t size;
	char **strings;
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
	backtrace_symbols_fd(array, size, fd);
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

	//SIGPIPE:在reader终止之后写pipe的时候发生
	//SIG_IGN:忽略信号的处理程序
	//SIGCHLD: 进程Terminate或Stop的时候,SIGPIPE会发送给进程的父进程,缺省情况下该Signal会被忽略
	//SIGINT:由Interrupt Key产生,通常是Ctrl+c或者Delete,发送给所有的ForeGroundGroup进程.
	signal(SIGPIPE, SIG_IGN); // this signal should call the socket check function. and remove the timeout socket.
	signal(SIGCHLD, SIG_IGN);
	signal(SIGINT, &on_sigint);
	signal(SIGSEGV, &on_sigint);

	try
	{
		CServerConfig config;
		if (argc >= 2)
		{
			string strCmd(args[1]);
			CUniversal::tolower(strCmd);

			if ((strCmd == "-r" || strCmd == "--r" || strCmd == "run") && (argc >= 3))
			{
				if (access(args[2], ACC_R) == 0)
				{
					try
					{
						config.Initialize(args[2]);
						config.LoadConfig();
					}
					catch (normal_except& e)
					{
						cout << "Error when load config. error meesage is: " << e.what() << endl;
						cout << "if you want run continue with default config, input 'r'." << endl;
						char key;
						cin >> key;
						if (key != 'r' && key != 'R')
							return 0;
					}
				}
				else
				{
					if (access(args[2], ACC_E) == 0)
						cerr << "cannot read file. file path is : " << args[2] << endl;
					else
						cerr << "file not exist. file path is : " << args[2] << endl;
					return -1;
				}
			}
			else if (strCmd != "-rd")
			{
				CCmdProcess::Parser(strCmd);
			}
		}


		SloongWallUS us;
		us.Initialize(&config);
		us.Run();
		SLEEP(1);
	}
	catch (exception& e)
	{
		cout << "exception happened, system will shutdown. message:" << e.what() << endl;
		write_call_stack();
	}
	catch (...)
	{
		cout << "Unhandle exception happened, system will shutdown. "<< endl;
		write_call_stack();
	}
    
	return 0;
}

