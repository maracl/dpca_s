#include "boyun_IPC_v2.h"
#include "basic_utils.h"
#include "spdlog_helper.h"
#include "../dpca/Util.h"
#include <assert.h>
#include "byte_buffer.h"
#include "defer.h"
#include "system.h"

//extern void FormatLogA(const char* format, ...);
namespace rmmt{
using namespace std;
using namespace rmmt;


static const uint32_t IPCv2_VFLAG = 0x8004537;

//static atomic<int64_t> g_ipcUid(1);

IPCv2::~IPCv2()
{
    //if (m_chan) m_chan->close();
	//close(true);
}

void IPCv2::start()
{
	if(m_thrMainLoop.joinable()) return;
	assert(m_chan);
	m_thrMainLoop = std::thread([&] {
        m_thrStat = 1;
		try
		{
			for (vector<shared_ptr<MemNode> > vmn; m_fQuit==0 ;) 
			{
	            //FormatLogA("999999999999999999-----------ipc before recv!!!!\n");
				auto rdata = m_chan->recv_msg(vmn);
	            //FormatLogA("999999999999999999-----------ipc after  recv!!!!\n");
				if (rdata.empty()) 
				{
	                //FormatLogA("begin ipc main thread break null!!!!\n");
				   vector<shared_ptr<MemNode> > vmntemp;
				   vmn.swap(vmntemp);
				   //vmn.clear();
					throw GeneralException2(-99,"warning!!rdata empty");
				}
				CharSeqReader chrd(rdata.data(), rdata.size());
				int rt = parse_ipc_cmd(chrd, vmn);
				if(rt == -76)
				{
	               //FormatLogA("begin ipc main thread break 76!!!!\n");
				   vector<shared_ptr<MemNode> > vmntemp;
				   vmn.swap(vmntemp);
				   //vmn.clear();
			       throw GeneralException2(-76,"task return eof!!");
				   //break;
				}
                if(m_fQuit == 1)
                {
	                //FormatLogA("begin ipc main thread break null!!!!\n");
				   vector<shared_ptr<MemNode> > vmntemp;
				   vmn.swap(vmntemp);
				   //vmn.clear();
					throw GeneralException2(-99,"warning!!ready to quit");
					//break;
                }
				   vector<shared_ptr<MemNode> > vmntemp;
				   vmn.swap(vmntemp);
				   //vmn.clear();
			}
		}
		catch (GeneralException2 &e)
		{
		    
	        //FormatLogA("begin lock!!!----catch ipc exception,%d--%s\n",e.err_code(),e.err_str());
			tbb::reader_writer_lock::scoped_lock lck(m_imRwl);
	        //FormatLogA("end lock!!!----catch ipc exception,%d--%s\n",e.err_code(),e.err_str());
			auto iter = m_ipcMapCb.begin();
			while (iter != m_ipcMapCb.end())
			{
				IPC_resp_callback_t cb = iter->second;
				vector<shared_ptr<MemNode> > vmn;
				IPCException err_ipc;
				err_ipc.ret_code = e.err_code();
				err_ipc.err_msg = e.err_msg();
				f_pushtask([vmn, err_ipc, cb]() mutable {
					try{
						cb(&err_ipc, "", vmn);
					}
					catch (...) 
					{
						printf("f_pushtask.cb throws exception in GeneralException2 handler!\n");
					}
				}, (uint32_t)iter->first);

				iter++;
			}
			//m_ipcMapCb.clear();
			//m_pendingTasks = 1;
			m_fQuit = 1;
			//FormatLogA("ipc start false---%d----%s,,\n",e.err_code(),e.err_str());
			set_chanerror(true);
		}
		catch (...)
		{
	        //FormatLogA("begin lock!!!----catch ipc exception\n");
			tbb::reader_writer_lock::scoped_lock lck(m_imRwl);
	        //FormatLogA("end lock!!!----catch ipc exception\n");
			auto iter = m_ipcMapCb.begin();
			while (iter != m_ipcMapCb.end())
			{
				IPC_resp_callback_t cb = iter->second;
				vector<shared_ptr<MemNode> > vmn;
				IPCException err_ipc;
				err_ipc.ret_code = -1;
				err_ipc.err_msg = "IPCv2::start error in recv_msg";
				f_pushtask([vmn, err_ipc, cb]() mutable {
					try{
						cb(&err_ipc, "", vmn);
					}
					catch (...) {
						printf("f_pushtask.cb throws exception in <...> handler!\n");
					}
				
				}, (uint32_t)iter->first);

				iter++;
			}
			//m_ipcMapCb.clear();
			//m_pendingTasks = 1;
			//FormatLogA("ipc start catch an unknown false\n");
			m_fQuit = 1;
			set_chanerror(true);
		}
        m_thrStat = 2;
	});
}

void IPCv2::wait_restart()
{
   if (m_thrMainLoop.joinable())
	   m_thrMainLoop.join();
}

void IPCv2::close(bool bForce /* = false */)
{
     m_fQuit = 1;
	 //FormatLogA("ipc close catch an unknown false\n");
     set_chanerror(true);
     if(m_thrStat != 0)
     {
        if(m_thrMainLoop.joinable())
        {
	      //FormatLogA("here!!!----ipc thread join begain,the thread flag is %d--\n",(int)m_thrStat);
              m_thrMainLoop.join();
	      //FormatLogA("here!!!----ipc thread join end,the thread flag is %d--\n",(int)m_thrStat);
        } 
     }
	 
     {
        tbb::reader_writer_lock::scoped_lock lck(m_imRwl);
		std::unordered_map<uint64_t, IPC_resp_callback_t> tempCb;
		m_ipcMapCb.swap(tempCb);
        //m_ipcMapCb.clear();
		m_pendingTasks = 0;
     }
     if (m_chan) m_chan->close();
     //FormatLogA("here!!!----ipc closed\n");
         
}

int  IPCv2::check_map()
{

    //int64_t res = m_pendingTasks.load(std::memory_order_seq_cst);
	//int64_t sum = m_unitFrames.  
	return m_unitFrames.load(std::memory_order_seq_cst);
}

int64_t IPCv2::async_IPC(const void* req_data, size_t req_size, const ShmVecType& req_vec, const IPC_resp_callback_t& cb)
{
	//int64_t _uid = g_ipcUid.fetch_add(1);
	int64_t _uid = m_ipcUid.fetch_add(1);
	byte_buffer bb;
	//step 1 按照协议构造请求数据
	bb << IPCv2_VFLAG << int(1) << _uid << BBPW(req_data, req_size);
	//step 2 添加到ipc-callback-map
	{
		tbb::reader_writer_lock::scoped_lock lck(m_imRwl);
		m_ipcMapCb[_uid] = cb;
        m_pendingTasks.fetch_add(1);
	}
	try
	{
		if (req_vec.empty())
		{
			m_chan->send_msg(bb.data_ptr(), bb.data_size());
		}
		else
		{
			m_chan->send_msg(bb.data_ptr(), bb.data_size(), req_vec.data(), req_vec.size());
		}
		
	}
	catch(GeneralException2 &e)
	{
			vector<shared_ptr<MemNode> > vmn;
			IPCException err_ipc;
			err_ipc.ret_code = e.err_code();
			err_ipc.err_msg = e.err_msg();
			f_pushtask([vmn, err_ipc, cb]() mutable {
				try{
					cb(&err_ipc, "", vmn);
				   }
				   catch (...) 
				   {
						printf("f_pushtask.cb throws exception in GeneralException2 handler!\n");
				   }
			},_uid);

	}
	catch (...) 
	{
#if 0
		tbb::reader_writer_lock::scoped_lock lck(m_imRwl);
		m_ipcMapCb.erase(_uid);
        m_pendingTasks.fetch_sub(1);
		throw;
#endif
		vector<shared_ptr<MemNode> > vmn;
		IPCException err_ipc;
		err_ipc.ret_code = -1;
		err_ipc.err_msg = "unhandled error!";
		f_pushtask([vmn, err_ipc, cb]() mutable {
				try{
					cb(&err_ipc, "", vmn);
				   }
				   catch (...) 
				   {
						printf("f_pushtask.cb throws exception in GeneralException2 handler!\n");
				   }
			},_uid);
	}
	return _uid;
	
}

void IPCv2::do_IPC_NW(const void* req_data, size_t req_size, const ShmVecType& req_vec, string& resp_data, ShmVecType& resp_vec,int type,const string& bindparam,const string& input)
{
	int64_t rtid  = async_IPC(req_data, req_size, req_vec, [&,input,req_vec,type,bindparam](IPCException* _e, const string& _resp_data, const ShmVecType& _resp_vec){
        if(m_fQuit == 1)
		{
		  return;
		}
	    algo_rsp_t algpt = make_shared<cache_algo_frm>();
		if(_e)
		{
		   algpt->ipc_e = *_e;
		}
		else
		{
		   if(type == 2) 
		   {
#if 1

#else
		   byte_buffer bb;
		   bb << BBPW(req_data, req_size);
			   CharSeqReader chrd(bb.data_ptr(),bb.data_size());
			    int cmd;
				chrd >> cmd;
			   printf("begin chrd input %d\n",cmd);
				string input;
				chrd >> input;
			   printf("end chrd input-%s\n",input.data());
			   string cmddata;
			   chrd >> cmddata;
			   CharSeqReader chrd_copy(cmddata.data(), cmddata.size());
			   int cmd_copy;
			   chrd_copy >> cmd_copy;
			   string input;
			   chrd_copy >> input;
#endif
			   algpt->algo_srcdata = input;
		   }
		   algpt->algo_srcvec = req_vec;
		   algpt->algo_rspdata = _resp_data;
		   algpt->algo_rspvec = _resp_vec;
		   algpt->bind_param = bindparam;
		}
		m_decRspQue[type - 1].push(algpt);
		m_unitFrames.fetch_sub(1);
	});
	m_unitFrames.fetch_add(1);
}


void IPCv2::do_IPC(const void* req_data, size_t req_size, const ShmVecType& req_vec, string& resp_data, ShmVecType& resp_vec)
{
	//if (m_fQuit == 1)
	//{
	//	throw GeneralException2(-1, "socket chan is disconnected!!!!!!!!!!!!!");
	//	return;
	//}
	SimpleNotifier nt;
	IPCException e;
	int64_t rtid  = async_IPC(req_data, req_size, req_vec, [&](IPCException* _e, const string& _resp_data, const ShmVecType& _resp_vec){
	   //timeout.fetch_sub(1);
        if (_e) {
			e = *_e;
		}
	    else 
        {
			resp_data = _resp_data;
			resp_vec = _resp_vec;
	    }
		nt.wake();
	});
	LOG_DEBUG(ANAL_LOG, "IPC: {} begain to wait:", rtid);
	nt.wait();
	LOG_DEBUG(ANAL_LOG, "IPC: {} end    to wait:", rtid);
	if (e.ret_code != 0 || !e.err_msg.empty())
		throw e;
   
}

int IPCv2::parse_ipc_cmd(CharSeqReader& chrd, vector<shared_ptr<MemNode>>& vmn)
{
	try
	{
		if (chrd.leftbytes() < 8) return -1;
		uint32_t _vflag = 0, _mtype = 0;
		chrd >> _vflag >> _mtype;
		if (_vflag != IPCv2_VFLAG) return -2;
		switch (_mtype)
		{
		case 0x1:	//IPC request: should async do ipc-callbak
		{
			string cmddata;
			int64_t uid;
			chrd >> uid >> cmddata;
			//m_pendingTasks++;
			f_pushtask([this, vmn, cmddata, uid] {
				//defer([this] {  m_pendingTasks--; });
				try{
					string repdata;
					vector<shared_ptr<MemNode>> repvmn;
					this->m_ipcCb(cmddata, vmn, repdata, repvmn);

					byte_buffer bb(256);
					bb << IPCv2_VFLAG << int(2) << uid << BBPW(repdata.data(), repdata.size());

					this->m_chan->send_msg(bb.data_ptr(), bb.data_size(), repvmn.data(), repvmn.size());
				}
				catch (...) {
					printf("parse_ipc_cmd.cmd=0x1 f_pushTask inner exception caught!");
				}

			}, (uint32_t)uid);
		}
		break;
		case 0x2: //IPC response recved.
		case 0x3: //exception got!
		{
			int64_t uid;
			chrd >> uid;
			IPC_resp_callback_t cb;
			{
				tbb::reader_writer_lock::scoped_lock_read rlck(m_imRwl);
				auto it = m_ipcMapCb.find(uid);
				if (it != m_ipcMapCb.end()) {
					cb = it->second;
				}
			}
			if (cb)
			{
				if (_mtype == 3) {
					IPCException e;
					chrd >> e.ret_code >> e.err_msg;
					f_pushtask([vmn, e, cb]() mutable {
						try{
							cb(&e, "", vmn);
						}
						catch (...) {
							printf("parse_ipc_cmd.cmd=0x3 f_pushTask inner exception caught!");
						}

					}, (uint32_t)uid);
					
				}
				else {
					string cmddata;
					chrd >> cmddata;
					f_pushtask([vmn, cmddata, cb]() mutable {
						try{
							cb(nullptr, cmddata, vmn);
						}
						catch (...) {
							printf("parse_ipc_cmd.cmd=0x2 f_pushTask inner exception caught!");
						}
					}, (uint32_t)uid);

					CharSeqReader chrd_copy(cmddata.data(), cmddata.size());
					int cmd_copy;
					chrd_copy >> cmd_copy;
					if(cmd_copy == 76)
					{
					    //FormatLogA("check the close cmd is %d",cmd_copy);
					    tbb::reader_writer_lock::scoped_lock lck(m_imRwl);
					    m_ipcMapCb.erase(uid);
                        m_pendingTasks.fetch_sub(1);
					    return -76;
					}
				}

				{
					tbb::reader_writer_lock::scoped_lock lck(m_imRwl);
					m_ipcMapCb.erase(uid);
                    m_pendingTasks.fetch_sub(1);
				}
			}
		}
		break;
		default:
			return -3;
			break;
		}
	}
	catch (...) {
		return -10;
	}

	return 0;
}

void IPCv2::register_IPC_callback(const IPC_callback_t& cb)
{
	tbb::reader_writer_lock::scoped_lock lck(m_imRwl);
	m_ipcCb = cb;
}



}///END of namesace ////////
