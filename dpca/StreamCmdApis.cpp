#include <assert.h>
#include "StreamCmdApis.h"
#include "socket_common.h"
#include "byte_buffer.h"
#include "General_exception2.h"
#include "basic_error.h"

namespace {

static void wrap_bb(byte_buffer& outbuf)
{
	size_t lp = outbuf.lpos();
	assert(lp >= 4);
	int oblen = (int)outbuf.data_size();
	outbuf.lset(lp - 4);
	*(int*)outbuf.data_ptr() = oblen;
}

inline void _bb_push(byte_buffer& bb) { }

template<class T, class ...Types>
inline void _bb_push(byte_buffer& bb, const T& arg1, Types ... args) {
	bb << arg1;
	_bb_push(bb, args...);
}


template<class ...Types>
CharSeqReader _rt_cmd_comm(const sockaddr_ex& addr, uint32_t conn_tmo, byte_buffer& _bb, int cmd, Types ... args) {
	sockwrapper sw(sk_create(AF_INET, SOCK_STREAM, 0));
	if (int rt = sw.connect_ex(addr, conn_tmo)) {
		throw GeneralException2(sockerr_get()).format_errmsg("socket connect error=%d", rt);
	}

	sk_set_tcpnodelay(sw, true);

	struct linger linger;
	linger.l_onoff = 1;
	linger.l_linger = 0;
	int rt = setsockopt((SOCKET)sw, SOL_SOCKET, SO_LINGER, (char *)&linger, sizeof(linger));

	_bb.bset(4);	_bb << cmd;
	_bb_push(_bb, args...);
	wrap_bb(_bb);
	if (send_all(sw, _bb.data_ptr(), (int)_bb.data_size()) <= 0) {
		throw GeneralException2(sockerr_get()).format_errmsg("socket send error! %s", system_errmsg().data());
	}
	uint32_t resp_sz = 0;
	if (recv_s_obj(sw, resp_sz) <= 0) 
	{
		throw GeneralException2(sockerr_get()).format_errmsg("socket recv error! %s", system_errmsg().data());
	}
	if (resp_sz > 10000000000) {
		throw GeneralException2(ERROR_ISE_API_RECV_SIZE, "socket response data size error!");
	}
	_bb.lset(0);	
	_bb.pre_alloc(resp_sz + 8);
	_bb.rset(resp_sz);
	if (recv_all(sw, _bb.data_ptr(), resp_sz) <= 0) {
		throw GeneralException2(sockerr_get()).format_errmsg("socket recv error! %s", system_errmsg().data());
	}
	CharSeqReader chrd(_bb.data_ptr(), _bb.data_size());
	int retc = 0;	chrd >> retc;
	if (retc != 0) {
		std::string emsg;  chrd>>emsg;
		throw GeneralException2(retc,emsg);
	}
	return chrd;
}

#define ISE_API_START	byte_buffer bb(256);
#define ISE_API_END		; 

#define RT_CMD_COMM(cmdid,...)	_rt_cmd_comm(*(this->addr_),this->conn_tmo,bb,cmdid,__VA_ARGS__)

}//............

IseApi::IseApi() :conn_tmo(4000)
{
	addr_ = new sockaddr_ex();

	sk_sys_init();
}

IseApi::~IseApi()
{
	delete addr_;
}

int IseApi::SetNetAddr(const char* ip, int port)
{
	if (sk_tcp_addr(*addr_, ip, port) != 0) {
		string errmsg = system_errmsg().data();
		throw GeneralException2(ERROR_ISE_API_SET_ISE_ADDR).format_errmsg("ExtrApi::setNetAddr failed: %s", system_errmsg().data());
		return -1;
	}

	return 0;
}

void IseApi::create_db(const string& dbname, const string& fieldlist, const string& dictionary)
{
ISE_API_START
	RT_CMD_COMM(1, dbname, fieldlist, dictionary);
ISE_API_END
}

void IseApi::delete_db(const string& dbname)
{
ISE_API_START
	RT_CMD_COMM(2, dbname);
ISE_API_END
}

int IseApi::add_db_cluster(const string& dbname, const string& fieldlist, const string& dictionary, int clusterType)
{
ISE_API_START
CharSeqReader chrd = RT_CMD_COMM(27, dbname, fieldlist, dictionary, clusterType);
	int ret = 0;
	chrd >> ret;
	return ret;
ISE_API_END
}

