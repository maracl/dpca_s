#include "rmmt_svc_sessions.h"
#include <thread>
#include <unordered_map>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "rmmt_shm_image.h"
#include <tbb/reader_writer_lock.h>
#include <tbb/concurrent_queue.h>
#include "basic_utils.h"
#include "byte_buffer.h"
#include "ws_stream_sender.h"
//#include "xvid_enc.h"
#include "defer.h"
#include "Util.h"
#include "bath_utils.h" 
#include "spdlog_helper.h"
#include "rapidjson/Rjson.hpp"
#include "create_uuid.h"
#include "StreamCmdApis.h"
#include "kafka_client.h"
#include "download_image.h"
#include "http_his_client.h"

using namespace std;
using namespace rmmt;

using MsgChanPtr = shared_ptr<MsgChannel>;

using tbb::reader_writer_lock;
extern SimpleNotifier g_chan_nt;
extern std::string tokenStr;//token 字段

extern KafkaClient pFacCt;//人脸
extern KafkaClient pClust;//人脸
extern int c_max_failct;
/////////////////////////////////////////////////////
extern ExtrApi pExtrApi_Prd;
extern ExtrApi pExtrApi_Bik;
extern int extr_port, extr_port1;
extern std::string extr_ip, extr_ip1;
extern std::string appId;
extern std::string appSecret;
extern std::string cloudwalkIp;
extern int cloudwalkPort;
extern std::string cloudwalkServer;
extern std::string clusterName;
int img_from_data1(const std::string& img_data, cv::Mat& res)
{
	if (img_data.empty())
		//throw GeneralException2(-1, "data is empty");
		return -1;
	cv::Mat mat(cv::imdecode(cv::Mat(1, img_data.size(), CV_8UC3, (unsigned char*)img_data.data()), CV_LOAD_IMAGE_COLOR));
	if (!mat.data)
		//throw GeneralException2(-1, "image decode error");
		return -1;
	res = mat;
	return 0;
}
int img_resize1(const cv::Mat& mat, cv::Mat& resImg, double& ratio)
{
	if (!mat.data)
		//throw GeneralException2(-1, "mat is invalid");
		return -1;
	int w = mat.cols;
	int h = mat.rows;
	ratio = 1.000f;
	if (w > 1920 || h > 1080)
	{
		double ratCol = (double)w / 1920.000f;
		double ratRow = (double)h / 1080.000f;
		ratio = ratCol > ratRow ? ratCol : ratRow;
		int cols = (int)((double)w / ratio);
		int rows = (int)((double)h / ratio);
		cv::Size resImgSize = cv::Size(cols, rows);
		resImg = cv::Mat(resImgSize, mat.type());
		cv::resize(mat, resImg, resImgSize, CV_INTER_CUBIC);
	}
	else
	{
		resImg = mat;
	}
	return 0;
}
std::string extra_feature(int type, const string& img_data)
{
   string obj_feture;
   try
   {
	   switch (type)
	   {
		   case 2:
				  obj_feture = pExtrApi_Bik.extract_feature(type, img_data);
				  break;
		   case 3:
				  obj_feture = pExtrApi_Prd.extract_feature(type, img_data);
				  break;
		   default:
				  obj_feture = "";
				  break;
	   }

   }
   catch(GeneralException2& e) 
   {
	    LOG_INFO(MAIN_LOG,"extract_feature error:code: {},msg:{} !", e.err_code(), e.err_str());
		return string("");
   }
   return obj_feture;
}

////////////////////////////////////////////////////
//布控报警
#if 0
typedef struct alarm_info_t
{
	int type;
	int64_t snapTime;
	string  featy;
	cv::Ptr<IplImage> objimg;
	cv::Ptr<IplImage> bkgimg;
}alarm_info_t;
typedef std::shared_ptr<alarm_info_t> alarm_t;//zj 20200428 报警
typedef struct frame_info_t
{
	    ShmVecType   vmn;
	    frame_info_t() {
		vmn.clear();
        }
		~frame_info_t(){
			vmn.clear();
		}
}frame_info_t;

