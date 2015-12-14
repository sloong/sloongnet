#include "utility.h"
#include <unistd.h>	// for sleep
#include <stdio.h>	// for FILE open and close
#include <fstream>
#include <string.h> // for stricmp
#include <openssl/md5.h>
using namespace std;
using namespace Sloong;


// Encoding lookup table
char base64encode_lut[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
	'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
	'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
	'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '=' };
// Decode lookup table
char base64decode_lut[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 62, 0, 0, 0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };




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
	int n;
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

string Sloong::CUtility::MD5_Encoding(string str, bool bFile /*= false*/)
{
	unsigned char md[16];
	char tmp[3] = { '\0' }, md5buf[33] = { '\0' };
	if ( bFile )
	{
		FILE *fd = fopen(str.c_str(), "r");
		MD5_CTX c;
		int len;
		unsigned char buffer[1024] = { '\0' };
		MD5_Init(&c);
		while (0 != (len = fread(buffer, 1, 1024, fd)))
		{
			MD5_Update(&c, buffer, len);
		}
		MD5_Final(md, &c);
		fclose(fd);
	}
	else
	{
		MD5((unsigned char *)str.c_str(), str.length(), md);
	}

	for (int i = 0; i < 16; i++)
	{
		sprintf(tmp, "%02X", md[i]);
		strcat(md5buf, tmp);
	}
	return md5buf;
}



string Sloong::CUtility::Base64_Encoding(string str)
{
	int i = 0, slen = str.size();
	char* dest = new char[slen + 1];
	const char* src = str.c_str();
	for (i = 0; i < slen; i += 3, src += 3)
	{ // Enc next 4 characters
		*(dest++) = base64encode_lut[(*src & 0xFC) >> 0x2];
		*(dest++) = base64encode_lut[(*src & 0x3) << 0x4 | (*(src + 1) & 0xF0) >> 0x4];
		*(dest++) = ((i + 1) < slen) ? base64encode_lut[(*(src + 1) & 0xF) << 0x2 | (*(src + 2) & 0xC0) >> 0x6] : '=';
		*(dest++) = ((i + 2) < slen) ? base64encode_lut[*(src + 2) & 0x3F] : '=';
	}
	*dest = '\0'; // Append terminator
	return dest;
}

string Sloong::CUtility::Base64_Decoding(string str)
{
	int i = 0, slen = str.size();
	char* dest = new char[slen + 1];
	const char* src = str.c_str();
	for (i = 0; i < slen; i += 4, src += 4)
	{ // Store next 4 chars in vars for faster access
		char c1 = base64decode_lut[*src], c2 = base64decode_lut[*(src + 1)], c3 = base64decode_lut[*(src + 2)], c4 = base64decode_lut[*(src + 3)];
		// Decode to 3 chars
		*(dest++) = (c1 & 0x3F) << 0x2 | (c2 & 0x30) >> 0x4;
		*(dest++) = (c3 != 64) ? ((c2 & 0xF) << 0x4 | (c3 & 0x3C) >> 0x2) : '\0';
		*(dest++) = (c4 != 64) ? ((c3 & 0x3) << 0x6 | c4 & 0x3F) : '\0';
	}
	*dest = '\0'; // Append terminator
	return dest;
}