void IseApi::delete_db_cluster(const string& dbname)
{
	ISE_API_START
		RT_CMD_COMM(56, dbname);
	ISE_API_END
}

vector<string> IseApi::enum_all_db(int type)
{
ISE_API_START
CharSeqReader chrd = RT_CMD_COMM(3, type);
	uint32_t nct = 0;
	chrd >> nct;
	vector<string> vn(nct);
	for (auto& v : vn) {
		chrd >> v;
	}
	return vn;
ISE_API_END
}

int64_t IseApi::get_db_status_flag(const string& dbname)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(36, dbname);
	int64_t flag = 0;
	chrd >> flag;
	return flag;
	ISE_API_END
}

int64_t IseApi::get_db_rec_ct(const string& dbname)
{
ISE_API_START
	auto chrd = RT_CMD_COMM(65, dbname);
	int64_t ct = 0;
	chrd >> ct;
	return ct;
ISE_API_END
}

int64_t IseApi::get_db_cluster_rec_ct(const string& dbname)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(37, dbname);
	int64_t rec_count = 0;
	chrd >> rec_count;
	return rec_count;
	ISE_API_END
}

IdxPac IseApi::push_image(const string& dbname, int ftype, const string& feat, const string& paralist, const string& paradata, bool iscluster)
{
ISE_API_START
	auto chrd = RT_CMD_COMM(10, dbname, ftype, feat, paralist, paradata, iscluster);
	IdxPac ip;
	chrd >> ip.idx;
	chrd >> ip.cluster_idx;
	chrd >> ip.gstat;
	return ip;
ISE_API_END
}

IdxPac IseApi::push_image_idx(const string& dbname, int ftype, const string& feat, int64_t idx,
	const string& paralist, const string& paradata, bool iscluster)
{
ISE_API_START
	auto chrd = RT_CMD_COMM(75, dbname, ftype, feat, idx, paralist, paradata, iscluster);
	IdxPac ip;
	chrd >> ip.idx;
	chrd >> ip.cluster_idx;
	chrd >> ip.gstat;
	return ip;
ISE_API_END
}

int64_t IseApi::push_image_cluster(const string& dbname, int64_t cluster_idx, int ftype, const string& feat, const string& paralist, const string& paradata)
{
ISE_API_START
		auto chrd = RT_CMD_COMM(42, dbname, cluster_idx, ftype, feat, paralist, paradata); 
	int64_t idx;
	chrd >> idx;
	return idx;
ISE_API_END
}

IdxPac IseApi::add_cluster_rec(const string& dbname, int ftype, const string& feat, const string& paralist, const string& paradata, int newRec)
{
ISE_API_START
	auto chrd = RT_CMD_COMM(43, dbname, ftype, feat, paralist, paradata, newRec);
	IdxPac ip;
	chrd >> ip.idx;
	chrd >> ip.cluster_idx;
	return ip;
ISE_API_END
}

int IseApi::retrieve_image(const string& dbname, int ftype, const string& feat,
	const string& wanted_param, const string& where_stmt, float min_sim, ImgRecord* pRecs, uint32_t max_rec)
{
ISE_API_START
auto chrd = RT_CMD_COMM(11, dbname, ftype, feat, wanted_param, where_stmt, max_rec, min_sim);
	uint32_t n = 0;
	chrd >> max_rec;
	for (uint32_t i = 0; i < max_rec; i++) {
		ImgRecord& r = pRecs[i];
		chrd >> r.idx >> r.sim;
		
		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params) {
			chrd >> v;
		}
	}
	return max_rec;
ISE_API_END
}

int64_t IseApi::select_rec_ct(const string& dbname, const string& where_stmt)
{
ISE_API_START
auto chrd = RT_CMD_COMM(16, dbname, where_stmt);
	int64_t n = 0;
	chrd >> n;
	return n;
ISE_API_END
}

int IseApi::select_img_rec(const string& dbname, const string& wanted_param, const string& where_stmt,
	ImgRecord* pRecs, uint32_t max_rec)
{
ISE_API_START
auto chrd = RT_CMD_COMM(12, dbname, wanted_param, where_stmt, max_rec);
	chrd >> max_rec;
	for (uint32_t i = 0; i < max_rec; i++) {
		ImgRecord& r = pRecs[i];
		chrd >> r.idx;
		float sim;
		chrd >> r.sim;
		uint32_t n = 0;
		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params) {
			chrd >> v;
		}
	}
	return max_rec;
