#ifndef _str_platform_system_h_
#define _str_platform_system_h_

#if defined(OS_WINDOWS)
#include <Windows.h>

typedef HMODULE module_t;
typedef DWORD   useconds_t;
typedef FARPROC funcptr_t;

#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef void* module_t;
typedef void (*funcptr_t)(void);
#endif

#if defined(OS_MAC)
#include <stdint.h>
#include <mach/mach_time.h>  
#endif

#include <sys/timeb.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>
using std::string;
using std::vector;

inline void system_sleep(useconds_t millisecond);

// Get CPU count
inline size_t system_getcpucount(void);
inline long long system_getcyclecount(void);
inline float system_getcpuusage(void);

///@return second.milliseconds(absolute time)
inline double system_time(void);
///@return milliseconds(relative time)
inline size_t system_clock(void);

inline long long system_timestamp(void);

inline int system_version(int* major, int* minor);

inline void split(vector<string> &vecElems, const string &str, char delim);
//////////////////////////////////////////////////////////////////////////
///
/// dynamic module load/unload
///
//////////////////////////////////////////////////////////////////////////
inline module_t system_load(const char* module);
inline int system_unload(module_t module);
inline funcptr_t system_getproc(module_t module, const char* producer);

//////////////////////////////////////////////////////////////////////////
///
/// implement
///
//////////////////////////////////////////////////////////////////////////
inline void system_sleep(useconds_t milliseconds)
{
#if defined(OS_WINDOWS)
	Sleep(milliseconds);
#else
	usleep(milliseconds*1000);
#endif
}

inline void split(vector<string> &vecElems, const string &str, char delim)
{
	vecElems.clear();
	std::istringstream ss(str);
	while (!ss.eof())
	{
		string x;               // here's a nice, empty string
		getline(ss, x, delim);    // try to read the next field into it
		if (!x.empty())
		{
			vecElems.push_back(x);
		}
	}
}

inline size_t system_getcpucount(void)
{
#if defined(OS_WINDOWS)
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;

#elif defined(OS_MAC) || defined(_FREEBSD_) || defined(_NETBSD_) || defined(_OPENBSD_)
	// FreeBSD, MacOS X, NetBSD, OpenBSD, etc.:
	int mib[4];
	size_t num;
	size_t len;

	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU; // alternatively, try HW_NCPU;

	len = sizeof(num);
	sysctl(mib, 2, &num, &len, NULL, 0);
	if(num < 1)
	{
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &num, &len, NULL, 0);

		if(num < 1)
			num = 1;
	}
	return num;

#elif defined(_HPUX_)
	// HPUX:
	return mpctl(MPC_GETNUMSPUS, NULL, NULL);

#elif defined(_IRIX_)
	// IRIX:
	return sysconf(_SC_NPROC_ONLN);

#else
	// linux, Solaris, & AIX
	return sysconf(_SC_NPROCESSORS_ONLN);

	//"cat /proc/cpuinfo | grep processor | wc -l"
#endif
}

inline long long system_getcyclecount(void)
{
#if defined(OS_WINDOWS)
	LARGE_INTEGER freq;
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	QueryPerformanceFrequency(&freq);
#else
	return 0;
#endif
}

