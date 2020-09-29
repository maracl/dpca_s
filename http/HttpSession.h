#ifndef _session_h_
#define _session_h_

//#include <string>
#include "httpSessionServer.h"
#include "General_exception2.h"
#include "basic_error.h"
#include "business_package_hdr.h"

//http方法session句柄
struct bp_http_hdl_t
{
	std::string resp;
};

//may throw!
void http_service_start(int port);

void http_service_close();

void register_http(const char* url, http_callback_t cb);
//void register_http(const string& url, const std::function<void(const string& req, string& resp)>& hdl);
void write_resp(bp_http_hdl_t* hdl, const char* resp_data, size_t resp_size);

#endif /* !_session_h_ */