ISE_API_END
}

int IseApi::get_rec_by_idx(const string& dbname, string idxes, const string& wanted_param, ImgRecord* pRecs)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(55, dbname, idxes, wanted_param);
	int rec_count = 0;
	chrd >> rec_count;
	for (uint32_t i = 0; i < rec_count; i++) 
	{
		ImgRecord& r = pRecs[i];
		chrd >> r.idx;
		float sim;
		chrd >> sim;
		uint32_t n = 0;
		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params) {
			chrd >> v;
		}
	}
	return rec_count;
	ISE_API_END
}

int IseApi::get_cluster_rec_by_cluster_idx(const string& dbname, string cluster_idxes, const string& wanted_param, ImgRecord* pRecs)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(57, dbname, cluster_idxes, wanted_param);
	int rec_count = 0;
	chrd >> rec_count;
	for (uint32_t i = 0; i < rec_count; i++)
	{
		ImgRecord& r = pRecs[i];
		chrd >> r.idx;
		float sim;
		chrd >> sim;
		uint32_t n = 0;
		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params) {
			chrd >> v;
		}
	}
	return rec_count;
	ISE_API_END
}

int IseApi::retrieve_image_cluster(const string& dbname, int ftype, const string& feat, const string& wanted_param, const string& where_stmt, float min_sim, ImgRecord* pRecs, uint32_t max_rec)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(29, dbname, ftype, feat, wanted_param, where_stmt, max_rec, min_sim);
	uint32_t n = 0;
	chrd >> max_rec;
	for (uint32_t i = 0; i < max_rec; i++) {
		ImgRecord& r = pRecs[i];
		chrd >> r.idx >> r.sim;

		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params) {
			chrd >> v;
		}
	}
	return max_rec;
	ISE_API_END
}

int64_t IseApi::select_cluster_ct(const string& dbname, const string& where_stmt)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(30, dbname, where_stmt);
	int64_t n = 0;
	chrd >> n;
	return n;
	ISE_API_END
}

int IseApi::select_image_cluster(const string& dbname, const string& wanted_param, const string& where_stmt, ImgRecord* pRecs, uint32_t max_rec)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(31, dbname, wanted_param, where_stmt, max_rec);
	chrd >> max_rec;
	for (uint32_t i = 0; i < max_rec; i++) {
		ImgRecord& r = pRecs[i];
		chrd >> r.idx;
		float sim;
		chrd >> sim;
		uint32_t n = 0;
		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params) {
			chrd >> v;
		}
	}
	return max_rec;
	ISE_API_END
}

int IseApi::get_image_rec_by_cluster_index(const string& dbname, int64_t cluster_idx, const string& wanted_param, vector<ImgRecord>& pRecs)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(28, dbname, cluster_idx, wanted_param);
	int rec_ct;
	chrd >> rec_ct;
	pRecs.resize(rec_ct);
	for (uint32_t i = 0; i < rec_ct; i++) {
		ImgRecord& r = pRecs[i];
		chrd >> r.idx;
		chrd >> r.sim;
		uint32_t n = 0;
		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params) {
			chrd >> v;
		}
	}
	return rec_ct;
	ISE_API_END
}

void IseApi::get_most_sim_rec_by_cluster_idxes(const string& dbname, const string& cluster_idxes, const string& wanted_param, vector<SimImgRecord>& pRecs)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(38, dbname, cluster_idxes, wanted_param);
	int rec_count = 0;
	chrd >> rec_count;
	pRecs.resize(rec_count);
	for (uint32_t i = 0; i < rec_count; i++) {
		SimImgRecord& r = pRecs[i];
		chrd >> r.cluster_idx;
		chrd >> r.idx;
		chrd >> r.sim;
		uint32_t n = 0;
		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params) {
			chrd >> v;
		}
	}
	ISE_API_END
}

int32_t IseApi::get_rec_ct_by_cluster_idx(const string& dbname, int64_t cluster_idx)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(49, dbname, cluster_idx);
	int32_t rec_count = 0;
	chrd >> rec_count;

	return rec_count;
}

void IseApi::delete_img_rec(const string& dbname, int64_t index)
{
	ISE_API_START
		RT_CMD_COMM(14, dbname, index);
	ISE_API_END
}

void IseApi::delete_img_rec_ws(const string& dbname, const string& where_stmt)
{
	ISE_API_START
		RT_CMD_COMM(21, dbname, where_stmt);
	ISE_API_END
}

