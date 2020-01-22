#include "utility.h"
#include <unistd.h>	// for sleep
#include <stdio.h>	// for FILE open and close
#include <fstream>
#include <string.h> // for stricmp
#include<univ/exception.h>
#include <uuid/uuid.h>
using namespace std;
using namespace Sloong;
#ifdef _WINDOWS  

#else

#endif  



CUtility::CUtility()
{
}

typedef struct PACKEDCPU
{
	char name[20];      //定义一个char类型的数组名name有20个元素
	unsigned int user; //定义一个无符号的int类型的user
	unsigned int nice; //定义一个无符号的int类型的nice
	unsigned int system;//定义一个无符号的int类型的system
	unsigned int idle; //定义一个无符号的int类型的idle
}CPU_OCCUPY;


int CUtility::GetMemory(int& total, int& free)
{
	ifstream ofs;
	ofs.open("/proc/meminfo");

	int num;
	char name[256];
	char unit[10];
	while (!ofs.eof())
	{

		ofs >> name >> num >> unit;
		if (strcasecmp(name, "memtotal:") == 0)
		{
			total = num;
		}
		else if (strcasecmp(name, "memfree:") == 0)
		{
			free = num;
			break;
		}
		else
		{
			continue;
		}
	}

	ofs.close();
	return 0;
}

int cal_cpuoccupy(CPU_OCCUPY *o, CPU_OCCUPY *n)
{
	unsigned long od, nd;
	unsigned long id, sd;
	int cpu_use = 0;

	od = (unsigned long)(o->user + o->nice + o->system + o->idle);//第一次(用户+优先级+系统+空闲)的时间再赋给od
	nd = (unsigned long)(n->user + n->nice + n->system + n->idle);//第二次(用户+优先级+系统+空闲)的时间再赋给od

	id = (unsigned long)(n->user - o->user);    //用户第一次和第二次的时间之差再赋给id
	sd = (unsigned long)(n->system - o->system);//系统第一次和第二次的时间之差再赋给sd
	if ((nd - od) != 0)
		cpu_use = (int)((sd + id) * 10000) / (nd - od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
	else cpu_use = 0;
	//printf("cpu: %u\n",cpu_use);
	return cpu_use;
}

void get_cpuoccupy(CPU_OCCUPY *cpust) //对无类型get函数含有一个形参结构体类弄的指针O
{
	FILE *fd;
	char buff[256];
	CPU_OCCUPY *cpu_occupy;
	cpu_occupy = cpust;

	fd = fopen("/proc/stat", "r");
	fgets(buff, sizeof(buff), fd);

	sscanf(buff, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle);

	fclose(fd);
}


int CUtility::GetCpuUsed(double nWaitTime)
{
	CPU_OCCUPY cpu_stat1;
	CPU_OCCUPY cpu_stat2;
	int cpu;

	get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);
	SLEEP(nWaitTime);

	get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);

	cpu = cal_cpuoccupy((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);
	return cpu;
}

string Sloong::CUtility::GetSocketIP(int socket)
{
	struct sockaddr_in add;
	int nSize = sizeof(add);
	memset(&add, 0, sizeof(add));
	getpeername(socket, (sockaddr*)&add, (socklen_t*)&nSize);
	return string(inet_ntoa(add.sin_addr));
}

int Sloong::CUtility::GetSocketPort(int socket)
{
	struct sockaddr_in add;
	int nSize = sizeof(add);
	memset(&add, 0, sizeof(add));
	getpeername(socket, (sockaddr*)&add, (socklen_t*)&nSize);
	return add.sin_port;
}

string Sloong::CUtility::GetSocketAddress(int socket)
{
	struct sockaddr_in add;
	int nSize = sizeof(add);
	memset(&add, 0, sizeof(add));
	getpeername(socket, (sockaddr*)&add, (socklen_t*)&nSize);
	return CUniversal::Format("%s:%d", inet_ntoa(add.sin_addr), add.sin_port);
}

int Sloong::CUtility::ReadFile(string filepath, char*& pBuffer)
{
    if( -1 == access(filepath.c_str(),R_OK))
        throw normal_except("File no exit or cannot read.file path is:" + filepath);
	ifstream in(filepath.c_str(), ios::in | ios::binary);
	streampos  pos = in.tellg();
	in.seekg(0, ios::end);
	int nSize = in.tellg();
	in.seekg(pos);
	pBuffer = new char[nSize];
	in.read(pBuffer, nSize);
	in.close();
	return nSize;
}

string Sloong::CUtility::GenUUID()
{
	char uuid[37] = { 0 };
	uuid_t uu;
	uuid_generate(uu);
	uuid_unparse(uu, uuid);
	return uuid;
}



#define MAX_STACK_LAYERS 256

void Sloong::CUtility::write_call_stack()
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
