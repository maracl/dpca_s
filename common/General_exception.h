#ifndef _GENERAL_EXCEPTION_H_
#define _GENERAL_EXCEPTION_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <string.h>

class GeneralException {
protected:
	int _err_code;
	::std::string _err_msg;
public:
	GeneralException(int errc):_err_code(errc) {}
	GeneralException(int errc,const char* errmsg):_err_code(errc),_err_msg(errmsg) {}
	GeneralException(int errc,const ::std::string& errs):_err_code(errc),_err_msg(errs) {}
	//GeneralException(int errc,const char* errmsg,...):_err_code(errc),_err_msg(errmsg) {}
	virtual ~GeneralException() {}


	GeneralException& format_errmsg(const char* format,...) {
		va_list args;
		va_start( args, format );
#ifdef _WIN32
		_err_msg.resize(_vscprintf( format, args )); // _vscprintf doesn't count terminating '\0'
		vsprintf_s((char*)_err_msg.data(),_err_msg.size()+1, format, args );
#else
		char* ps=0;
		int rt=vasprintf(&ps,format,args);
		if(rt>=0) {
			_err_msg.assign(ps,rt); free(ps);
		}
#endif
		return *this;
	}

	int err_code() const {
		return _err_code;
	}

	const char* err_str() const {
		return _err_msg.data();
	}

	const ::std::string& err_msg() const {
		return _err_msg;
	}
};

#define THROWABLE(x) x


static inline THROWABLE(void) Check0Throw(int result,const char* msg="") {
	if(result!=0) {
		throw GeneralException(result,msg);
	}
}

#ifdef __linux
#include <errno.h>

static inline std::string system_errmsg() {
	char buff[456] = { 0 };
	int ec=errno;
	sprintf(buff,"errno=%d,msg=%s",ec,strerror(ec));
	return buff;
}

#else
#include <WS2tcpip.h>

static inline std::string system_errmsg() {
	char buff[456] = { 0 };
	DWORD dwErr = GetLastError(), off = (DWORD)strlen(buff);
	sprintf_s(buff, "LastError=%u,Msg=", dwErr);
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErr, 0, buff + off, sizeof(buff)-off, NULL);
	return buff;
}

#endif



#endif