typedef std::shared_ptr<frame_info_t> frame_t;
#endif
namespace {


class SvcVideoSession
{
	MsgChanPtr m_chan;
    //tbb::concurrent_bounded_queue<alarm_t>  m_AlarmQue;
    //tbb::concurrent_bounded_queue<frame_t>  m_FrameQue;
	//reader_writer_lock m_rwl;
	AlgoConnector  m_algo;
	bool volatile  is_run_;
	std::vector<std::thread>    m_detect_thr;
	std::thread    m_feat_thr;
	std::thread    m_alarm_thr;
	std::thread    m_bigdata_thr;
	
public:
	SvcVideoSession(const MsgChanPtr& p,const char* name) :m_chan(p) {
		is_run_ = 1;
		//m_AlarmQue.set_capacity(250);
		//m_FrameQue.set_capacity(250);
		//m_ControlVec.reserve(20);
		m_detect_thr.resize(10);
		if (!m_algo.Init_Conn(name))
		{
			throw GeneralException2(-20, "alg is not ready!");
		}
	}
	~SvcVideoSession() 
	{
		is_run_ = 0;
		LOG_INFO(MAIN_LOG,"begin the release function!!\n");
#if 1
		{
			reader_writer_lock::scoped_lock lck(m_algo.m_rwl);
			//auto i = m_ControlVec.begin();
			//while(i != m_ControlVec.end())
			//{
			//	m_ControlVec.erase(i);
			//}
			std::vector<control_t> temp;
			temp.swap(m_algo.m_ControlVec);
		}
		LOG_INFO(MAIN_LOG,"clear vector!!\n");
#endif
		//m_AlarmQue.clear();
		//m_FrameQue.clear();
		try{
			for(int i = 0;i< m_detect_thr.size();i++) 
			{
				if(m_detect_thr[i].joinable()) m_detect_thr[i].join();
                        }
			if(m_feat_thr.joinable()) m_feat_thr.join();
			if(m_alarm_thr.joinable()) m_alarm_thr.join();
			if(m_bigdata_thr.joinable()) m_bigdata_thr.join();
			m_algo.set_ipc_end();
			m_algo.End_task(true);
			m_algo._send_null_frame();
			LOG_INFO(MAIN_LOG,"ipc release-{}!!\n");
		}
		catch(GeneralException2& e)
		{
			LOG_INFO(MAIN_LOG,"catch the general exp {}---{}",e.err_code(), e.err_str());
		}
		LOG_INFO(MAIN_LOG,"thread join-{}!!\n");
	}
#if 1
    void set_control_info(const string& vt)
	{
		//reader_writer_lock::scoped_lock lck(m_algo.m_rwl);
		//for(auto& i : vt) 
		//{
		//	m_algo.m_ControlVec.push_back(i);
		//}
        Document dc_f;
		Rjson::Parse(dc_f,vt);
		m_algo.m_control.controlReason = Rjson::GetString("controlReason", &dc_f);
		m_algo.m_control.dataSource =    Rjson::GetInt("dataSource", &dc_f);
		m_algo.m_control.departmentId = Rjson::GetString("departmentId", &dc_f);
		m_algo.m_control.keyPlaceId = Rjson::GetString("keyPlaceId", &dc_f);
		m_algo.m_control.keyUserId = Rjson::GetString("keyUserId", &dc_f);
		m_algo.m_control.taskid = Rjson::GetString("taskId", &dc_f);
		//m_algo.m_control.type = Rjson::GetInt("type", &dc_f);
	}
#endif
	int frame_and_dispatch(const ShmVecType& vmn,string param) 
	{
		try{
			m_algo._send_relate_frame(vmn,"","","",param,1);
		}
		catch(GeneralException2& e)
		{
			LOG_ERROR(MAIN_LOG,"2 catch the general exp {}---{}\n",e.err_code(), e.err_str());
			//continue;
		}
		return 0;
	}
       
