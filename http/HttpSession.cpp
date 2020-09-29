#include <opencv2/opencv.hpp>
#include "spdlog_helper.h"
#include "Util.h"
#include "HttpSession.h"
#undef min
#undef max

#include "rapidjson/Rjson.hpp"
#include "General_exception2.h"
#include "basic_error.h"
#include "business_package_hdr.h"
//#include "global_vas_conn.h"


std::string& replace_get_string(std::string& target, const std::string& find, const std::string& replace) {

	std::string::size_type pos = std::string::npos;
	while ((pos = target.find(find)) != std::string::npos) {
		target.replace(pos, find.length(), replace);
	}

	return target;
}





//////////////////////////////////////////////////////////////////////////
using namespace LHttp;
//namespace{

static LHttp::HttpSessionServer hss;


//void register_http(const string& url_suffix, const std::function<void(const string& req, string& resp)>& hdl) 
void register_http(const char* url_suffix, http_callback_t http_callback)
{
	hss.AddHandler(url_suffix, [http_callback](const HttpHeader& hh, const string&body, fieldmap_t& rep_hdr, string& rep_body)->int{
			int ret = 200;
			//hdl(body, rep_body);
			if (hh.method == (enum http_method) 3)
			{
				bp_http_hdl_t http_hdl;
				http_callback(body.data(), body.size(), &http_hdl);//获取接口响应返回
				rep_body = http_hdl.resp;
				LOG_INFO(TRIFLE_LOG_BC_PT,"the request content is {}",body.data());

			}
			else if (hh.method == (enum http_method) 1)
			{
			string str = "{\"";
			string str1 = "\"}";
			str += hh.query;
			str += str1;
			replace_get_string(str,"=","\":\"");
			replace_get_string(str,"&","\",\"");
			bp_http_hdl_t http_hdl;
			http_callback(str.data(), str.size(), &http_hdl);//获取接口响应返回
			rep_body = http_hdl.resp;
			//LOG_INFO(TRIFLE_LOG_BC_PT,"the request content is {}",str.data());
			LOG_INFO(TRIFLE_LOG_BC_PT, "the request content is {}", str.data());
			}
			else
			{
				bp_http_hdl_t http_hdl;
				http_callback(body.data(), body.size(), &http_hdl);//获取接口响应返回
				rep_body = http_hdl.resp;
				//LOG_INFO(TRIFLE_LOG_BC_PT,"the request content is {}",body.data());
				LOG_INFO(TRIFLE_LOG_BC_PT, "the request content is {}", body.data());
			}
	rep_hdr["Connection"] = "keep-alive";
		rep_hdr["Keep-Alive"] = "timeout=5,max=100";
		rep_hdr["Content-Type"] = "application/json";
		return ret;
	});
}

void write_resp(bp_http_hdl_t* hdl, const char* resp_data, size_t resp_size)
{
	if (hdl)
	{
		hdl->resp = string(resp_data);
	}
}

void http_service_start(int port)
{
	//RegisterHandlers();
	hss.InitServer(port);
}

void http_service_close()
{
	hss.Close();
}


