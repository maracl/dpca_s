#ifndef _CREATE_UUID_H_
#define	_CREATE_UUID_H_

#include <string>

#ifdef WIN32
#include <objbase.h>
#else
#include <uuid/uuid.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
typedef struct _GUID
{
	unsigned long Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
}GUID;
#endif

inline std::string create_uuid(bool splitter)
{
	GUID guid;
#ifdef WIN32
	CoCreateGuid(&guid);
#else
	uuid_generate(reinterpret_cast<unsigned char *>(&guid));
#endif

	std::string format;
	if (splitter)
		format = "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";
	else
		format = "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X";

	char buf[64] = { 0 };
#ifdef __GNUC__
	snprintf(
#else
	_snprintf_s(
#endif
		buf,
		sizeof(buf),
		format.c_str(),
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	return std::string(buf);
}

#endif	/* _CREATE_UUID_H_ */