	void do_task_open()
	{
		LOG_INFO(MAIN_LOG,"do start dpca task open!!");
		is_run_ = 1;
        for(int i = 0;i< m_detect_thr.size();i++) 
        {
           m_detect_thr[i]  = std::thread([&]{
#if 1
		LOG_INFO(MAIN_LOG,"do start thrad 111111");
            while(is_run_)
		    {
			try{
			   if(is_run_ == 1) m_algo._quit_flag = false;
			   else m_algo._quit_flag = true;
			   ap_output_t output = m_algo.algo_relate_proc_recv(1,"");
               if(output.errc == -6) 
			   {
			      m_algo.algopkg_output_release(&output);
			      break;
			   }
			   m_algo.algopkg_output_release(&output);
			}
			catch(GeneralException2& e)
			{
			LOG_INFO(MAIN_LOG,"1 catch the general exp {}---{}",e.err_code(), e.err_str());
			continue;
			}
		   }

			is_run_ = 0;
#endif
		});
        }
		m_feat_thr = std::thread([&]{
#if 1
		 LOG_INFO(MAIN_LOG,"do start thrad 22222");
			bool is_done = false;
				while(is_run_)
				{
				try{
					if(is_run_ == 1) m_algo._quit_flag = false;
					else m_algo._quit_flag = true;
					ap_output_t output = m_algo.algo_relate_proc_recv(2,"");
					if(output.errc == -6) 
					{
						LOG_ERROR(MAIN_LOG,"return catch a false,break");
						is_done = true;
					}
					m_algo.algopkg_output_release(&output);
					if(is_done) break;
					}
					catch(GeneralException2& e)
					{
					LOG_ERROR(MAIN_LOG,"2 catch the general exp {}---{}\n",e.err_code(), e.err_str());
					continue;
					}
				}
				is_run_ = 0;

#endif
		 LOG_INFO(MAIN_LOG,"do end thrad 22222");
		});
		m_alarm_thr = std::thread([&]{
#if 1
		LOG_INFO(MAIN_LOG,"do start thrad 33333");
			bool is_done = false;
			while(is_run_)
			{
			try{
			   if(is_run_ == 1) m_algo._quit_flag = false;
			   else m_algo._quit_flag = true;
			       ap_output_t output = m_algo.algo_relate_proc_recv(3,"");
                   if(output.errc == -6) 
				   {
			          //m_algo.algopkg_output_release(&output);
					  is_done = true;
					  //break;
			       }
			       m_algo.algopkg_output_release(&output);
				if(is_done) break;
			}
			catch(GeneralException2& e)
			{
				LOG_INFO(MAIN_LOG,"3 catch the general exp {}---{}",e.err_code(), e.err_str());
				continue;
			}
		}
			is_run_ = 0;
#endif
		});
		m_bigdata_thr = std::thread([&]{
#if 1
		LOG_INFO(MAIN_LOG,"do start thrad 44444");
			while(is_run_)
			{
			    LOG_INFO(MAIN_LOG,"begin pop---{}");
			    bigdata_t pbg = make_shared<bigdata_info_t>(); 
				while (!m_algo.m_BigdataQue.try_pop(pbg))
				{
					//LOG_INFO(MAIN_LOG,"begin try false---{}");
					if(is_run_) usleep(100);
					else break;
				}
			    LOG_INFO(MAIN_LOG,"end pop---{}");
				string json;
                            string clusterd; 
                try{
			    json = get_face_recong(pbg->obj,pbg->param_control,pbg->featy,clusterd);  
				}
				catch(GeneralException2& e)
				{
					LOG_INFO(MAIN_LOG,"4 catch the general exp {}---{}",e.err_code(), e.err_str());
					continue;
				}
				LOG_INFO(MAIN_LOG,"push to kafka---{}",json);
				pFacCt.kafka_client_poll(json.data());
				LOG_INFO(MAIN_LOG,"cluster to kafka---{}",clusterd);
				pClust.kafka_client_poll(clusterd.data());
			}
			is_run_ = 0;
#endif
		});
	}

};
	unordered_map<string, shared_ptr<SvcVideoSession>> g_vsmap;
	reader_writer_lock g_rwl;
}

//=====================================================
using namespace rmmt;

