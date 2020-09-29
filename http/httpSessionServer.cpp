#include "spdlog_helper.h"
#include "Util.h"
#include "httpSessionServer.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "tbb/concurrent_queue.h"
//#include "tbb/concurrent_vector.h"
#include "General_exception2.h"
#include <iostream>
#include "system.h"

#ifdef __linux
#include <unistd.h>
#define strtok_s strtok_r
static int Sleep(uint32_t mill) {
	return usleep(mill*1000L);
}
#endif

//repeat once
namespace LHttp{
//using namespace std;
	struct HsgData
	{
		std::shared_ptr<HttpHeader> httpMsgData;
		string vbody;
		void swap(HsgData& r) {
			std::swap(httpMsgData, r.httpMsgData);
			std::swap(vbody, r.vbody);
		}
	};

	struct RecvMsgData
	{
		HsgData rMsgData;
		HRWType hsh;
		void swap(RecvMsgData& rt)
		{
			std::swap(rMsgData, rt.rMsgData);
		}
	};
	static tbb::concurrent_bounded_queue<std::shared_ptr<RecvMsgData>> g_recvPumpQueue;

HttpSessionServer::~HttpSessionServer()
{
	if (m_server_thr.joinable()) m_server_thr.join();
	while (m_pendingTasks > 0) {
		Sleep(8);
	}
}

void HttpSessionServer::Close()
{
	SOCKET ss = m_server_sock.exchange(0);
	if (ss) {
		sk_close(ss);
	}
}

void HttpSessionServer::InitServer(int listen_port)
{
	sk_sys_init();

	if (m_server_sock > 0)
		throw GeneralException2(1, "server socket already initialized!");
	sockaddr_ex addr;
	if (sk_tcp_addr(addr, "0.0.0.0", listen_port))
		throw GeneralException2(-1, system_errmsg());
	m_server_sock = sk_create(addr.sa_family, SOCK_STREAM, 0);
	assert(m_server_sock > 0);
	int nOptval = 1;
	setsockopt(m_server_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&nOptval, sizeof(int));

	struct linger linger;
	linger.l_onoff = 1;
	linger.l_linger = 5;
	setsockopt(m_server_sock, SOL_SOCKET, SO_LINGER, (char *)&linger, sizeof(linger));
	if (addr.bindto(m_server_sock)) {
		throw GeneralException2(-1, system_errmsg());
	}
	if (::listen(m_server_sock, SOMAXCONN)) {
		throw GeneralException2(-1, system_errmsg());
	}
	m_server_thr = ::std::thread([&](){
		for (; m_server_sock > 0;) {
			SOCKET s = sk_accept1(m_server_sock);
			if (s == INVALID_SOCKET) break;
			do_handle_http(s);
		}
	});
}

bool HttpSessionServer::AddHandler(const string&url, const SimRepFuncType& f)
{
	return m_handlers.insert(make_pair(url, HandlerGrp(f))).second;
}

bool HttpSessionServer::AddHandlerL(const string&url, const LongRepFuncType& f)
{
	return m_handlers.insert(make_pair(url, HandlerGrp(f))).second;
}

static inline int _hexc(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;
}
static string url_decode(const char* us) {
	string rs;
	for (int i = 0;;) {
		char c = us[i];
		if (c == 0) break;
		else if (c == '%') {
			if (us[i + 1] == 0 || us[i + 2] == 0) //部分分析（错误）
				break;
			rs += ((char)(16 * _hexc(us[i + 1]) + _hexc(us[i + 2])));
			i += 3;
		}
		else if (c == '+') {
			rs += ' ';
			i++;
		}
		else {
			rs += c;
			i++;
		}
	}
	return rs;
}

void HttpSessionServer::parse_url_fields(string query, fieldmap_t& fs) {
	if (parse_MPFD(query, fs)>=0) return;
	char* sp = nullptr;
	char* tok = strtok_s((char*)query.data(), "&", &sp);
	for (; tok; tok = strtok_s(nullptr, "&", &sp)){
		char* pe = strchr(tok, '=');
		if (!pe) continue;
		*pe = 0; ++pe;
		fs[url_decode(tok)] = url_decode(pe);
	}
}

int HttpSessionServer::parse_MPFD(const string& data, fieldmap_t& fs) {
	if (!(data.size() > 2 && data[0] == '-'&& data[1] == '-')) return -1;
	size_t p1 = data.find("\r\n");
	if (p1 == string::npos) return -2;
	static const string fn_st("form-data; name=\""), fn_ed("\"");
	const string mpf_bdary(data.data(), p1);
	for (size_t pst = p1 + 2;;) {
		size_t pby = data.find(mpf_bdary, pst);
		if (pby == string::npos) pby = data.size();
		if (pby < 8 + pst) break;
		size_t pfn1 = data.find(fn_st, pst);
		if (pfn1 > pby) return 1;
		pfn1 += fn_st.size();
		size_t pfn2 = data.find(fn_ed, pfn1);
		if (pfn2 > pby) return 2;
		string keyname(data.data() + pfn1, pfn2 - pfn1);
		size_t pd1 = data.find("\r\n\r\n", pfn2);
		if (pd1 > pby - 4) return 3;
		string valdata(data.data() + pd1 + 4, pby - pd1 - 4 - 2);
		fs[keyname] = valdata;
		pst = pby + mpf_bdary.size();
	}
	return 0;
}

int HttpSessionServer::parse_mpfd_fields(const string& data, fieldmap_t& fs1, filemap_t& fs2)
{
	if (!(data.size() > 2 && data[0] == '-'&& data[1] == '-')) return -1;
	size_t p1 = data.find("\r\n");
	if (p1 == string::npos) return -2;
	static const string fn_st("form-data; name=\""), fn_ed("\""), file_st("filename=\"");
	const string mpf_bdary(data.data(), p1);
	for (size_t pst = p1 + 2;;) {
		size_t pby = data.find(mpf_bdary, pst);
		if (pby == string::npos) pby = data.size();
		if (pby < 8 + pst) break;
		size_t pfn1 = data.find(fn_st, pst);
		if (pfn1 > pby) return 1;
		pfn1 += fn_st.size();
		size_t pfn2 = data.find(fn_ed, pfn1);
		if (pfn2 > pby) return 2;
		string keyname(data.data() + pfn1, pfn2 - pfn1);
		//find 'filename' if exist.
		string filename_;
		size_t pfn3 = data.find(file_st, pfn2);
		if (pfn3 != string::npos) {
			pfn3 += file_st.size();
			size_t pfn4 = data.find(fn_ed, pfn3);
			if (pfn4 > pby) return 5;
			filename_.assign(data.data() + pfn3, pfn4 - pfn3);
			pfn2 = pfn4;
		}
		size_t pd1 = data.find("\r\n\r\n", pfn2);
		if (pd1 > pby - 4) return 3;
		string valdata(data.data() + pd1 + 4, pby - pd1 - 4 - 2);
		if (filename_.empty()) {
			fs1[keyname] = valdata;
		}
		else {
			fs2.insert(std::make_pair(keyname, FileInfo{ filename_, valdata }));
		}
		
		pst = pby + mpf_bdary.size();
	}
	return 0;
}

namespace {
	enum {
		SHST_PARSE_URL = 1,
		SHST_PARSE_HEADER_FIELDS_KEY = 2,
		SHST_PARSE_HEADER_FIELDS_VALUE = 3,
		SHST_PARSE_REQ_BODY = 4,
		SHST_PARSE_REQ_BODY_COMP = 5,
	};

