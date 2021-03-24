#include "utility.h"
#include <unistd.h> // for sleep
#include <stdio.h>	// for FILE open and close
#include <fstream>
#include <string.h> // for stricmp
#include <uuid/uuid.h>
#include <execinfo.h>
// For hostname
#include <netdb.h>
#include <sys/socket.h>
using namespace std;
using namespace Sloong;
#ifdef _WINDOWS

#else

#endif

int CUtility::GetMemory(int &total, int &free)
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

void CUtility::RecordCPUStatus(CPU_OCCUPY *cpust)
{
	FILE *fd;
	char buff[256];

	fd = fopen("/proc/stat", "r");
	fgets(buff, sizeof(buff), fd);

	sscanf(buff, "%s %u %u %u %u", cpust->name, &cpust->user, &cpust->nice, &cpust->system, &cpust->idle);

	fclose(fd);
}

double CUtility::CalculateCPULoad(CPU_OCCUPY *prev)
{
	CPU_OCCUPY cur;
	RecordCPUStatus(&cur);

	unsigned long od, nd;
	unsigned long id, sd;
	double cpu_use = 0.0;

	od = (unsigned long)(prev->user + prev->nice + prev->system + prev->idle); //第一次(用户+优先级+系统+空闲)的时间再赋给od
	nd = (unsigned long)(cur.user + cur.nice + cur.system + cur.idle);		   //第二次(用户+优先级+系统+空闲)的时间再赋给od

	id = (unsigned long)(cur.user - prev->user);	 //用户第一次和第二次的时间之差再赋给id
	sd = (unsigned long)(cur.system - prev->system); //系统第一次和第二次的时间之差再赋给sd
	if ((nd - od) != 0)
		cpu_use = ((sd + id) * 10000) / (nd - od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
	else
		cpu_use = 0;
	return cpu_use;
}

string Sloong::CUtility::GetSocketIP(int socket)
{
	struct sockaddr_in add;
	int nSize = sizeof(add);
	memset(&add, 0, sizeof(add));
	getpeername(socket, (sockaddr *)&add, (socklen_t *)&nSize);
	return string(inet_ntoa(add.sin_addr));
}

int Sloong::CUtility::GetSocketPort(int socket)
{
	struct sockaddr_in add;
	int nSize = sizeof(add);
	memset(&add, 0, sizeof(add));
	getpeername(socket, (sockaddr *)&add, (socklen_t *)&nSize);
	return add.sin_port;
}

string Sloong::CUtility::GetSocketAddress(int socket)
{
	struct sockaddr_in add;
	int nSize = sizeof(add);
	memset(&add, 0, sizeof(add));
	getpeername(socket, (sockaddr *)&add, (socklen_t *)&nSize);
	return Helper::Format("%s:%d", inet_ntoa(add.sin_addr), add.sin_port);
}

unique_ptr<char[]> Sloong::CUtility::ReadFile(const string &filepath, int *out_size)
{
	if (-1 == access(filepath.c_str(), R_OK))
	{
		*out_size = -1;
		return nullptr;
	}

	ifstream in(filepath.c_str(), ios::in | ios::binary);
	streampos pos = in.tellg();
	in.seekg(0, ios::end);
	int nSize = in.tellg();
	in.seekg(pos);
	auto pBuf = make_unique<char[]>(nSize);
	in.read(pBuf.get(), nSize);
	in.close();
	*out_size = nSize;
	return pBuf;
}

uint64_t Sloong::CUtility::CityEncodeFile(const string& path)
{
	if (-1 == access(path.c_str(), R_OK))
		return 0;
	ifstream in(path.c_str(), ios::in | ios::binary);
	streampos pos = in.tellg();
	in.seekg(0, ios::end);
	int nSize = in.tellg();
	in.seekg(pos);
	string out;
	out.resize(nSize);
	in.read(out.data(), nSize);
	in.close();

	return CCity::Encode64(out);
}

uint32_t Sloong::CUtility::CRC32EncodeFile(const string& path)
{
	if (-1 == access(path.c_str(), R_OK))
		return 0;
	ifstream in(path.c_str(), ios::in | ios::binary);
	streampos pos = in.tellg();
	in.seekg(0, ios::end);
	int nSize = in.tellg();
	in.seekg(pos);
	string out;
	out.resize(nSize);
	in.read(out.data(), nSize);
	in.close();

	return CRC::Calculate(out.c_str(), out.length(), CRC::CRC_32());
}

string Sloong::CUtility::SHA1EncodeFile(const string& path)
{
	unsigned char t[Sloong::Universal::SHA1_LENGTH] = {0};
	CSHA1::Binary_Encoding(path, t, true );
	return string((char*)t,Sloong::Universal::SHA1_LENGTH);
}

string Sloong::CUtility::SHA256EncodeFile(const string& path)
{
	unsigned char t[Sloong::Universal::SHA256_LENGTH] = {0};
	CSHA256::Binary_Encoding(path, t, true );
	return Helper::BinaryToHex(t,Sloong::Universal::SHA256_LENGTH);
}

CResult Sloong::CUtility::WriteFile(const string &filepath, const char *pBuffer, int size)
{
	return CResult::Make_Error("No realize.");
}

string Sloong::CUtility::GenUUID()
{
	char uuid[37] = {0};
	uuid_t uu;
	uuid_generate(uu);
	uuid_unparse(uu, uuid);
	return uuid;
}

#define MAX_STACK_LAYERS 20

string Sloong::CUtility::GetCallStack()
{
	string strRet("");
	void *array[MAX_STACK_LAYERS];
	auto size = backtrace(array, MAX_STACK_LAYERS);
	auto strings = backtrace_symbols(array, size);
	for (int i = 0; i < size; i++)
	{
		strRet.append(strings[i]);
		if (i < size - 1)
			strRet.append(1, '\n');
	}
	free(strings);
	return strRet;
}

VStrResult CUtility::IPToHostName(const string &ip)
{
	if (ip.length() == 0)
	{
		return VStrResult::Make_Error("invalid params");
	}

	struct addrinfo hints;
	struct addrinfo *addr_res;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME | AI_NUMERICHOST;
	hints.ai_protocol = 0;

	if (auto res = getaddrinfo(ip.c_str(), nullptr, &hints, &addr_res) != 0)
	{
		return VStrResult::Make_Error(Helper::Format("getaddrinfo: %s", gai_strerror(res)));
	}

	vector<string> list;
	string errmsg;
	for (auto res_p = addr_res; res_p != nullptr; res_p = res_p->ai_next)
	{
		char host[1024] = {0};
		auto ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NAMEREQD);
		if (ret != 0)
		{
			errmsg += Helper::Format("getaddrinfo: %s", gai_strerror(ret));
		}
		else
		{
			list.push_back(host);
		}
	}

	freeaddrinfo(addr_res);
	if( list.size() > 0 )
		return VStrResult::Make_OKResult(list,errmsg);
	else
		return VStrResult::Make_Error(errmsg);
}

VStrResult CUtility::HostnameToIP(const string &hostname)
{
	if (hostname.length() == 0)
	{
		return VStrResult::Make_Error("invalid params");
	}

	struct addrinfo hints;
	struct addrinfo *hostname_res;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;
	hints.ai_protocol = 0;

	if (auto res = getaddrinfo(hostname.c_str(), NULL, &hints, &hostname_res) != 0)
	{
		return VStrResult::Make_Error(Helper::Format("getaddrinfo: %s", gai_strerror(res)));
	}

	vector<string> list;
	string errmsg;
	for (auto res_p = hostname_res; res_p != NULL; res_p = res_p->ai_next)
	{
		char host[1024] = {0};
		auto ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
		if (ret != 0)
		{
			errmsg += Helper::Format("getaddrinfo: %s", gai_strerror(ret));
		}
		else
		{
			list.push_back(host);
		}
	}

	freeaddrinfo(hostname_res);
	if( list.size() > 0 )
		return VStrResult::Make_OKResult(list,errmsg);
	else
		return VStrResult::Make_Error(errmsg);
}
