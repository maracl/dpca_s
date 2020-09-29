﻿#ifndef _HTTP_SESSION_SERVER_H
#define _HTTP_SESSION_SERVER_H
#ifdef _MSC_VER
#pragma once
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <functional>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <memory>
#include "socket_common.h"
#include "General_exception2.h"
#include "common_interface.h"

namespace LHttp{
#include "http_parser.h"

using std::string;
using fieldmap_t = ::std::unordered_map<string, string>;

struct HttpHeader{
	http_method method;
	//url缁嗚妭
	string path, query;
	fieldmap_t header_fields;
};

struct FileInfo {
	string fname;
	string fdata;
};

using filemap_t = std::unordered_map<string, FileInfo>;

//response鏁版嵁锛堝紓姝ワ級鍐欏叆宸ュ叿锛屽彲鐢ㄤ簬闀胯繛鎺?
//娉ㄦ剰锛氭墍鏈夋柟娉曞潎鍙兘鎶涘嚭寮傚父锛圙eneralException2锛?
struct HttpRespWriter {
	virtual ~HttpRespWriter() {}
	virtual void header_write(int httpCode) {
		fieldmap_t defmap;
		header_write(httpCode, defmap);
	}
	virtual void header_write(int httpCode, const fieldmap_t& respHeaderFields) = 0;
	virtual void header_write(int httpCode, const fieldmap_t& respHeaderFields, string& hd) = 0;
	virtual void data_write(const char* data, size_t sz) = 0;
	virtual void data_write(const string& ds) {
		data_write(ds.data(), ds.size());
	}
	virtual void data_end() = 0;
};

using HRWType = ::std::shared_ptr<HttpRespWriter>;

class HttpSessionServer :no_copy {
public:
	//鐭繛鎺ュ鐞?
	using SimRepFuncType = ::std::function<int(const HttpHeader& hh, const string&body, fieldmap_t& rep_hdr, string& rep_body)>;

	//闀胯繛鎺ュ鐞嗭紙鍙紓姝ワ級
	using LongRepFuncType = ::std::function<void(const HttpHeader& hh, const string&body, const HRWType& hrw)>;

	HttpSessionServer() { m_server_sock = 0; m_pendingTasks = 0; on_url_not_found = nullptr; }
	~HttpSessionServer();

	//throw!
	void InitServer(int listen_port, int threadNum);

	void Close();

	bool AddHandlerL(const string&url, const LongRepFuncType& f);

	bool AddHandler(const string&url, const SimRepFuncType& f);

	LongRepFuncType on_url_not_found;

	//std::vector < std::thread > buss_thrd;

	static void parse_url_fields(string query, fieldmap_t& fs);
	static int parse_mpfd_fields(const string& body, fieldmap_t& fs1, filemap_t& fs2);
protected:
	static int parse_MPFD(const string& data, fieldmap_t& fs);
	void do_handle_http(SOCKET hsd);
private:
	struct HandlerGrp{
		LongRepFuncType lrh;
		SimRepFuncType srh;
		HandlerGrp() { lrh = {}; srh = {}; }
		explicit HandlerGrp(const LongRepFuncType&l) :lrh(l), srh(nullptr) {}
		explicit HandlerGrp(const SimRepFuncType& s) :lrh(nullptr), srh(s) {}
		HandlerGrp(HandlerGrp&& h) :lrh(::std::move(h.lrh)), srh(::std::move(h.srh)) {}
	};
	::std::atomic<SOCKET> m_server_sock;
	::std::thread m_server_thr;
	::std::unordered_map<string, HandlerGrp> m_handlers;
	::std::atomic<int> m_pendingTasks;
};


}


#endif