void IseApi::update_img_rec(const string& dbname, int64_t index, const string& set_stmt)
{
	ISE_API_START
		RT_CMD_COMM(13, dbname, index, set_stmt); 
	ISE_API_END
}

void IseApi::update_img_rec_ws(const string& dbname, const string& where_stmt, const string& set_stmt)
{
	ISE_API_START
		RT_CMD_COMM(22, dbname, where_stmt, set_stmt);
	ISE_API_END
}

void IseApi::delete_cluster_rec(const string& dbname, int64_t cluster_idx, int del_rec)
{
	ISE_API_START
		RT_CMD_COMM(32, dbname, cluster_idx, del_rec);
	ISE_API_END
}

void IseApi::delete_cluster_rec_ws(const string& dbname, const string& where_stmt, int del_rec)
{
	ISE_API_START
		RT_CMD_COMM(33, dbname, where_stmt, del_rec);
	ISE_API_END
}

void IseApi::update_cluster_data(const string& dbname, int64_t cluster_idx, const string& set_stmt)
{
	ISE_API_START
		RT_CMD_COMM(34, dbname, cluster_idx, set_stmt);
	ISE_API_END
}

void IseApi::update_cluster_data_ws(const string& dbname, const string& where_stmt, const string& set_stmt)
{
	ISE_API_START
		RT_CMD_COMM(41, dbname, where_stmt, set_stmt); 
	ISE_API_END
}

void IseApi::update_rec_cluster(const string& dbname, int64_t idx, int64_t cluster_idx)
{
	ISE_API_START
		RT_CMD_COMM(58, dbname, idx, cluster_idx);
	ISE_API_END
}

void IseApi::remove_rec_of_cluster(const string& dbname, int64_t cluster_idx, const string& idxes)
{
	ISE_API_START
		RT_CMD_COMM(40, dbname, cluster_idx, idxes);
	ISE_API_END
}

void IseApi::join_cluster_rec(const string& dbname, int64_t cluster_idx1, int64_t cluster_idx2)
{
	ISE_API_START
		RT_CMD_COMM(35, dbname, cluster_idx1, cluster_idx2);
	ISE_API_END
}

int IseApi::cluster_comp_by_image_rec(const string& dbname, int64_t idx, int64_t cluster_idx, const string& wanted_param, ImgRecord* pRecs, uint32_t max_rec)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(53, dbname, idx, cluster_idx, wanted_param, max_rec);
	int rec_count = 0;
	chrd >> rec_count;
	for (uint32_t i = 0; i < rec_count; i++)
	{
		ImgRecord& r = pRecs[i];
		chrd >> r.idx;
		float sim;
		chrd >> sim;
		uint32_t n = 0;
		chrd >> n;
		r.v_params.resize(n);
		for (auto& v : r.v_params)
		{
			chrd >> v;
		}
	}
	return rec_count;
	ISE_API_END
}

void IseApi::two_cluster_comp(const string& dbname, int64_t idx, int64_t cluster_idx, const string& wanted_param, 
								unordered_map<shared_ptr<ImgRecord>, vector<shared_ptr<ImgRecord>>>& pRecs, uint32_t max_rec)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(54, dbname, idx, cluster_idx, wanted_param, max_rec);
	uint32_t n = 0;
	int recCount = 0;
	int vrecCount = 0;
	chrd >> vrecCount;
	for (size_t i = 0; i < vrecCount; i++)
	{
		shared_ptr<ImgRecord> irec = make_shared<ImgRecord>();
		chrd >> irec->idx >> irec->sim;
		chrd >> n;
		irec->v_params.resize(n);
		for (auto& v : irec->v_params)
		{
			chrd >> v;
		}

		vector<shared_ptr<ImgRecord>> irecVec;
		chrd >> recCount;
		for (uint32_t j = 0; j < recCount; j++)
		{
			shared_ptr<ImgRecord> r = make_shared<ImgRecord>();
			chrd >> r->idx >> r->sim;
			uint32_t paranum = 0;
			chrd >> paranum;
			r->v_params.resize(paranum);
			for (auto& v : r->v_params)
			{
				chrd >> v;
			}

			irecVec.push_back(r);
		}

		pRecs.insert(make_pair(irec, irecVec));
	}

	return;
	ISE_API_END
}

