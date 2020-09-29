#include <opencv2/opencv.hpp>
#include "spdlog_helper.h"
#include "Util.h"
#include "HttpSession.h"
#undef min
#undef max

//#include "Rjson.hpp"
#include "rapidjson/Rjson.hpp"
#include "General_exception2.h"
#include "basic_error.h"
#include "business_package_hdr.h"
//#include "global_vas_conn.h"


//替换字符串中的指定字符
std::string& replace_get_string(std::string& target, const std::string& find, const std::string& replace) {
	std::string::size_type pos = std::string::npos;
	while ((pos = target.find(find)) != std::string::npos) {
		target.replace(pos, find.length(), replace);
	}
	return target;
}

using namespace LHttp;

static LHttp::HttpSessionServer hss;

void register_http(const char* url_suffix, http_callback_t http_callback)
{
	hss.AddHandler(url_suffix, [http_callback](const HttpHeader& hh, const string&body, fieldmap_t& rep_hdr, string& rep_body)->int {
		int ret = 200;
		bool isDis = false;
		try {
			//INFO("http request path is {}",hh.path.data());
			//LOG_DEBUG(MAIN_LOG_BC_PT, " http request path is {}", hh.path.data());
			if (hh.method == (enum http_method) 1)//如果是get请求
			{
				isDis = true;
				string str = "{\"";
				string str1 = "\"}";
				str += hh.query;
				str += str1;
				replace_get_string(str, "=", "\":\"");
				replace_get_string(str, "&", "\",\"");
				bp_http_hdl_t http_hdl;
				http_callback(str.data(), str.size(), &http_hdl);//获取接口响应返回
				rep_body = http_hdl.resp;
				if ((body.empty()) || (body[body.size() - 1] == '}'))
				{
					rep_body = http_hdl.resp;
				}
				else
				{
					auto pos = body.find_last_of('}');
					string reqId;
					if (pos == string::npos)
					{
						rep_body = http_hdl.resp;
					}
					else
					{
						reqId = body.substr(pos + 1);
						rep_body = http_hdl.resp;
						rep_body += reqId;
					}
				}
			}
			else //post请求
			{

				bp_http_hdl_t http_hdl;
				if ((body.empty()) || (body[body.size() - 1] == '}'))
				{
					ret = 200;
					isDis = false;
					http_callback(body.data(), body.size(), &http_hdl);//获取接口响应返回
					rep_body = http_hdl.resp;
				}
				else
				{
					isDis = true;
					ret = 201;
					auto pos = body.find_last_of('}');
					string reqId;
					if (pos == string::npos)
					{
						reqId = body;
						ret = 200;
						isDis = false;
						         http_callback("", 0, &http_hdl);//获取接口响应返回
						rep_body = http_hdl.resp;
					}
					else
					{
						reqId = body.substr(pos + 1);
						const_cast<string &>(body).erase(pos + 1);
						http_callback(body.data(), body.size(), &http_hdl);//获取接口响应返回
						rep_body = http_hdl.resp;
					}
					rep_body += reqId;//在回复的内容中，加入多节点发来的reqid
				}
			}
		}
		catch (GeneralException2& e) {
			Document dc = Rjson::rWriteDC();
			Rjson::rAdd(dc, "errCode", e.err_code());
			Rjson::rAdd(dc, "retFlag", StringRef(e.err_msg()));
			rep_body = Rjson::ToString(dc);
			ret = isDis ? 501 : 500;
		}
		catch (...) {
			Document dc = Rjson::rWriteDC();
			Rjson::rAdd(dc, "errCode", -100);
			Rjson::rAdd(dc, "retFlag", StringRef("unhandled exception"));
			rep_body = Rjson::ToString(dc);
			ret = isDis ? 501 : 500;
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
	hss.InitServer(port, 4);//
}

void http_service_close()
{
	hss.Close();
}


