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
#include <direct.h>  
#include <io.h>  
#else
#include <stdarg.h>  
#include <sys/stat.h>  
#endif  

#ifdef _WINDOWS  
#define ACCESS _access  
#define MKDIR(a) _mkdir((a))  
#else 
#define ACCESS access  
#define MKDIR(a) mkdir((a),0755)  
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


int CUtility::GetCpuUsed(int nWaitTime)
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

string Sloong::CUtility::CheckFileDirectory(string filePath)
{
	if (filePath == "")
	{
		return "";
	}

	int iLen = filePath.size();
	char* pszDir = new char[iLen+1];
	memset(pszDir, 0, iLen+1);
	memcpy(pszDir, filePath.c_str(), iLen);
	string strDir;
	int iRet;
	// 创建中间目录  
	for (int i = 1; i < iLen; i++)
	{
		if (pszDir[i] == '\\' || pszDir[i] == '/')
		{
			pszDir[i] = '\0';
			strDir = pszDir;

			//如果不存在,创建  
			iRet = ACCESS(pszDir, 0);
			if (iRet != 0)
			{
				iRet = MKDIR(pszDir);
				if (iRet != 0)
				{
					return pszDir;
				}
			}
			//支持linux,将所有\换成/  
			pszDir[i] = '/';
		}
	}

	SAFE_DELETE_ARR(pszDir);
	return strDir;
}


