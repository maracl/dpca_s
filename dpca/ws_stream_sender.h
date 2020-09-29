#ifndef _WS_STREAM_SENDER_H
#define _WS_STREAM_SENDER_H

#include "HttpSession.h"
#include <memory>
#include <functional>
#include <string>
#include <atomic>
#include <tbb/concurrent_queue.h>
#include "byte_buffer.h"
#include "General_exception2.h"

struct RedisBidId
{
   std::string normalId;
   int    type;//1:定期提交回调 2:过期销毁回调 
   RedisBidId(){
	   normalId = "";
	   type = -1;
   };
   RedisBidId(std::string id,int cmd){
       normalId = id;
       type = cmd;
   }; 
};
typedef std::shared_ptr<RedisBidId>  control_ct_ptr;
class FrameWriter
{
	FrameWriter(const FrameWriter&);
public:
	FrameWriter()
	{
		m_que.set_capacity(8);
		m_b1stframe = true;
		m_stt = 0;
	}

	bool is1stframe() const 
	{
		return m_b1stframe;
	}

	void push(const std::shared_ptr<byte_buffer>& f) 
	{
		m_que.push(f);
		m_b1stframe = false;
	}

	bool try_push(const std::shared_ptr<byte_buffer>& f)
	{
		if (m_que.try_push(f)) 
		{
			m_b1stframe = false;
			return true;
		}
		return false;
	}

	void pop(std::shared_ptr<byte_buffer>& f) {
		m_que.pop(f);
	}

	void set_stt(int s)
	{
		//fmt::print("set_stt : {}\n", s);
		m_stt.store(s);
	}

	int get_stt() const 
	{
		return m_stt;
	}

	//向ws端通知svc端关闭
	void notify_svc_quit() 
	{
		try_push({});
		m_que.abort();
	}
private:
	tbb::concurrent_bounded_queue<std::shared_ptr<byte_buffer> > m_que;
	std::atomic<int> m_stt;
	bool m_b1stframe;
};
/*
	外部实现
*/
extern int RegisterWsConnection(void* hdl,std::shared_ptr<FrameWriter>& fw,const std::string& init_msg);
THROWABLE(void) InitWebsocketServerAndStartLoop(int sport);
THROWABLE(void) InitGusetChan(const std::string& sessionid);
void FaceControlBegain1(const char* request);
void FaceControlBegain(const char* request, size_t req_size, bp_http_hdl_t* hdl);
void FaceControlCancel(const char* request, size_t req_size, bp_http_hdl_t* hdl);
#endif
