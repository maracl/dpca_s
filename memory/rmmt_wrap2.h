#ifndef _BOYUN_RMMT_WRAP_CXX_H
#define _BOYUN_RMMT_WRAP_CXX_H
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "General_exception2.h"

//remote message & (huge)memory transfer (RMMT) SDK
namespace rmmt {
using std::shared_ptr;
using std::string;
using std::function;

class MsgChannel;

class MemNode {
	MemNode(const MemNode&);
	friend class MsgChannel;
protected:
	void* p;
	MemNode(void* _p) :p(_p) {}
public:
	~MemNode();
	void* get_ptr() const;
	uint32_t get_size() const;
};

//throwable
void global_init(const string& init_para);

//throwable
shared_ptr<MemNode> allocate_memnode(uint32_t size, uint32_t align = 8);


class MsgChannel {
	MsgChannel(const MsgChannel&);
	void* p;
protected:
	explicit MsgChannel(void* x) :p(x) {}
public:
	~MsgChannel();

	bool is_valid() const;

	//throwable!
	void send_msg(const char* msg, size_t msg_len, const shared_ptr<MemNode>* vMemNodes = nullptr, size_t nMemNode = 0);

	void send_msg(const string& msgdata, const shared_ptr<MemNode>* vMemNodes = nullptr, size_t nMemNode = 0) {
		send_msg(msgdata.data(), msgdata.size(), vMemNodes, nMemNode);
	}
	//throwable!
	std::string recv_msg(std::vector<shared_ptr<MemNode> >& vMemNodes);

	static shared_ptr<MsgChannel> connect_server(const string& conn_para);

	void close();
};


class MsgServer {
	MsgServer(const MsgServer&);
	void* p;
protected:
	explicit MsgServer(void* x) :p(x) {}
public:
	
	bool is_valid() const;

	static shared_ptr<MsgServer> create_server(const string& init_para);

	//throwable!
	shared_ptr<MsgChannel> accept_chan();

	void close();
};


}//END of namespace rmmt


#endif //_BOYUN_RMMT_WRAP_CXX_H