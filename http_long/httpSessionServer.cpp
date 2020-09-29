#include "spdlog_helper.h"
#include "Util.h"
#include "httpSessionServer.h"
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "tbb/concurrent_queue.h"
//#include <tbb/concurrent_vector.h>
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




//static auto hlog = spdlog::basic_logger_mt("http", "lhttp.txt", false);


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

void HttpSessionServer::InitServer(int listen_port,int threadNum)
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
	//do_handle_http(s, threadNum);
	g_recvPumpQueue.set_capacity(120);
	m_server_thr = ::std::thread([&]() {
		for (; m_server_sock > 0;) {
			SOCKET s = sk_accept1(m_server_sock);
			if (s == INVALID_SOCKET) continue;
			do_handle_http(s);
		}
	});
	//m_server_thr.detach();
	//////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < threadNum; ++i)
	{
		::std::thread([&/*, sock*/]()
		{
			m_pendingTasks.fetch_add(1);
			for (std::shared_ptr<RecvMsgData> rmd;;)
			{
				g_recvPumpQueue.pop(rmd);
				try
				{
					auto iter = m_handlers.find(rmd->rMsgData.httpMsgData->path);
					//auto iter = m_handlers.find(sh->hh.path);
					if (iter == m_handlers.end())
					{
						//auto hrw = initRespWrite(sh);
						auto hrw = rmd->hsh;
						string resp = "{\"errCode\":0, \"retFlag\":\"url not found\"}";
						fieldmap_t rephdrflds;
						rephdrflds["Content-length"] = std::to_string(resp.size());
						hrw->header_write(200, rephdrflds);
						hrw->data_write(resp);
						hrw->data_end();
						system_sleep(3000);
						//isLongSoc = false;
						//break;
						continue;
					}
					else
					{
						auto& hdl = iter->second;
						//auto hrw = initRespWrite(sh);
						auto hrw = rmd->hsh;
						int rc = 500;
						string repdata;
						fieldmap_t rephdrflds;
						try
						{
							if (hdl.lrh)
							{
								//hdl.lrh(sh->hh, sh->vbody, hrw);
								hdl.lrh(*(rmd->rMsgData.httpMsgData), rmd->rMsgData.vbody, hrw);
							}
							else
							{
								rc = hdl.srh(*(rmd->rMsgData.httpMsgData), rmd->rMsgData.vbody, rephdrflds, repdata);
								//printf("here!!!!send message content is %s\n", repdata.data());
								//hlog->info("here!!!!!send message content is {}",sh->vbody);
								//hlog->info("here!!!!send message content is {}", repdata.data());
								LOG_ERROR(TRIFLE_LOG_BC_PT, "send message content is {}", repdata.data());
							}
						}
						catch (GeneralException2& e) {
							if (e.err_code())
								//hlog->error("hdl exception:{} {} {}", rmd->rMsgData.httpMsgData->path, e.err_code(), e.err_str());
							LOG_ERROR(TRIFLE_LOG_BC_PT,"hdl exception:{} {} {}", rmd->rMsgData.httpMsgData->path, e.err_code(), e.err_str());
							string resp = fmt::format("{}\"errCode\":-1, \"retFlag\":\"{} throw exception,{}\"{}", "{", rmd->rMsgData.httpMsgData->path, e.err_str(), "}");
							fieldmap_t rephdrflds;
							rephdrflds["Content-length"] = std::to_string(resp.size());
							hrw->header_write(500, rephdrflds);
							hrw->data_write(resp);
							hrw->data_end();
							system_sleep(3000);
							continue;
						}
						catch (...) {
							//hlog->error("hdl exception:{}", rmd->rMsgData.httpMsgData->path);
							LOG_ERROR(TRIFLE_LOG_BC_PT, "hdl exception:{}", rmd->rMsgData.httpMsgData->path);
							string resp = fmt::format("{}\"errCode\":-1, \"retFlag\":\"{}, throw exception \"{}", "{", rmd->rMsgData.httpMsgData->path, "}");
							fieldmap_t rephdrflds;
							rephdrflds["Content-length"] = std::to_string(resp.size());
							hrw->header_write(500, rephdrflds);
							hrw->data_write(resp);
							hrw->data_end();
							system_sleep(3000);
							continue;
						}
						if (hdl.srh)
						{
							if (rc == 201 || rc == 501)
							{
								rephdrflds["Content-length"] = std::to_string(repdata.size());
								string hd;
								if (rc == 201)
								{
									rc = 200;
								}
								else
								{
									rc = 500;
								}
								hrw->header_write(rc, rephdrflds, hd);
								hrw->data_write(hd + repdata);
							}
							else
							{
								//printf("%s\n", sh->hh.path.c_str());
								rephdrflds["Content-length"] = std::to_string(repdata.size());
								hrw->header_write(rc, rephdrflds);
								hrw->data_write(repdata);
								hrw->data_end();
								//system_sleep(3000);
								//isLongSoc = false;
								//break;
								continue;
							}
						}

					}
				}
				catch (GeneralException2& e) {
					if (e.err_code())
						//hlog->error("Http parse exception: {} {}", e.err_code(), e.err_str());
						LOG_ERROR(TRIFLE_LOG_BC_PT,"Http parse exception: {} {}", e.err_code(), e.err_str());
				}
				catch (std::exception& e) {
					string a = e.what();
				}
				catch (...) {
					//hlog->error("Http parse user unhandled error");
					LOG_ERROR(TRIFLE_LOG_BC_PT,"Http parse user unhandled error");
				}
			}
			m_pendingTasks.fetch_sub(1);
		}).detach();
	}
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
			if (us[i + 1] == 0 || us[i + 2] == 0) 
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
		int _state;
		string vbuffer, tmp_kn;
		int _offset;
	public:
		//parsed-request-data
		HttpHeader hh;
		string vbody;
	protected:


		static int _on_hdr_complete(http_parser* hp) {
			SessionHolder* psh = (SessionHolder*)hp->data;
			psh->_state = SHST_PARSE_REQ_BODY;
			return 2;
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
			vbuffer.reserve(1024*1024);	_state = 0; _offset = 0;
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

		void bufferclear()
		{
			vbuffer.clear();
			tmp_kn.clear();
			http_parser_init(&hp, HTTP_REQUEST);
			hp.data = this;
			_state = 0;
			_offset = 0;
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
		int pump() 
		{
			char buf[1024*1024];
			int rsz = 0;

			if (_offset > 0 && vbuffer.size() != _offset)
			{
				//LOG_INFO(TRIFLE_LOG_BC_PT,"step1-offset:{}-------buffer sz:{}-------vbuffer content:{}",_offset,vbuffer.size(),vbuffer);
				while (true)
				{
					size_t p_end = vbuffer.find("\r\n\r\n", _offset);
					if (p_end != string::npos)
					{
						break;
					}

					rsz = ::recv(s, buf, sizeof(buf), MSG_NOSIGNAL);
					if (rsz == 0)
						throw GeneralException2(_state == 0 ? 0 : 1, "unexpected http socket read end!");
					if (rsz < 0) {
						throw GeneralException2(-2, system_errmsg());
					}

					vbuffer.append(buf, rsz);
				}
			}
			else
			{
				//LOG_INFO(TRIFLE_LOG_BC_PT,"step2---offset:{}-------buffer sz:{}-------vbuffer content:{}",_offset,vbuffer.size(),vbuffer);
				rsz = ::recv(s, buf, sizeof(buf), MSG_NOSIGNAL);
				if (rsz == 0)
					throw GeneralException2(_state == 0 ? 0 : 1, "unexpected http socket read end!");
				if (rsz < 0) {
					throw GeneralException2(-2, system_errmsg());
				}

				vbuffer.append(buf, rsz);
			}
			
			if (_state == 0)  //
			{
				//LOG_INFO(TRIFLE_LOG_BC_PT,"step3---offset:{}-------buffer sz:{}-------vbuffer content:{}",_offset,vbuffer.size(),vbuffer);
				size_t p_end = vbuffer.find("\r\n\r\n", _offset);
				if (p_end != string::npos) {
					_state = SHST_PARSE_URL;
					_offset += http_parser_execute(&hp, &settings, vbuffer.data() + _offset, vbuffer.size() - _offset);

					if (hp.http_errno) {
						throw GeneralException2(-1).format_errmsg("http_parse error! errno=%d,%s", hp.http_errno,
							http_errno_name(HTTP_PARSER_ERRNO(&hp)));
					}
				}
			}
			else  //
			{
				//LOG_INFO(TRIFLE_LOG_BC_PT,"step4---offset:{}-------buffer sz:{}-------vbuffer content:{}",_offset,vbuffer.size(),vbuffer);
				_offset += http_parser_execute(&hp, &settings, vbuffer.data() + _offset, vbuffer.size() - _offset);
				if (hp.http_errno) {
					throw GeneralException2(-1).format_errmsg("http_parse error! errno=%d,%s", hp.http_errno,
						http_errno_name(HTTP_PARSER_ERRNO(&hp)));
				}
			}

			if (_state == SHST_PARSE_REQ_BODY_COMP)
			{
				//LOG_INFO(TRIFLE_LOG_BC_PT,"step5---offset:{}-------buffer sz:{}-------vbuffer content:{}",_offset,vbuffer.size(),vbuffer);
				if (_offset == vbuffer.size())
				{
					bufferclear();
				}
				else
				{
					_state = 0;
				}

				return 0;
			}
			else
			{
				return 1;
			}
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
	
	::std::thread([&, sock]()
	{
		m_pendingTasks.fetch_add(1);
		::std::shared_ptr<SessionHolder> sh = ::std::make_shared<SessionHolder>(sock);
		try
		{
			bool isLongSoc = true;
			while (isLongSoc)
			{
				sh->vbody.clear();
				//sh->bufferclear();
				while (sh->pump()) {}
				//hlog->info("here!!!!message content is {}", sh->vbody);
				LOG_INFO(TRIFLE_LOG_BC_PT,"receive!!!message content is {}", sh->vbody);
				std::shared_ptr<RecvMsgData> rmd = std::make_shared<RecvMsgData>();

				HsgData hd;
				hd.httpMsgData = std::make_shared<HttpHeader>(sh->hh);
				//hd.vbody = sh->vbody;
				hd.vbody.swap(sh->vbody);
				rmd->rMsgData.swap(hd);
				rmd->hsh = initRespWrite(sh);
				g_recvPumpQueue.push(rmd);

			}
		}
		catch (GeneralException2& e) {
			if (e.err_code())
				//hlog->error("Http parse exception: {} {}", e.err_code(), e.err_str());
				LOG_ERROR(TRIFLE_LOG_BC_PT,"Http parse exception: {} {}", e.err_code(), e.err_str());
		}
		catch (std::exception& e) {
			string a = e.what();
		}
		catch (...) {
			//hlog->error("Http parse user unhandled error");
			LOG_INFO(TRIFLE_LOG_BC_PT,"Http parse user unhandled error");
		}

		//sh->sock_close();
		m_pendingTasks.fetch_sub(1);
	}).detach();
}


}/////END of namespace