inline float system_getcpuusage(void)
{
	static int usage = 0;
#if defined(OS_WINDOWS)
	static UINT64 last_user, last_kernel, last_idle;
	UINT64 user, kernel, idle;
	UINT64 usr, ker, idl, sys;
	FILETIME idleTime, kernelTime, userTime;

	BOOL res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
	if(!res)
		return usage;

	user = userTime.dwHighDateTime;
	kernel = kernelTime.dwHighDateTime;
	idle = idleTime.dwHighDateTime;

	user = (user << 32) + userTime.dwLowDateTime;
	kernel = (kernel << 32) + kernelTime.dwLowDateTime;
	idle = (idle << 32) + idleTime.dwLowDateTime;

	usr = user - last_user;
	ker = kernel - last_kernel;
	idl = idle - last_idle;

	last_user = user;
	last_kernel = kernel;
	last_idle = idle;

	sys = ker + usr;
	if(sys == 0) //在多线程调用该函数时有可能出现
		return usage;

	usage = (float)(sys - idl)*100 / sys;
#else
	vector<string> vecItems;

	FILE *file;
	char line[100];
	file = popen("vmstat", "r");
	if (NULL != file)
	{
		int i = 0;
		while (fgets(line, 100, file) != NULL)
		{
			vecItems.push_back(line);
		}
	}

	string strState = vecItems.at(vecItems.size() - 1);
	vector<string> vecStr;
	split(vecStr, strState, ' ');
	if (vecStr.size() >= 3)
	{
		//int index = vecStr.size() - 3;
		int index = vecStr.size() - 3;//需要验证下
		usage = 100.0 - stof(vecStr[index]);
	}
	pclose(file);
#endif

	if(usage < 0)
		usage = 0.0;
	else if(usage > 100)
		usage = 100.0;

	return usage;
}

inline int system_getmemusage(void)
{
	static int usage = 0;

#if defined(OS_WINDOWS)
	MEMORYSTATUS ms;
	INT64 total, avail;
	GlobalMemoryStatus(&ms);
	total = ms.dwTotalPhys;
	avail = ms.dwAvailPhys;

#else
	unsigned int total, avail;
	char buff[256];
	FILE *fd;
	int ftotal = 0;
	int favail = 0;

	fd = fopen ("/proc/meminfo", "r");
	if(!fd)
		return usage;

	while(fgets(buff, sizeof(buff), fd) != NULL)
	{
		char *p = strstr(buff, "MemTotal:");
		if(p)
		{
			p += strlen("MemTotal:");
			while(*p == '\x20' || *p == '\t')
				++p;

			total = atoi(p);
			ftotal = 1;
			continue;
		}

		p = strstr(buff, "MemFree:");
		if(p)
		{
			p += strlen("MemFree:");
			while(*p == '\x20' || *p == '\t')
				++p;

			avail = atoi(p);
			favail = 1;
			continue;
		}

		if(ftotal && favail)
			break;
	}

	fclose(fd);

#endif

	if(total > 0)
		usage = (int)((total - avail) * 100 / total);

	if(usage < 0)
		usage = 0;
	else if(usage > 100)
		usage = 100;

	return usage;
}

inline void system_getmeminfo(uint64_t &total, uint64_t &freed , uint64_t &used)
{
	static int usage = 0;

#if defined(OS_WINDOWS)
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	total = ms.dwTotalPhys;
	avail = ms.dwAvailPhys;

#else
	char buff[256];
	FILE *fd;
	int ftotal = 0;
	int fslab = 0;
	int ffreed = 0;
	int fbuffer = 0;
	int fcached = 0;
	uint64_t buffers, catched, slab;
	int cacheNum = 0;

	fd = fopen("/proc/meminfo", "r");
	if (!fd)
		return;

	while (fgets(buff, sizeof(buff), fd) != NULL)
	{
		char *p = strstr(buff, "MemTotal:");
		if (p)
		{
			p += strlen("MemTotal:");
			while (*p == '\x20' || *p == '\t')
				++p;

			total = atoi(p);
			ftotal = 1;
			continue;
		}

		p = strstr(buff, "MemFree:");
		if (p)
		{
			p += strlen("MemFree:");
			while (*p == '\x20' || *p == '\t')
				++p;

			freed = atoi(p);
			ffreed = 1;
			continue;
		}

		p = strstr(buff, "Buffers:");
		if (p)
		{
			p += strlen("Buffers:");
			while (*p == '\x20' || *p == '\t')
				++p;

			buffers = atoi(p);
			fbuffer = 1;
			continue;
		}

		p = strstr(buff, "Cached:");
		if (p)
		{
			if (cacheNum % 2 != 0)
			{
				cacheNum++;
				continue;
			}
			p += strlen("Cached:");
			while (*p == '\x20' || *p == '\t')
				++p;

			catched = atoi(p);
			fcached = 1;
			cacheNum++;
			continue;
		}

		p = strstr(buff, "Slab:");
		if (p)
		{
			p += strlen("Slab:");
			while (*p == '\x20' || *p == '\t')
				++p;

			slab = atoi(p);
			fslab = 1;
			continue;
		}
		

		if (ftotal && ffreed && fslab && fbuffer && fcached)
			break;
	}
	used = total - freed - buffers - catched - slab;
	fclose(fd);

#endif
}

