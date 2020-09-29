#include "rmmt_wrap2.h"
#include <vector>

namespace rmmt {
using namespace std;

#ifdef __linux
#define RMMT_API extern
#else
#define RMMT_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"{
#endif

	//struct to hold API error informations
	struct rmmt_errinfo {
		int err_code;
		char err_msg[252];
	};

	//global structure & functions
	struct rmmt_init_para_t
	{
		const char* resource;
	};

	struct rmmt_membuf_t{
		char* data;
		size_t size;
	};

	RMMT_API int rmmt_global_init(rmmt_init_para_t* pp, rmmt_errinfo* errinfo);

	RMMT_API void rmmt_free_buffer(rmmt_membuf_t* pbuf);

	//(interprocess) memory node ( hold a memory block which can be shared among processes! )
	struct rmmt_memnode_t;

	RMMT_API rmmt_memnode_t* rmmt_alloc_memnode(uint32_t size, uint32_t align, rmmt_errinfo* errinfo);

	RMMT_API int rmmt_memnode_deref(rmmt_memnode_t* pn, rmmt_errinfo* errinfo);

	RMMT_API uint32_t rmmt_memnode_getsize(rmmt_memnode_t* pn);

	RMMT_API void* rmmt_memnode_getptr(rmmt_memnode_t* pn);

	//message trans channel
	struct rmmt_msgchan_t;

	RMMT_API rmmt_msgchan_t* rmmt_connect_server(const char* server_para, rmmt_errinfo* ei);

	RMMT_API int rmmt_release_msgchan(rmmt_msgchan_t* s);

	RMMT_API int rmmt_chan_send_msg(rmmt_msgchan_t* mc, const char* msg, size_t msg_len, rmmt_memnode_t** ppNode, size_t nNodes, rmmt_errinfo *ei);

	RMMT_API int rmmt_chan_recv_msg(rmmt_msgchan_t* mc, rmmt_membuf_t* pbRecvMsg, rmmt_membuf_t* pbRecvMemnodes, rmmt_errinfo *ei);

	RMMT_API int rmmt_chan_close(rmmt_msgchan_t* s);

	RMMT_API bool rmmt_chan_isvalid(rmmt_msgchan_t* s);

	struct rmmt_msgserver_t;


	RMMT_API rmmt_msgserver_t* rmmt_create_msgserver(const char* server_init_para, rmmt_errinfo* ei);

	RMMT_API rmmt_msgchan_t* rmmt_msgserver_accept_chan(rmmt_msgserver_t* ms, rmmt_errinfo* ei);

	RMMT_API int rmmt_msgserver_close(rmmt_msgserver_t* ms);




#ifdef __cplusplus
}
#endif

void* MemNode::get_ptr() const {
	rmmt_memnode_t* pn = (rmmt_memnode_t*)p;
	return rmmt_memnode_getptr(pn);
}

uint32_t MemNode::get_size() const {
	rmmt_memnode_t* pn = (rmmt_memnode_t*)p;
	return rmmt_memnode_getsize(pn);
}

MemNode::~MemNode()
{
	rmmt_memnode_t* pn = (rmmt_memnode_t*)p;
	rmmt_memnode_deref(pn, nullptr);
}

//-------------------------------------
void global_init(const string& init_para)
{
	rmmt_init_para_t pp = {};
	pp.resource = init_para.data();
	rmmt_errinfo ei = {};
	int rt = rmmt_global_init(&pp, &ei);
	if (rt) throw GeneralException2(rt, ei.err_msg);
}

static shared_ptr<MemNode> _make_memnode(rmmt_memnode_t* pn)
{
	//tricky!haha!
	struct MemNode_sharedenabler :public MemNode {
		MemNode_sharedenabler(void* x) :MemNode(x) {}
	};
	return std::make_shared<MemNode_sharedenabler>(pn);
}

shared_ptr<MemNode> allocate_memnode(uint32_t size, uint32_t align /* = 8 */)
{
	rmmt_errinfo ei = {};
	rmmt_memnode_t*pn = rmmt_alloc_memnode(size, align, &ei);
	if (!pn) throw GeneralException2(ei.err_code, ei.err_msg);
	return _make_memnode(pn);
}

//-------------------------------------------
// void MsgChannel::mc_callback(void* _s, int errcode, const char* msg, size_t msg_len, void* _ppNode, size_t nNodes, void* opaque)
// {
// 	MsgChannel* self = (MsgChannel*)opaque;
// 	rmmt_memnode_t** ppNode = (rmmt_memnode_t**)_ppNode;
// 	vector<shared_ptr<MemNode> > vMN(nNodes);
// 	struct MemNode_sharedenabler :public MemNode {
// 		MemNode_sharedenabler(void* x) :MemNode(x) {}
// 	};
// 	for (size_t i = 0; i < nNodes; i++) {
// 		vMN[i]= std::make_shared<MemNode_sharedenabler>(ppNode[i]);
// 	}
// 	self->cb(*self, errcode, msg, msg_len, nNodes ? vMN.data() : nullptr, nNodes);
// }