static void on_vdo_conn(shared_ptr<MsgChannel> chan)
{
	vector<shared_ptr<MemNode>> vmnd;
	std::string sess_id;
	{
		std::string rdata = chan->recv_msg(vmnd);
		LOG_INFO(MAIN_LOG, "({}) sessid recv!",rdata);
		CharSeqReader seqd(rdata.data(), rdata.size());
		seqd >> sess_id;
	}
	{
		reader_writer_lock::scoped_lock_read lck(g_rwl);
		auto it = g_vsmap.find(sess_id);
		if(it != g_vsmap.end())
			throw GeneralException2(-5, "task exit!");
	}
#if 1
	Document dc_n = Rjson::rWriteDC();
	Rjson::rAdd(dc_n, "algtype", StringRef(AP_CNN_FACE));
	Rjson::rAdd(dc_n, "relflg", 1);
	string name = Rjson::ToString(dc_n); 
#endif
	shared_ptr<SvcVideoSession> vdo_sess = make_shared<SvcVideoSession>(chan,name.data());
	{
		reader_writer_lock::scoped_lock lck(g_rwl);
		g_vsmap[sess_id] = vdo_sess;
	}
	LOG_INFO(MAIN_LOG, "({}) session registered!", sess_id);

	defer([sess_id]{
		LOG_INFO(MAIN_LOG, "({}) session Released!", sess_id);
		reader_writer_lock::scoped_lock lck(g_rwl);
		g_vsmap.erase(sess_id);
	});

	vdo_sess->do_task_open();
	{
		byte_buffer bb;
		bb << (int)0 << string("ok!");
		chan->send_msg(bb.data_ptr(), bb.data_size());
		LOG_INFO(MAIN_LOG,"do send response to hst");
	}
	for (;;)
	{
		vector<shared_ptr<MemNode>> vmn;
		//LOG_INFO(MAIN_LOG, "({}) begin recv!", sess_id);
		std::string rdata = chan->recv_msg(vmn);
		g_chan_nt.wake();
		//LOG_INFO(MAIN_LOG, "({}) end recv!",rdata);
		CharSeqReader chrd(rdata.data(), rdata.size());
		//LOG_INFO(MAIN_LOG, "({}) parser param!",rdata.data());
		int cmdid;
		chrd >> cmdid;
		//LOG_INFO(MAIN_LOG, "({}) >> operate!",cmdid);
#if 1
		//if(cmdid == 0)
		//{
        //    vdo_sess->do_task_open();
		//	{
		//		byte_buffer bb;
		//		bb << (int)0 << string("ok!");
		//		chan->send_msg(bb.data_ptr(), bb.data_size());
		//		LOG_INFO(MAIN_LOG,"do send response to hst");
		//	}
		//}
		if(cmdid == 1) 
		{
#if 1
			//if(vmn.empty())
			//{
			//	//throw GeneralException2(ERROR_WMVS_NO_MEMNODE, "received command 1(frame got) contains 0 MemNode!");
			//	continue;
			//}
			//rmmt::ShmImage simg(vmn[0]);
			//IplImage img = simg.toCvImage();
            int code;
			string input;
			chrd >> code;
			Document dc_w = Rjson::rWriteDC();
                        if(code == 1)
			{
				//LOG_INFO(MAIN_LOG,"do chrd input");
				chrd >> input;
				LOG_INFO(MAIN_LOG,"end chrd input---{}",input.data());
		        }
			else if(code == 5)
			{
				string info;
				chrd >> info;
				Document dc_w = Rjson::rWriteDC();
				Rjson::rAdd(dc_w, "RLBK", info);
				input = Rjson::ToString(dc_w);
 
			}
			//Rjson::rAdd(dc_w, "ImgUrl", imgurl);
			if (vdo_sess->frame_and_dispatch(vmn,input))
			{
				LOG_INFO(MAIN_LOG, "({}) session quit because of No success frame dispatch for A Long Time!", sess_id);
				//break;
				//continue;
			}
#endif
		}
		else if(cmdid == 2)
		{
		       LOG_INFO(MAIN_LOG, "({}) eof revceived just break!", sess_id);
               break;
		}
		else if(cmdid == 3)
		{
			string parjs;
			chrd >> parjs;
		    	vdo_sess->set_control_info(parjs);
			{
				byte_buffer bb;
				bb << (int)0 << string("ok!");
				chan->send_msg(bb.data_ptr(), bb.data_size());
			}
		}
		else
		{
			LOG_INFO(MAIN_LOG, "({}) session received unhandled command: {}", sess_id, cmdid);
		}
#endif
	}
}

void svc_session_server_start(const std::string& addr)
{
	auto msgServer = MsgServer::create_server(addr);
	//......
	std::thread([msgServer]{
		for (;;) 
		{
			try
			{
				auto chan = msgServer->accept_chan();
				std::thread([chan] {
					try
					{
						LOG_INFO(MAIN_LOG, "begin an element dpca!");
						on_vdo_conn(chan);
					}
					catch (GeneralException2& e)
					{
					   try{
						LOG_ERROR(MAIN_LOG, "on_svc_cli exception: {},{}", e.err_code(), e.err_str());
						byte_buffer bb;
						bb << e.err_code() << e.err_str();
						chan->send_msg(bb.data_ptr(), bb.data_size());	
						}
						catch (GeneralException2& e) 
						{
							LOG_ERROR(MAIN_LOG, "'svc_session_server_start' error: {} {}\n", e.err_code(), e.err_str());
						}

					}
					catch (...)
					{
						LOG_ERROR(MAIN_LOG, "on_svc_cli exception!!");
						byte_buffer bb;
						bb << (int)99 << string("unknown error!");
						chan->send_msg(bb.data_ptr(), bb.data_size());	
					}
				}).detach();
			}
			catch (GeneralException2& e) 
			{
				LOG_ERROR(MAIN_LOG, "'svc_session_server_start' error: {} {}\n", e.err_code(), e.err_str());
			}
		}
	}).detach();

}

int RegisterWsConnection(void* hdl, std::shared_ptr<FrameWriter>& fw, const std::string& init_msg)
{
#if 0
	shared_ptr<SvcVideoSession> sess;
	{
		reader_writer_lock::scoped_lock_read lck(g_rwl);
		auto it = g_vsmap.find(init_msg);
		if (it == g_vsmap.end())
		{
			return -1;
		}
		sess = it->second;
	}

	sess->regist_ws(hdl, fw);
#endif
	return 0;
}