	class SessionHolder :no_copy{

		http_parser hp;
		http_parser_settings settings;
		::std::atomic<SOCKET> s;
		//中间量
		int _state;
		string vbuffer, tmp_kn;
	public:
		//parsed-request-data
		HttpHeader hh;
		string vbody;
	protected:


		static int _on_hdr_complete(http_parser* hp) {
			SessionHolder* psh = (SessionHolder*)hp->data;
			psh->_state = SHST_PARSE_REQ_BODY;
			return 0;
		}

		static int _on_msg_complete(http_parser* hp) {
			SessionHolder* psh = (SessionHolder*)hp->data;
			psh->_state = SHST_PARSE_REQ_BODY_COMP;
			return 0;
		}

		static int _on_xxx_data(http_parser* hp, const char *at, size_t length) {
			SessionHolder* psh = (SessionHolder*)hp->data;
			auto& hh = psh->hh;
			switch (psh->_state)
			{
			case SHST_PARSE_URL:{
				hh.method = (enum http_method)hp->method;
				//分析url细节
				http_parser_url u = {};
				if (http_parser_parse_url(at, length, 0, &u) == 0) {

					if (u.field_set & (1 << UF_PATH)) {
						hh.path.assign(at + u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
					}
					if (u.field_set & (1 << UF_QUERY)) {
						hh.query.assign(at + u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
					}
				}
				psh->_state = SHST_PARSE_HEADER_FIELDS_KEY;
			}break;
			case SHST_PARSE_HEADER_FIELDS_KEY:
				psh->tmp_kn.assign(at, length);
				psh->_state = SHST_PARSE_HEADER_FIELDS_VALUE;
				break;
			case SHST_PARSE_HEADER_FIELDS_VALUE:
				if (psh->tmp_kn.empty() == false) {
					hh.header_fields[psh->tmp_kn].assign(at, length);
				}
				psh->_state = SHST_PARSE_HEADER_FIELDS_KEY;
				break;
			case SHST_PARSE_REQ_BODY:
				psh->vbody.append(at, length);
				break;
			default:
				break;
			}
			return 0;
		}
	public:
		explicit SessionHolder(SOCKET sock) :s(sock){
			vbuffer.reserve(1024);	_state = 0;
			http_parser_init(&hp, HTTP_REQUEST);
			hp.data = this;

			http_parser_settings_init(&settings);
			settings.on_headers_complete = _on_hdr_complete;
			settings.on_message_complete = _on_msg_complete;
			settings.on_header_field = _on_xxx_data;
			settings.on_header_value = _on_xxx_data;
			settings.on_body = _on_xxx_data;
			settings.on_url = _on_xxx_data;
		}

		/* throwable!!
			分析完成：0
			分析中：1
			*/
		int pump() {
			char buf[1024];
			int rsz = ::recv(s, buf, sizeof(buf), MSG_NOSIGNAL);
			if (rsz == 0)
				throw GeneralException2(_state == 0 ? 0 : 1, "unexpected http socket read end!");
			if (rsz < 0) {
				throw GeneralException2(-2, system_errmsg());
			}
			if (_state == 0) {//接收整个header。直到header收完整再进行parse
				vbuffer.append(buf, rsz);
				size_t p_end = vbuffer.find("\r\n\r\n");
				if (p_end != string::npos) {
					//分析header其余部分，以及可能的部分body
					_state = SHST_PARSE_URL;
					http_parser_execute(&hp, &settings, vbuffer.data(), vbuffer.size());
					if (hp.http_errno) {
						throw GeneralException2(-1).format_errmsg("http_parse error! errno=%d,%s", hp.http_errno,
							http_errno_name(HTTP_PARSER_ERRNO(&hp)));
					}
				}
			}
			else {//header接收完毕，接收body
				http_parser_execute(&hp, &settings, buf, rsz);
				if (hp.http_errno) {
					throw GeneralException2(-1).format_errmsg("http_parse error! errno=%d,%s", hp.http_errno,
						http_errno_name(HTTP_PARSER_ERRNO(&hp)));
				}
			}
			return _state == SHST_PARSE_REQ_BODY_COMP ? 0 : 1;
		}

		void sock_close() {
			SOCKET cs = s.exchange(0);
			if (cs) sk_close(cs);
		}

		~SessionHolder() {
			sock_close();
		}

		//throwable!!
		void sock_write(const char* data, size_t sz) {
			SOCKET ss = s;
			if (ss == 0) throw GeneralException2(1, "http 'sock_write' failed. socket was closed!");
			int rt = send_all(ss, data, (int)sz);
			if (rt < 0) throw GeneralException2(2, string("http 'sock_write' failed. ")+system_errmsg());
		}
	};


	HRWType initRespWrite(::std::shared_ptr<SessionHolder>& sh)
	{
		struct HRWImpl:public HttpRespWriter {
			::std::shared_ptr<SessionHolder> sh;
			explicit HRWImpl(::std::shared_ptr<SessionHolder>& s) :sh(s) {}
			virtual ~HRWImpl() {}
			virtual void header_write(int httpCode, const fieldmap_t& fm) {
				string hd = fmt::format("HTTP/1.1 {} {}\r\n", httpCode, http_status_str((enum http_status)httpCode));
				size_t reserved_sz = hd.size();
				for (auto& ik : fm) {
					reserved_sz += ik.first.size() + ik.second.size() + 8;
				}
				hd.reserve(reserved_sz + 32);
				for (auto& ik : fm) {
					hd += ik.first + ": " + ik.second + "\r\n";
				}
				if (fm.find("Connection") == fm.end()) {
					hd += "Connection: Close\r\n";
				}
				hd += "\r\n";
				sh->sock_write(hd.data(), hd.size());
			}
			virtual void header_write(int httpCode, const fieldmap_t& fm, string& hd) {
				hd = fmt::format("HTTP/1.1 {} {}\r\n", httpCode, http_status_str((enum http_status)httpCode));
				size_t reserved_sz = hd.size();
				for (auto& ik : fm) {
					reserved_sz += ik.first.size() + ik.second.size() + 8;
				}
				hd.reserve(reserved_sz + 32);
				for (auto& ik : fm) {
					hd += ik.first + ": " + ik.second + "\r\n";
				}
				if (fm.find("Connection") == fm.end()) {
					hd += "Connection: Close\r\n";
				}
				hd += "\r\n";
			}
			virtual void data_write(const char* data, size_t sz) {
				sh->sock_write(data, sz);
			}
			virtual void data_end() {
				sh->sock_close();
			}
		};
		return ::std::make_shared<HRWImpl>(sh);
	}
}////////////////////////////////

void HttpSessionServer::do_handle_http(SOCKET sock)
{
	m_pendingTasks.fetch_add(1);
	::std::thread([&, sock](){
		//sk_set_tcpnodelay(sock, true);
		::std::shared_ptr<SessionHolder> sh = ::std::make_shared<SessionHolder>(sock);
		try{
			while (sh->pump()) {}
			LOG_INFO(TRIFLE_LOG_BC_PT,"receive!!!message content is {}", sh->vbody);
			auto iter = m_handlers.find(sh->hh.path);
			if (iter == m_handlers.end()) {
				//hlog->info("URL=[{}] cannot find url handler!", sh->hh.path);
				LOG_INFO(TRIFLE_LOG_BC_PT,"URL=[{}] cannot find url handler!", sh->hh.path);
				if (on_url_not_found) {
					auto hrw = initRespWrite(sh);
					on_url_not_found(sh->hh, sh->vbody, hrw);
				}
				else {
					static const string resp404("HTTP/1.1 404 Not found\r\nConnection: Close\r\n\r\n");
					//static const string resp404("{\"code\":-1,\"message\":\"unknown url\"}");
					sh->sock_write(resp404.data(), resp404.size());
				}
			}
			else
			{
				auto& hdl = iter->second;
				auto hrw = initRespWrite(sh);
				if (hdl.lrh) {
					hdl.lrh(sh->hh, sh->vbody, hrw);
				}
				else {
					string repdata;
					fieldmap_t rephdrflds;
					int rc = hdl.srh(sh->hh, sh->vbody, rephdrflds, repdata);
					LOG_ERROR(TRIFLE_LOG_BC_PT, "send message content is {}", repdata.data());
					rephdrflds["Content-length"] = std::to_string(repdata.size());
					hrw->header_write(rc, rephdrflds);
					hrw->data_write(repdata);
				}
			}

		}
		catch (GeneralException2& e) {
			if (e.err_code())
				//hlog->error("Http parse exception: {} {}", e.err_code(), e.err_str());
				LOG_ERROR(TRIFLE_LOG_BC_PT,"Http parse exception: {} {}", e.err_code(), e.err_str());
		}
		catch (...) {
			//hlog->error("Http parse user unhandled error");
			LOG_INFO(TRIFLE_LOG_BC_PT,"Http parse user unhandled error");
		}

		m_pendingTasks.fetch_sub(1);
	}).detach();
}


}/////END of namespace
