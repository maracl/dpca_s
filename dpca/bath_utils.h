#ifndef _BATH_UTILS_H
#define _BATH_UTILS_H
// #ifdef _MSC_VER
// #pragma once
// #endif // _MSC_VER

#include "businesspkg_common.h"
#include "boyun_IPC_v2.h"
#include "spdlog_helper.h"
#include <memory>
#include "basic_utils.h"
//#include "rmmt_svc_sessions.h"
#include <tbb/reader_writer_lock.h>
#include "rmmt_shm_image.h"
#include "utility_method.h"
//#include "rapidjson/Rjson.hpp"
using namespace std;
using namespace rmmt;
using std::shared_ptr;
//算法包应用实例句柄
struct bp_algopkg_t
{
	string handle_name;
	string rmmt_url;
	string cfg;
	std::shared_ptr<rmmt::IPCv2> pIpc;
	bool is_conn;
	bool is_init;
	bp_algopkg_t(const char* name, const char* url);
};

typedef struct alarm_info_t
{
	string  taskid;
	int     dataSource;
    string  keyUserId;
	string  keyPlaceId;
	string  departmentId;
	string  controlReason;
	alarm_info_t(){
		dataSource = 0;
	}
}alarm_info_t;


typedef struct control_info_t
{
	int     type;
	string  featy;
	string  ImgUrl;
}control_info_t;
typedef std::shared_ptr<control_info_t> control_t;//zj 20200428 布控

typedef struct Cordinate
{
	int left_top_x;
	int left_top_y;
	int width;
	int height;
}Cordinate;

typedef struct bigdata_info_t
{
	//string  uuid;
	//string  param_detect;
	string featy;
	obj_info_t obj;
	string  param_control;
	bigdata_info_t(){
		obj = {};
	}
}bigdata_info_t;
typedef std::shared_ptr<bigdata_info_t> bigdata_t;//zj 20200428 推送大数据


typedef enum PushType
{
	BIGPUSH = 1,
	SMALLPUSH = 2,
	IMAGEPUSH = 3 
}PushType;




//算法包应用实例创建的会话session句柄

struct bp_alogpkg_sess_t
{
	void* ap_sess;
	bp_algopkg_t* ap_hdl;
        bp_alogpkg_sess_t(){ ap_sess = nullptr; ap_hdl = nullptr;}
};


typedef std::shared_ptr<FTaskPool2> FTaskPool_t;
typedef std::shared_ptr<FTaskPool4> FTaskPool4_t;
typedef std::shared_ptr<bp_algopkg_t> bp_algo_sptr;
typedef std::shared_ptr<bp_alogpkg_sess_t> bp_sess_ptr;


class AlgoConnector
{
public:
	AlgoConnector();
	//析构函数安全等待vas/vss退出！
	~AlgoConnector();
public:
	std::vector<control_t> m_ControlVec;
	tbb::concurrent_bounded_queue<bigdata_t>  m_BigdataQue;
	alarm_info_t m_control;
	bp_algo_sptr m_algo_pkg;
	bp_sess_ptr  m_sess_t;
	bool volatile _quit_flag;
        bool volatile final_quit;
	bool volatile relate_flag;//行人人脸关联标志
	tbb::reader_writer_lock m_rwl;

	bool Init_Conn(const char* name);
	bool Start_Task(const char* param, int& errorCode);
	void End_task(bool wait_flag);
	ap_output_t Alg_Sess_Process(inputinfo_t* input);
	ap_output_t Alg_Process(inputinfo_t* input);
	ap_output_t algo_sess_proc_recv();
	void algo_sess_proc_send(inputinfo_t* input);
	void algo_proc_send(inputinfo_t* input);
	ap_output_t algo_proc_recv();
	void set_ipc_end();
	void _send_algo_frame(rmmt::ShmImage& image,int64_t pts);
        void _send_relate_frame(const rmmt::ShmVecType& req_vec,const string& req_data,const string& feat1,const string& feat2,const string& param,int do_type = 1);
    ap_output_t algo_relate_proc_recv(int do_type,const string& feat);

        void _send_null_frame();
	void do_relate();
    ap_output_t _send_null_frame_wait();
	int get_frame_load();
	void algopkg_output_release(ap_output_t* output);
        void do_ws_response(const string& param,const string& imgdata,const string& devid,int64_t pts,const string& captureId);
        void do_ws_response1(const string& param,const string& imgdata,const string& devid,int64_t pts,const string& captureId,int wsflg);
        void do_face_control_alarm(const string& srcdata,const string& bkg_img,const string& relateparam,const string& feat,int64_t pts,int dbNo);
	//////////////////////////////////////////////////////////////////////////
	FTaskPool4_t m_stp = std::make_shared<FTaskPool4>(16);
	weak_ptr<FTaskPool4> m_tp;// = m_stp;
private:
	int64_t        m_relateSkip;
	bool           m_brelate;
};

#endif//_BATH_UTILS_H