bool MsgChannel::is_valid() const
{
	return p != nullptr && rmmt_chan_isvalid((rmmt_msgchan_t*)p);
}


void MsgChannel::send_msg(const char* msg, size_t msg_len, const shared_ptr<MemNode>* vMemNodes /* = nullptr */, size_t nMemNode /* = 0 */)
{
	if(!is_valid()) throw GeneralException2(-19, "MsgChannel is not running. Cannot send msg!");
	rmmt_msgchan_t* mc = (rmmt_msgchan_t*)p;
	vector<rmmt_memnode_t*> vpmn(nMemNode);
	for (size_t i = 0; i < nMemNode; i++) {
		vpmn[i] = (rmmt_memnode_t*)vMemNodes[i]->p;
	}
	rmmt_errinfo ei = {};
	if (rmmt_chan_send_msg(mc, msg, msg_len, nMemNode ? vpmn.data() : nullptr, nMemNode, &ei))
		throw GeneralException2(ei.err_code, ei.err_msg);
}

std::string MsgChannel::recv_msg(std::vector<shared_ptr<MemNode> >& vMemNodes)
{
	if (!is_valid()) throw GeneralException2(-19, "MsgChannel is not running. Cannot recv msg!");
	rmmt_msgchan_t* mc = (rmmt_msgchan_t*)p;
	rmmt_errinfo ei = {};
	rmmt_membuf_t bMsg = {}, bNodes = {};
	int rt = rmmt_chan_recv_msg(mc, &bMsg, &bNodes, &ei);
	if (rt == 100) {//EOF!
		throw GeneralException2(0, "EOF");
	}
	if(rt)
		throw GeneralException2(ei.err_code, ei.err_msg);
	std::string retdata(bMsg.data, bMsg.size);
	rmmt_free_buffer(&bMsg);
	if (bNodes.size) {
		rmmt_memnode_t** ppmn = (rmmt_memnode_t**)bNodes.data;
		size_t nmn = bNodes.size / sizeof(rmmt_memnode_t*);
		vMemNodes.resize(nmn);
		for (size_t i = 0; i < nmn; i++) {
			vMemNodes[i] = _make_memnode(ppmn[i]);
		}
		rmmt_free_buffer(&bNodes);
	}
	return retdata;
}

shared_ptr<MsgChannel> MsgChannel::connect_server(const string& conn_para)
{
	rmmt_errinfo ei = {};
	rmmt_msgchan_t* p = rmmt_connect_server(conn_para.data(), &ei);
	if (p == 0)
		throw GeneralException2(ei.err_code, ei.err_msg);
	struct MsgChannel_se :public MsgChannel {
		MsgChannel_se(void* x) :MsgChannel(x) {}
	};
	return std::make_shared<MsgChannel_se>(p);
}

void MsgChannel::close()
{
	if (p) {
		rmmt_chan_close((rmmt_msgchan_t*)p);
	}
}

MsgChannel::~MsgChannel()
{
	if (p) {
		rmmt_chan_close((rmmt_msgchan_t*)p);
		rmmt_release_msgchan((rmmt_msgchan_t*)p);
		p = nullptr;
	}
}

//-------------------------------------------------
// void MsgServer::ms_callback(void* ms, int server_ec, void* new_chan, void* opaque)
// {
// 	MsgServer* self = (MsgServer*)opaque;
// 	struct MC_sharedenabler :public MsgChannel {
// 		MC_sharedenabler(void* x) :MsgChannel(x) {}
// 	};
// 	shared_ptr<MC_sharedenabler> rmc = make_shared<MC_sharedenabler>(new_chan);
// 	self->cb(*self, server_ec, rmc);
// }

bool MsgServer::is_valid() const {
	//rmmt_msgserver_t* ps = (rmmt_msgserver_t*)p;
	return p!=nullptr;
}

void MsgServer::close()
{
	if (p) {
		rmmt_msgserver_close((rmmt_msgserver_t*)p);
		p = nullptr;
	}
}

shared_ptr<MsgServer> MsgServer::create_server(const string& init_para)
{
	struct MS_sharedenabler:public MsgServer {
		MS_sharedenabler(void* c) :MsgServer(c) {}
	};
	
	rmmt_errinfo ei = {};
	rmmt_msgserver_t* ps = rmmt_create_msgserver(init_para.data(), &ei);
	if (!ps)
		throw GeneralException2(ei.err_code, ei.err_msg);
	
	return make_shared<MS_sharedenabler>(ps);
}

shared_ptr<MsgChannel> MsgServer::accept_chan()
{
	struct MS_sharedenabler :public MsgChannel {
		MS_sharedenabler(void* c) :MsgChannel(c) {}
	};
	if (!p) throw GeneralException2(-1, "MsgServer invalid");
	rmmt_errinfo ei = {};
	rmmt_msgchan_t* pc= rmmt_msgserver_accept_chan((rmmt_msgserver_t*)p, &ei);
	if (!pc) 
		throw GeneralException2(ei.err_code, ei.err_msg);
	return make_shared<MS_sharedenabler>(pc);
}

}///END of namespace///