float IseApi::calc_image_feat_sim(const string& dbname, int64_t idx1, int64_t idx2)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(52, dbname, idx1, idx2);
	float sim = 0;
	chrd >> sim;
	return sim;
	ISE_API_END
}

float IseApi::calc_center_feat_sim(const string& dbname, int64_t cluster_idx1, int64_t cluster_idx2)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(39, dbname, cluster_idx1, cluster_idx2);
	float sim = 0;
	chrd >> sim;
	return sim;
	ISE_API_END
}

void IseApi::multi_retrieve(const string& session_id, const string& src_dbname, const string& dst_dbname, const string& wantParaSrc, 
								const string& wantParaDst, const string& whereStm, float min_sim, uint32_t max_rec)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(45, session_id, src_dbname, dst_dbname, wantParaSrc, wantParaDst, whereStm, min_sim, max_rec);
	return;
	ISE_API_END
}

void IseApi::multi_cluster_retrieve(const string& session_id, const string& src_dbname, const string& dst_dbname, const string& wantParaSrc, const string& wantParaDst, const string& whereStm, float min_sim)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(50, session_id, src_dbname, dst_dbname, wantParaSrc, wantParaDst, whereStm, min_sim);
	return;
	ISE_API_END
}

double IseApi::get_multi_retrieve_progress(const string& session_id)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(44, session_id);
	double progress = 0.0;
	chrd >> progress;

	return progress;
	ISE_API_END
}

void IseApi::multi_retrieve_stop(const string& session_id)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(46, session_id);

	return;
	ISE_API_END
}

void IseApi::get_multi_retrieve_result(const string& session_id, unordered_map<shared_ptr<ImgRecord>, vector<shared_ptr<ImgRecord>>>& pRecs)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(47, session_id);
	uint32_t n = 0;
	int recCount = 0;
	int vrecCount = 0;
	chrd >> vrecCount;
	for (size_t i = 0; i < vrecCount; i++)
	{
		shared_ptr<ImgRecord> irec = make_shared<ImgRecord>();
		chrd >> irec->idx >> irec->sim;
		chrd >> n;
		irec->v_params.resize(n);
		for (auto& v : irec->v_params) 
		{
			chrd >> v;
		}

		vector<shared_ptr<ImgRecord>> irecVec;
		chrd >> recCount;
		for (uint32_t j = 0; j < recCount; j++)
		{
			shared_ptr<ImgRecord> r = make_shared<ImgRecord>();
			chrd >> r->idx >> r->sim;
			uint32_t paranum = 0;
			chrd >> paranum;
			r->v_params.resize(paranum);
			for (auto& v : r->v_params)
			{
				chrd >> v;
			}

			irecVec.push_back(r);
		}

		pRecs.insert(make_pair(irec, irecVec));
	}

	return;
	ISE_API_END
}

void IseApi::get_feature(const string& dbname, int64_t idx, string& feature)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(48, dbname, idx);
	chrd >> feature;

	ISE_API_END
}

void IseApi::get_cluster_feature(const string& dbname, int64_t cluster_idx, string& feature)
{
	ISE_API_START
		auto chrd = RT_CMD_COMM(51, dbname, cluster_idx);
	chrd >> feature;

	ISE_API_END
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ExtrApi::ExtrApi() :conn_tmo(4000)
{
	addr_ = new sockaddr_ex();

	sk_sys_init();
}

ExtrApi::~ExtrApi()
{
	delete addr_;
}

int ExtrApi::SetNetAddr(const char* ip, int port)
{
	addr_ = new sockaddr_ex();

	if (sk_tcp_addr(*addr_, ip, port) != 0) {
		string errmsg = system_errmsg().data();
		throw GeneralException2(ERROR_ISE_API_SET_EXTR_ADDR).format_errmsg("ExtrApi::setNetAddr failed: %s", system_errmsg().data());
		return -1;
	}
	return 0;
}

string ExtrApi::extract_feature(int ftype, const string& imgdata)
{
ISE_API_START
	auto chrd = RT_CMD_COMM(802, ftype, imgdata);
	string fdata;
	chrd >> fdata;
	return fdata;
ISE_API_END
}

string ExtrApi::extract_feature(int ftype, uint32_t w, uint32_t h, uint32_t pitch,
	const char* pixdata)
{
ISE_API_START
	auto chrd = RT_CMD_COMM(801, ftype, w, h, BBPW(pixdata, pitch*h));
	string fdata;
	chrd >> fdata;
	return fdata;
ISE_API_END
}
