#ifndef _BOYUN_RMMT_IPC_V2_H
#define _BOYUN_RMMT_IPC_V2_H

#include <atomic>
#include <thread>
#include <functional>
#include <unordered_map>
#include <atomic>
#include "rmmt_wrap2.h"
#include "tbb/concurrent_queue.h"
#include <tbb/reader_writer_lock.h>
#include "common_interface.h"
#include "General_exception2.h"
#include "byte_buffer.h"

//also use 'rmmt' namespace
namespace rmmt{

using TaskPusherFunc = std::function<void(const std::function<void()>& , uint32_t)>;
using ShmVecType = std::vector<shared_ptr<MemNode> >;
class IPCException  {
public:
	string err_msg;
	int ret_code;
	IPCException() :ret_code(0) {err_msg.clear();}
};

typedef struct cache_algo_frm
{
	    ShmVecType   algo_rspvec;
		ShmVecType   algo_srcvec;
		string       algo_srcdata;
		string       algo_rspdata;
		string       bind_param;
		IPCException ipc_e;
		cache_algo_frm() {
			algo_srcdata.clear();
			algo_rspdata.clear();
			bind_param.clear();
        }
		~cache_algo_frm(){
			algo_rspdata.clear();
			algo_rspvec.clear();
			algo_srcvec.clear();
			bind_param.clear();
		}
}cache_algo_frm;

typedef std::shared_ptr<cache_algo_frm>  algo_rsp_t;

class IPCv2:no_copy{
public:
	
	//(响应端)IPC响应回调函数类型
	using IPC_callback_t = std::function<void(const string & req_data, const ShmVecType & req_vec, string & resp_data, ShmVecType & resp_vec)>;
	
	//(请求端)IPC回复回调函数类型
	using IPC_resp_callback_t = std::function<void(IPCException* e, const string& resp_data, const ShmVecType& resp_vec)>;
	
	//(响应端)AsyncQueue响应回调函数类型
	using AQ_callback_t = std::function<void(const string& resp_data, const ShmVecType& resp_vec)>;
	
	explicit IPCv2(){}
	explicit IPCv2(const shared_ptr<MsgChannel>& c, const TaskPusherFunc& tp) :m_chan(c), f_pushtask(tp),m_fQuit(0),m_pendingTasks(0),chan_error(false),m_thrStat(0),m_ipcUid(1),m_unitFrames(0) {
		//m_algoRspQue.set_capacity(42);m_fetRspQue.set_capacity(42);m_comRspQue.set_capacity(42);
		for(int i = 0;i < 3;i++)
		{
            m_decRspQue[i].set_capacity(4200);
		}
	}

	~IPCv2();


	void start();

	void new_chan(const shared_ptr<MsgChannel>& c) { m_chan = c;}

	void normal_close() {if(m_chan) m_chan->close(); }

	void close(bool bForce = false);

	void re_init() { m_fQuit = 0; m_pendingTasks = 0; chan_error = false; m_thrStat = 0;}

	void wait_restart();

    int  check_map();


	///=============IPC接口实现======================
	void register_IPC_callback(const IPC_callback_t& cb);

	//throwable
	void do_IPC(const void* req_data, size_t req_size, const ShmVecType& req_vec, string& resp_data, ShmVecType& resp_vec);
	void do_IPC_NW(const void* req_data, size_t req_size, const ShmVecType& req_vec, string& resp_data, ShmVecType& resp_vec,int type,const string& bindparam,const string& input);

	int64_t async_IPC(const void* req_data, size_t req_size, const ShmVecType& req_vec,
	const IPC_resp_callback_t& cb);

	///=============异步有序队列实现======================
	void register_AQ_callback(const AQ_callback_t& cb);

	void push_AQ_msg(const void* req_data, size_t req_size, const ShmVecType& req_vec);

	void set_chanerror(bool chan_status){ chan_error = chan_status; }
	bool get_chanerror(){ return chan_error; }
	bool check_task_end(){
		if(m_pendingTasks < 10) return true;
		else return false;
	}
public:
	tbb::concurrent_bounded_queue<algo_rsp_t> m_decRspQue[3];
	//tbb::concurrent_bounded_queue<algo_rsp_t> m_fetRspQue;
	//tbb::concurrent_bounded_queue<algo_rsp_t> m_comRspQue;
	std::atomic<int> m_fQuit;
    std::atomic<int> m_thrStat;
private:

	int parse_ipc_cmd(CharSeqReader& bb, std::vector<shared_ptr<MemNode> >& vmn);

    bool chan_error;
	shared_ptr<MsgChannel> m_chan;
	TaskPusherFunc f_pushtask;
	//std::atomic<int> m_fQuit;
#if 0
	std::atomic<uint32_t> m_pendingTasks;
#else
	std::atomic<int64_t> m_pendingTasks;
	std::atomic<int>     m_unitFrames;
#endif
	//std::atomic<int> m_mapcount;
	
    std::atomic<int64_t> m_ipcUid;

	IPC_callback_t m_ipcCb;		//用于接收端接收IPC message并回复
	std::unordered_map<uint64_t, IPC_resp_callback_t> m_ipcMapCb;
	tbb::reader_writer_lock m_imRwl;
	//tbb::reader_writer_lock m_ipcRwl;

	std::unordered_map<uint64_t, AQ_callback_t> m_aqMapCb;
	tbb::reader_writer_lock m_amRwl;

	std::thread m_thrMainLoop;	//main message recv loop
};



}///END of namesace ////////


#endif //_BOYUN_RMMT_IPC_V1_H