///@return second.milliseconds(absolute time)
inline double system_time(void)
{
	struct timeb t;
	ftime(&t);
	return t.time+t.millitm*0.001;
}

///@return milliseconds(relative time)
inline size_t system_clock(void)
{
#if defined(OS_WINDOWS)
	LARGE_INTEGER freq;
	LARGE_INTEGER count;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&count);
	return (size_t)(count.QuadPart * 1000 / freq.QuadPart);
#elif defined(OS_MAC)
	uint64_t tick;
	mach_timebase_info_data_t timebase;
	tick = mach_absolute_time();
	mach_timebase_info(timebase);
	return (size_t)(tick * timebase.numer / timebase.denom / 1000000);
#elif defined(OS_LINUX)
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (size_t)(tp.tv_sec*1000 + tp.tv_nsec/1000000);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000 + tv.tv_usec / 1000;
#endif
}

#if defined(OS_MAC)
#include <sys/param.h>
#include <sys/sysctl.h>
#include <mach/mach_time.h>
inline int mac_gettime(struct timespec *t){
	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	uint64_t time;
	time = mach_absolute_time();
	double nseconds = ((double)time * (double)timebase.numer)/((double)timebase.denom);
	double seconds = ((double)time * (double)timebase.numer)/((double)timebase.denom * 1e9);
	t->tv_sec = seconds;
	t->tv_nsec = nseconds;
	return 0;
}
#endif

inline long long system_timestamp(void)
{
	long long system_ts = 0;
#ifdef OS_LINUX
	struct timeval tv;
	gettimeofday(&tv, NULL);
	system_ts = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif

	return system_ts;
}

inline int system_version(int* major, int* minor)
{
#if defined(OS_WINDOWS)
	/*
	Operating system		Version number 
	Windows 8				6.2
	Windows Server 2012		6.2
	Windows 7				6.1
	Windows Server 2008 R2	6.1
	Windows Server 2008		6.0 
	Windows Vista			6.0 
	Windows Server 2003 R2	5.2 
	Windows Server 2003		5.2 
	Windows XP				5.1 
	Windows 2000			5.0 
	Windows Me				4.90 
	Windows 98				4.10 
	Windows NT 4.0			4.0 
	Windows 95				4.0 
	*/
	OSVERSIONINFO version;
	memset(&version, 0, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);
	GetVersionEx(&version);
	*major = (int)(version.dwMajorVersion);
	*minor = (int)(version.dwMinorVersion);
	return 0;
#else
	struct utsname ver;
	if(0 != uname(&ver))
		return errno;
	if(2!=sscanf(ver.release, "%8d.%8d", major, minor))
		return -1;
	return 0;
#endif
}

inline module_t system_load(const char* module)
{
#if defined(OS_WINDOWS)
	return LoadLibraryExA(module, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
	return dlopen(module, RTLD_LAZY|RTLD_LOCAL);
#endif
}

inline int system_unload(module_t module)
{
#if defined(OS_WINDOWS)
	return FreeLibrary(module);
#else
	return dlclose(module);
#endif
}

inline funcptr_t system_getproc(module_t module, const char* producer)
{
#if defined(OS_WINDOWS)
	return GetProcAddress(module, producer);
#else
	return (funcptr_t)dlsym(module, producer);
#endif
}

#endif /* !_platform_system_h_ */
