//#include "spdlog_helper.h"
#include "Util.h"
//#include "global_alg_conn.h"
#include "bath_utils.h"
//#include "global_header.h"
#include "rmmt_shm_image.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "rapidjson/Rjson.hpp"
#include "parser_dtinfo.h"
#include "Base64.h"
#include "byte_buffer.h"
#include "StreamCmdApis.h"
#include "activemq_producer.h"
#include "download_image.h"
#include "create_uuid.h"
#include "../include/hiredis/hiredis.h"
#include "../include/hiredis/async.h"
#include "http_his_client.h"
#include "video_info_dict.h"
#include "kafka_client.h"


	bp_algopkg_t::bp_algopkg_t(const char* name, const char* url)
	: handle_name(name)
	, rmmt_url(url)
	, is_conn(false)
	, pIpc(nullptr)
	, cfg("")
	, is_init(false)
{

}

extern std::vector<int> s_rediserr_list;
extern std::vector<string> ise_dbname;
extern IseApi  pIseApi;
extern std::string follow_servers;
extern std::string follow_topic;
extern KafkaClient pFowst;//äººè¸
extern int g_lefttopx;
extern int g_lefttopy;
extern int g_rightbottomx;
extern int g_rightbottomy;
extern int g_sim; 
extern int g_fcthdValue;
extern std::string tokenStr;//token å­æ®µ
extern std::string appId;
extern std::string appSecret;
extern std::string cloudwalkIp;
extern int cloudwalkPort;
extern std::string cloudwalkServer;
extern std::string clusterName;
unordered_map<string, string> bp_algo_url_t;
extern std::string alarm_broker;
extern std::string alarm_username;
extern std::string alarm_password;
extern std::string alarm_topic;
extern AmqClient pAlCt;//ï¿½ï¿½ï¿½ï¿½mq
extern bool bUseRedis;
extern std::string redisIp;
extern int redisdbNo;
extern int redisPort;
extern std::string redisUserName;
extern std::string redisPassWord;
extern timeval g_RedisTimeOut;
extern bool bConnect2Redis;
extern bool bConnectWsRedis; 
//extern redisAsyncContext* g_RedisConnector;

extern redisContext* g_RedisWsbzConnector;
extern atomic<int> g_globalCtlNum;//¿¿¿¿¿¿
extern redisContext* g_RedisSyncConnector;
//extern redisContext* g_RedisSync2Connect;
extern tbb::reader_writer_lock g_Redisrwl;


extern std::string active_broker;
extern std::string active_username;
extern std::string active_password;
extern std::string active_topic;
extern AmqClient pAcCt;//å¸æ§mq
extern std::string facl_broker;
extern std::string facl_broker1;
extern std::string facl_username;
extern std::string facl_password;
extern std::string facl_topic;
extern AmqClient pFaCt;//布控告警mq
extern AmqClient pFaCt1;//布控告警mq
//unordered_map<string, shared_ptr<bp_vec_algpkg>> bp_algo_t;
//
extern std::string mndIp;
extern int mndPort;
std::string translateDeviceInfo(const std::string& devinfo)
{
	auto pos = devinfo.find_first_of('|');//非精准追踪
	if(pos == string::npos)
		return string("");
	std::string reqid = devinfo.substr(0, pos);//
	return reqid;
}
///**
//*/
void add_algopkg(const char* name, const char* rmmt_url)
{
	bp_algo_url_t.insert(make_pair(name,rmmt_url));
}
//

int  GetMinSize(int x, int y, int z)
{	
	return x < y ? (x < z ? x : z) : (y < z ? y : z);
}

void CordinateConverse(cv::Mat& pMat,   Cordinate *input, Cordinate &output)
{
	
	//¿ª¹Ø¿ØÖÆÊÇ·ñ×ø±ê×ª»»
	{
		 //°´±ÈÀýÏòÍâÀ©Õ¹
	  
		 int wLeft = input->left_top_x;          //Ä¿±ê×ó²à
		 int wRight = wLeft + input->width;      //Ä¿±êÓÒ²à
		 int wTop = input->left_top_y;           //Ä¿±ê¶¥²¿
		 int wBottom = wTop + input->height;     //Ä¿±êµ×²¿
	  
		 int big_img_height =  pMat.rows;
		 int big_img_width = pMat.cols;
	  
		int nObjectW = input->width;
		int nObjectH = input->height;
	  
		int nLeftMargin = 0;    //Ä¿±ê×ó±ß¾à
		int nRightMargin = 0;   //Ä¿±êÓÒ±ß¾à
		int nTopMargin = 0;     //Ä¿±êÉÏ±ß¾à
		int nBottomMargin = 0;  //Ä¿±êÏÂ±ß¾à
	  
	  
		//Ïò×óÀ©min(w/3,100),Èç¹û¸Ã·½Ïò×î´ó¿ÉÒÆ¶¯¾àÀëÐ¡ÓÚmin£¬Ôò°´ÕÕ×î´ó¿ÉÒÆ¶¯¾àÀë±£ÁôÀ©Õ¹
		nLeftMargin = wLeft - GetMinSize(nObjectW / 3, 100, wLeft);
		if (nLeftMargin < 0)
			nLeftMargin = 0;
 
		//ÏòÓÒÀ©min(w/3,100),Èç¹û¸Ã·½Ïò×î´ó¿ÉÒÆ¶¯¾àÀëÐ¡ÓÚmin£¬Ôò°´ÕÕ×î´ó¿ÉÒÆ¶¯¾àÀë±£ÁôÀ©Õ¹
		nRightMargin = wRight +  GetMinSize(nObjectW / 3, 100, (big_img_width - 5 - wRight));
		//ÏòÉÏÀ©min(H/5, 50),Èç¹û¸Ã·½Ïò×î´ó¿ÉÒÆ¶¯¾àÀëÐ¡ÓÚmin£¬Ôò°´ÕÕ×î´ó¿ÉÒÆ¶¯¾àÀë±£ÁôÀ©Õ¹
		nTopMargin = wTop - GetMinSize(nObjectH / 3, 100, wTop);
		if (nTopMargin < 0)
			nTopMargin = 0;
		  
		//ÏòÏÂÀ©min(H/5, 50),Èç¹û¸Ã·½Ïò×î´ó¿ÉÒÆ¶¯¾àÀëÐ¡ÓÚmin£¬Ôò°´ÕÕ×î´ó¿ÉÒÆ¶¯¾àÀë±£ÁôÀ©Õ¹
		nBottomMargin = wBottom + GetMinSize(nObjectH / 3, 100, (big_img_height - 5 - wBottom));
		  
	  
		output.height = nBottomMargin - nTopMargin;
		output.left_top_x = nLeftMargin;
		output.left_top_y = nTopMargin;
		output.width = nRightMargin - nLeftMargin;
		 
	}

}

AlgoConnector::AlgoConnector()
{
	m_brelate = false;
	//m_facerl = nullptr;
	m_relateSkip = 0;
	m_tp = m_stp;
	//if(m_stp)
	//m_stp->init_pool(2);

	if (!m_tp.expired())
	{
		m_tp.lock()->init_pool(2);        // ï¿½ï¿½ï¿½ï¿½ï¿½this is class CA!
	}
	_quit_flag = false;
        final_quit = false;
	relate_flag = true;
	m_control = {};
}

AlgoConnector::~AlgoConnector()
{
    m_algo_pkg->pIpc->close(true);
}

bool AlgoConnector::Init_Conn(const char * name)
{
	Document dc;
	Rjson::Parse(dc, name);
	string algname;
	if (!Rjson::GetStringV(algname, "algtype", &dc))
		return false;
    int relate_t;
    if (!Rjson::GetIntV(relate_t, "relflg", &dc))
		return false;
#if 1
	if(relate_t == 0) relate_flag = false;
	if(relate_t == 1) relate_flag = true;
	auto it = bp_algo_url_t.find(algname);
#else
	auto it = bp_algo_url_t.find(name);
#endif
	if (it == bp_algo_url_t.end())
	{
		return false;
	}
	//return it->second.get();
	string alg_url = it->second;
	m_algo_pkg = make_shared<bp_algopkg_t>(name, alg_url.data());
	if (m_algo_pkg == nullptr)
	{
		LOG_INFO(MAIN_LOG, "m_algo_pkg is null!!");
		return false;
	}
	if (m_algo_pkg->is_conn)
	{
		return false;
	}
	try
	{
		LOG_INFO(MAIN_LOG, "begin create a new ipc!!!!");
		std::shared_ptr<rmmt::MsgChannel> chan = rmmt::MsgChannel::connect_server(m_algo_pkg->rmmt_url);
		if (!m_algo_pkg->is_init)
		{
			m_algo_pkg->pIpc = std::shared_ptr<rmmt::IPCv2>(new rmmt::IPCv2(chan, [&](const std::function<void()>& task, uint32_t nt) {
				//g_tp.push_task(task);
				try
				{
				  if(m_stp)
				  {//m_tp->push_task(task);
					if (!m_tp.expired())
				    {
				  	  m_tp.lock()->push_task(task);        // ï¿½ï¿½ï¿½ï¿½ï¿½this is class CA!
				    }
				  }
				}
				catch(GeneralException2& e)
				{
				   LOG_INFO(MAIN_LOG, "FTaskTool3 push a false {}---{}!!!!", e.err_code(), e.err_str());
				}
			}));
		}
		else
		{
			m_algo_pkg->pIpc->normal_close();
			m_algo_pkg->pIpc->wait_restart();
			m_algo_pkg->pIpc->re_init();
			m_algo_pkg->pIpc->new_chan(chan);
		}
		//LOG_INFO(MAIN_LOG, "end create a new ipc!!!!---{}", (int)m_algo_pkg->pIpc->m_fQuit);
		m_algo_pkg->pIpc->start();
		m_algo_pkg->is_conn = true;
		m_algo_pkg->is_init = true;
		LOG_INFO(MAIN_LOG, "wake stop,just do ipc!!");
		return true;
	}
	catch (GeneralException2& e)
	{
		LOG_INFO(MAIN_LOG, "start a new ipc catch a false {}---{}!!!!", e.err_code(), e.err_str());
		return false;
	}
	catch (...)
	{
		return false;
	}
}

bool AlgoConnector::Start_Task(const char* param, int& errorCode)
{
	if (m_algo_pkg == nullptr)
	{
		errorCode = -2;
		return false;
	}
	m_sess_t = make_shared<bp_alogpkg_sess_t>();
	bool hasError = false;
	rmmt::ShmVecType vmn;
	rmmt::ShmVecType vmn_resp;

	try
	{
		if (!m_algo_pkg->is_init)
		{
			errorCode = -2;
			return false;
		}
		byte_buffer bb;
		if (param != nullptr)
		{
			if (m_algo_pkg->cfg == "")
			{
				m_algo_pkg->cfg = string(param);
			}
		}
		bb << (int)75 << m_algo_pkg->cfg;

		std::string resp_data = "";


		m_algo_pkg->pIpc->do_IPC(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp);

		CharSeqReader chrd(resp_data.data(), resp_data.size());
		int code;
		string errmsg;
		chrd >> code >> errmsg >> m_sess_t->ap_sess;
		m_sess_t->ap_hdl = m_algo_pkg.get();
		//sess->errcode = code;
		if (code == -100)
		{
			errorCode = -100;
		}
		if (code != 0)
		{
			throw GeneralException2(code, errmsg);
		}
	}
	catch (GeneralException2 &e)
	{
		hasError = true;
		errorCode = e.err_code();
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new GeneralException2 error! {},{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_algo_pkg->is_conn = false;
		}
		//ap_hdl->is_conn = false;
	}
	catch (rmmt::IPCException &e)
	{
		hasError = true;
		errorCode = e.ret_code;
		//ap_hdl->is_conn = false;
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new IPCException error! {},{}", e.ret_code, e.err_msg);
	}
	catch (std::exception& e)
	{
		hasError = true;
		//ap_hdl->is_conn = false;
		errorCode = -99;
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new stdException error!{}", e.what());
	}
	catch (...)
	{
		hasError = true;
		//ap_hdl->is_conn = false;
		errorCode = -99;
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new error!");
	}
	if (hasError)
	{
		//delete sess;
		//sess = nullptr;
		//ï¿½ï¿½Îªtrue,ï¿½Â´Î¿ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		//return sess;
		return false;
	}
	//sess->is_correct = true;
	vmn.clear();
	vmn_resp.clear();
	return true;
}

void AlgoConnector::End_task(bool wait_flag)
{
	if (m_sess_t == nullptr)
	{
		return;
	}
	rmmt::ShmVecType vmn;
	rmmt::ShmVecType vmn_resp;
	try
	{

		byte_buffer bb;
#if 0
                if(m_sess_t->ap_sess == nullptr)
		bb << (int)78 << string("");
                else
#endif
		bb << (int)76 << m_sess_t->ap_sess;
		std::string resp_data;

		//LOG_INFO(MAIN_LOG, "wait stop,just do ipc!!");
		if(!wait_flag)
		m_sess_t->ap_hdl->pIpc->do_IPC_NW(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp,1,"","{}");
		else
		{
		    m_sess_t->ap_hdl->pIpc->do_IPC(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp);
		    CharSeqReader chrd(resp_data.data(), resp_data.size());
		    int code;
		    string errmsg;
		    chrd >> code >> errmsg;
		    if (code != 76)
		    {
			    LOG_INFO(MAIN_LOG,"~~~~ap close code:{},msg:{}", code, errmsg.data());
			    throw GeneralException2(code, errmsg);
		    }
		}
	}
	catch (GeneralException2& e)
	{
		LOG_INFO(MAIN_LOG,"algopkg_release_session do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		LOG_ERROR(MAIN_LOG, "algopkg_release_session do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_sess_t->ap_hdl->is_conn = false;
		}
		//		sess->ap_hdl->is_conn = false;
		//
	}
	catch (rmmt::IPCException &e)
	{
		LOG_INFO(MAIN_LOG,"algopkg_release_session do_IPC IPCException: {}/{}", e.ret_code, e.err_msg.data());
		LOG_ERROR(MAIN_LOG, "algopkg_release_session do_IPC IPCException: {}/{}", e.ret_code, e.err_msg);
		//sess->ap_hdl->is_conn = false;
	}
	catch (std::exception& e)
	{
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new stdException error!{}", e.what());
		//sess->ap_hdl->is_conn = false;
	}
	catch (...)
	{
		LOG_INFO(MAIN_LOG,"algopkg_release_session do_IPC exception\n");
		//sess->ap_hdl->is_conn = false;
	}
	vmn.clear();
	vmn_resp.clear();
}

ap_output_t AlgoConnector::Alg_Sess_Process(inputinfo_t * input)
{
	ap_output_t data = {};
	data.errc = 0;
	//data.errmsg = "success";
	if (input == nullptr)
	{
		return data;
	}
	//////////////////////////////////////////////////////////////////////////
	bool checky = false;
	Document dc;
	Rjson::Parse(dc, input->param1);
	int type = Rjson::GetInt("type", &dc);
	if (type == 0)
	{
		checky = true;
	}
	rmmt::ShmVecType vmn;
	rmmt::ShmVecType vmn_resp;
	//////////////////////////////////////////////////////////////////////////
	try
	{
		byte_buffer bb;
		if (checky)
		{
			bb << (int)77 << m_sess_t->ap_sess << (int64_t)0;
		}
		else
		{
			if (input->n_img > 0)
			{
				std::vector<imageinfo_t> vImgDatas(*input->p_imgs, *input->p_imgs + input->n_img);
				for (auto& it : vImgDatas)
				{
					rmmt::ShmImage simg;
					simg.clone(it.p_img);

					if (it.pts < 0)
					{
						simg.getHdr()->pts = (uint64_t)0;
					}
					else
					{
						simg.getHdr()->pts = it.pts;
					}//zj 20190815 ï¿½ï¿½ï¿½ï¿½ï¿½Ð·ï¿½ï¿½ï¿½×ªï¿½Þ·ï¿½ï¿½Å²ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
					vmn.push_back(simg.get_shm());
					bb << (int)77 << m_sess_t->ap_sess << it.pts;
				}
			}
			else
			{
				bb << (int)77 << m_sess_t->ap_sess << (int64_t)0;
			}
		}
		std::string resp_data;

		//LOG_INFO(MAIN_LOG, "wait stop,just do ipc!!");
		m_sess_t->ap_hdl->pIpc->do_IPC(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp);
		//vmn.clear();
		data.text_len = resp_data.size();
		data.text = (char*)malloc(data.text_len + 1);
		strncpy(data.text, resp_data.data(), resp_data.size());
		data.text[data.text_len] = '\0';
		//FormatLogA("do ipc 77 binary len is %d", vmn_resp.size());
		if (vmn_resp.size() > 0)
		{
			data.binary_len = vmn_resp[0]->get_size();
			data.binary = (char*)malloc(data.binary_len + 1);
			memcpy(data.binary, (char*)vmn_resp[0]->get_ptr(), data.binary_len);
			data.binary[data.binary_len] = '\0';
		}
		//vmn_resp.clear();
	}
	catch (GeneralException2& e)
	{
		//LOG_INFO(MAIN_LOG,"algopkg_alg_process do_IPC GeneralException2 : %d/%s\n", e.err_code(), e.err_str());
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_sess_t->ap_hdl->is_conn = false;
		}
		data.errc = e.err_code();
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	catch (rmmt::IPCException &e)
	{
		//LOG_INFO(MAIN_LOG,"algopkg_alg_process do_IPC IPCException: %d/%s\n", e.ret_code, e.err_msg.data());
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC IPCException: {}/{}", e.ret_code, e.err_msg);
		//sess->ap_hdl->is_conn = false;
		data.errc = -19;
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	catch (std::exception& e)
	{
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new stdException error!{}", e.what());
		//sess->ap_hdl->is_conn = false;
		data.errc = -18;
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	catch (...)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC exception\n");
		//sess->ap_hdl->is_conn = false;
		data.errc = -20;
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	vmn.clear();
	vmn_resp.clear();
	return data;
	
}

ap_output_t AlgoConnector::Alg_Process(inputinfo_t * input)
{
	ap_output_t data = {};
	data.errc = 0;
	if (input == nullptr)
	{
		data.errc = -1;
		return data;
	}
	byte_buffer bb;
	bb << 78 << string(input->param1);
	rmmt::ShmVecType vmn;
	try
	{
		// 		//ï¿½ï¿½ï¿½ï¿½ï¿½ã·¨Êµï¿½ï¿½
		// 		conn_algo_pkt(ap_hdl);
		//ï¿½ï¿½ï¿½ï¿½ï¿½ã·¨Êµï¿½ï¿½
		if (!m_algo_pkg->is_init)
		//if (!conn_algo_pkt(ap_hdl))
		{
			m_algo_pkg->is_conn = false;
			data.errc = -1;
			return data;
		}
		if (input->n_img > 0)
		{
			std::vector<imageinfo_t> vImgDatas(*input->p_imgs, *input->p_imgs + input->n_img);
			for (auto& it : vImgDatas)
			{
				rmmt::ShmImage simg;
				simg.clone(it.p_img);
				vmn.push_back(simg.get_shm());
			}
		}
	}
	catch (GeneralException2 &e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process clone failed: {}/{}", e.err_code(), e.err_str());
		//ap_hdl->is_conn = false;
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_algo_pkg->is_conn = false;
		}
		vmn.clear();
		//ap_hdl->is_conn = false;
		data.errc = -19;
		return data;
	}
	std::string resp_data;
	rmmt::ShmVecType vmn_resp;
	try
	{
		//LOG_INFO(MAIN_LOG, "wait stop,just do ipc!!");
		m_algo_pkg->pIpc->do_IPC(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp);

	}
	catch (GeneralException2& e)
	{
		//LOG_INFO(MAIN_LOG,"algopkg_alg_process do_IPC GeneralException2 : %d/%s\n", e.err_code(), e.err_str());
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == -19)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_algo_pkg->is_conn = false;
		}
		//
		data.errc = e.err_code();
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	catch (rmmt::IPCException &e)
	{
		//LOG_INFO(MAIN_LOG,"algopkg_alg_process do_IPC IPCException: %d/%s\n", e.ret_code, e.err_msg.data());
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC IPCException: {}/{}", e.ret_code, e.err_msg);
		//ap_hdl->is_conn = false;
		data.errc = -19;
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	catch (std::exception& e)
	{
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new stdException error!{}", e.what());
		//ap_hdl->is_conn = false;
		data.errc = -19;
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	catch (...)
	{
		LOG_INFO(MAIN_LOG,"algopkg_alg_process do_IPC exception\n");
		//ap_hdl->is_conn = false;
		data.errc = -19;
		vmn.clear();
		vmn_resp.clear();
		return data;
	}

	data.text_len = resp_data.size();
	data.text = (char*)malloc(data.text_len + 1);
	strncpy(data.text, resp_data.data(), resp_data.size());
	data.text[data.text_len] = '\0';
	if (vmn_resp.size() > 0)
	{
		data.binary_len = vmn_resp[0]->get_size();
		data.binary = (char*)malloc(data.binary_len + 1);
		memcpy(data.binary, (char*)vmn_resp[0]->get_ptr(), data.binary_len);
		data.binary[data.binary_len] = '\0';
	}
	vmn.clear();
	vmn_resp.clear();
	return data;
}

ap_output_t AlgoConnector::algo_sess_proc_recv()
{
	ap_output_t data = {};
	data.errc = 0;

	rmmt::algo_rsp_t algrsp = make_shared<rmmt::cache_algo_frm>();
	while(!m_algo_pkg->pIpc->m_decRspQue[0].try_pop(algrsp))
	{
        if(_quit_flag)
	    {
        	if (final_quit /*&& (m_algo_pkg->pIpc->check_task_end())*/)
		    {
				_send_null_frame();
				data.errc = -6;
				return data;
			}
        	else
        	{
        	    std::this_thread::sleep_for(std::chrono::milliseconds(40)); 
        	    //_send_null_frame();
        	}
         }
         else
         {
               std::this_thread::sleep_for(std::chrono::milliseconds(40));
         }
	}

    if(_quit_flag) _send_null_frame();
	
	if (algrsp->ipc_e.ret_code != 0 || !algrsp->ipc_e.err_msg.empty())
	{
		data.errc = algrsp->ipc_e.ret_code;
		return data;
	}

#if 0
	if (data.image_info == nullptr)
	{
		data.image_info = new imageinfo_t;
		memset(data.image_info, 0, sizeof(imageinfo_t));//
		data.image_info->_verflag = 0x1;//
	}
#endif

	data.text_len = algrsp->algo_rspdata.size();
	data.text = (char*)malloc(data.text_len + 1);
	strncpy(data.text, algrsp->algo_rspdata.data(), algrsp->algo_rspdata.size());
	data.text[data.text_len] = '\0';

	//LOG_INFO(MAIN_LOG,"check the rspvec size is %d", (int)algrsp->algo_rspvec.size());
	if (algrsp->algo_rspvec.size() > 0)
	{
		data.binary_len = algrsp->algo_rspvec[0]->get_size();
		data.binary = (char*)malloc(data.binary_len + 1);
		memcpy(data.binary, (char*)algrsp->algo_rspvec[0]->get_ptr(), data.binary_len);
		data.binary[data.binary_len] = '\0';
	}
#if 0
	if(algrsp->algo_srcvec.size() > 0)
	{
#if 0
		rmmt::ShmImage simg(algrsp->algo_srcvec[0]);
		IplImage img = simg.toCvImage();
		data.image_info->p_img = cvCreateImage(cvGetSize(&img), img.depth, img.nChannels);
		memcpy(data.image_info->p_img->imageData, img.imageData, img.imageSize);
		data.image_info->pts = simg.getHdr()->pts;
#endif
		if(m_brelate)
		{    
			if(m_relateSkip % 3 == 0)
			{
				m_relateSkip = 0;
		        rmmt::ShmImage simg(algrsp->algo_srcvec[0]);
				if(check_do_relate(algrsp->algo_rspdata,simg.getHdr()->width,simg.getHdr()->height))
				{
					//if(m_facerl != nullptr)
					//{
					//	using namespace Rjson;
					//	Rjson::Doc doc({ { "cmd", "AP_FACE_FEATURE_CMD_DETECT" } });
					//	auto paramObj = doc.lv().add_new_obj("param");
					//	paramObj.add(string("flag"), FVal(1));
					//	string param = doc.dumps();
					//	//INFO("--->face detect param is {}", param.data());

                    //   m_facerl->algo._send_relate_frame(algrsp->algo_srcvec,algrsp->algo_rspdata,param);
					//}
				}
			}
			m_relateSkip++;
		}
	}
#endif

	return data;

}

void AlgoConnector::_send_algo_frame(rmmt::ShmImage& image,int64_t pts)
{

	rmmt::ShmVecType vmn;
	rmmt::ShmVecType vmn_resp;
    try
	{
		byte_buffer bb;
		bb << (int)77 << m_sess_t->ap_sess << pts;
		vmn.push_back(image.get_shm());
		std::string resp_data;
		m_sess_t->ap_hdl->pIpc->do_IPC_NW(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp,1,"","{}");
	}
	catch(GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_sess_t->ap_hdl->is_conn = false;
		}
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (...)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC exception\n");
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	vmn.clear();
	vmn_resp.clear();
	return;
}


ap_output_t AlgoConnector::algo_relate_proc_recv(int do_type,const string& feat)
{
	ap_output_t data = {};//
	data.errc = 0;

	rmmt::algo_rsp_t  relatersp = make_shared<rmmt::cache_algo_frm>();
        m_algo_pkg->pIpc->m_decRspQue[do_type - 1].pop(relatersp);
	//while(!m_algo_pkg->pIpc->m_decRspQue[do_type - 1].try_pop(relatersp))
	//{
	//	if(_quit_flag)
	//	{
	//		_send_null_frame();
	//		data.errc = -6;
	//		return data;
	//	}
	//	else
	//	{
	//		data.errc = -1;
	//		return data;
	//	}
	//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//}
	if(_quit_flag) 
	{ 
		_send_null_frame();
		data.errc = -2;
	}

	if (relatersp->ipc_e.ret_code != 0 || !relatersp->ipc_e.err_msg.empty())
	{
		LOG_ERROR(ANAL_LOG, "ipc frame error!!");
		data.errc = relatersp->ipc_e.ret_code;
		return data;
	}
#if 0
	if (data.image_info == nullptr)
	{
		data.image_info = new imageinfo_t;
		memset(data.image_info, 0, sizeof(imageinfo_t));//
		data.image_info->_verflag = 0x1;//
	}
#endif

	switch(do_type)
	{
		case 1:
		{
			LOG_INFO(ANAL_LOG, "recv a face detect cmd,do it!");
			if (relatersp->algo_rspdata.empty()) 
			{
				LOG_ERROR(ANAL_LOG, "algo_rspdata.empty() true");
				//return data;
				break;
			}
			Document dc_f;
			Rjson::Parse(dc_f,relatersp->algo_rspdata);
			int errCode = Rjson::GetInt("code", &dc_f);
			if (errCode != 0) 
                        {
			    LOG_ERROR(ANAL_LOG, "boyun errcode is not 0");
                            return data;
                        }
			rmmt::ShmImage simg(relatersp->algo_srcvec[0]);
			cv::Mat ipmg = simg.toCvMat();
			std::vector<uchar> bak_encode;
			cv::imencode(".jpg", ipmg, bak_encode);
			string bkg_img = string(bak_encode.begin(), bak_encode.end());
			const Value* obj_img = Rjson::GetObject("results", 0, &dc_f);
			const Value* obj_infos = Rjson::GetArray("obj_infos", obj_img);
			Document dc_t;
			Rjson::Parse(dc_t,relatersp->bind_param);
                        if(dc_t.HasMember("RLBK"))
                        {
                           string _bind_param = Rjson::GetString("RLBK",&dc_t);
                           if( obj_infos->Size() > 0)
                           {
                              const Value* obj = Rjson::GetObject(0, obj_infos);
                                string dtInfo = Rjson::ToString(obj);
                              _send_relate_frame(relatersp->algo_srcvec,dtInfo,"","",_bind_param,2);  
                           }   
 			   return data;	
 			}
                        if(dc_t.HasMember("WSBZ")) 
		        {
                              int wsflg =  Rjson::GetInt("WSBZ",&dc_t);
			      LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!check in wsbz--person num {}---wsflg {}",obj_infos->Size(),wsflg);
                              if(obj_infos->Size() > 1 || wsflg == 2 || wsflg == 3)
                              {
				int64_t pts = Rjson::GetInt64("captureTime",&dc_t);
                                string captureId = Rjson::GetString("captureId",&dc_t);
                                string ExtField = Rjson::GetString("captureExtField1",&dc_t);
                                Document dt;
                                Rjson::Parse(dt,ExtField);
                                string devId = Rjson::GetString("realDeviceId",&dt);
                         	string recgTemp = relatersp->algo_rspdata;
				LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!thread start bind param pts {} deviceId {}",pts,devId);
                             	std::thread([&,recgTemp,bkg_img,pts,devId,wsflg,captureId](){
                             	   try{
                                       if(wsflg == 2 || wsflg == 3)
                             	       do_ws_response1(recgTemp,bkg_img,devId,pts,captureId,wsflg);
                                       else
                             	       do_ws_response(recgTemp,bkg_img,devId,pts,captureId);
                             	   }
	  		     	   catch (GeneralException2& e)
	  		     	   {
	  		     	      LOG_ERROR(ANAL_LOG, "on_do_weisui exception: {},{}", e.err_code(), e.err_str());
	  		     	   }
                             	   catch(...)
	  		     	      {LOG_ERROR(ANAL_LOG, "on_do_weisui unhandled false");}
                             	     
                             	 }).detach();
                              }
                              dc_t.RemoveMember("WSBZ");
                        }
			LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!check in face num {}",obj_infos->Size());
			string bkg64data = base64_encode(reinterpret_cast<const unsigned char*>(bkg_img.data()), bkg_img.size());
			for (int k = 0; k < obj_infos->Size();k++)
			{
				const Value* obj = Rjson::GetObject(k, obj_infos);
				string dtInfo = Rjson::ToString(obj);
                                
				/////
				Document dc_a;
				dc_a.CopyFrom(dc_t, dc_t.GetAllocator());
				
				string uuid = create_uuid(false);
				int rect[4];
				for (int m = 0; m < 4; ++m)
				{
				     rect[m] = (int)Rjson::GetFloat("rect", m, obj);
				}
        
        //add by jiang 2020-07-22
       	//°´±ÈÀý·Å´óÈËÁ³Í¼
				Cordinate input, output;
				input.left_top_x = rect[0];
				input.left_top_y = rect[1];
				input.width = rect[2];
				input.height = rect[3];
				
				CordinateConverse(ipmg, &input, output);
				
				rect[0] = output.left_top_x;
				rect[1] = output.left_top_y;
				rect[2] = output.width;
				rect[3] = output.height;
				////////////////////////////////////////

				cv::Range r1(rect[0], rect[0] + rect[2]);
				cv::Range r2(rect[1], rect[1] + rect[3]);
				cv::Mat roi = cv::Mat(ipmg, r2, r1).clone();
#if 0
                                {
					string sid = create_uuid(false);
					string path = fmt::format("/home/test_zj/thank666/{}_smallcap.jpg",sid);
					if(roi.data)
					{
						cv::imwrite(path.data(),roi); 
					}
                                }
#endif
				std::vector<uchar> orig_encode;
				cv::imencode(".jpg", roi, orig_encode);
                                roi.release();
				string orig_img = string(orig_encode.begin(), orig_encode.end());
                                string obj64data = base64_encode(reinterpret_cast<const unsigned char*>(orig_img.data()), orig_img.size());
				//string orig_img_path;
				//int reUploadOrigimg = ftp_upload_image(orig_img, orig_img_path, SMALLPUSH);
				//if (reUploadOrigimg == -1)
				//	return data;
				//orig_img_path = orig_img_path.replace(0, 3, "http");
				//string bak_img_path;
				//int reUploadBkgimg = ftp_upload_image(bkg_img, bak_img_path, BIGPUSH);
				//if (reUploadBkgimg == -1)
				//	return data;
				//bak_img_path = bak_img_path.replace(0, 3, "http");
				dc_a.AddMember("captureImg",obj64data,dc_a.GetAllocator());      
				dc_a.AddMember("panoramaImg",bkg64data,dc_a.GetAllocator());      
				dc_a.AddMember("panoramaId",uuid,dc_a.GetAllocator());      
				dc_a.AddMember("x",rect[0],dc_a.GetAllocator());      
				dc_a.AddMember("y",rect[1],dc_a.GetAllocator());      
				dc_a.AddMember("w",rect[2],dc_a.GetAllocator());      
				dc_a.AddMember("h",rect[3],dc_a.GetAllocator());      
				string _bind_param = Rjson::ToString(dc_a);
                                {
					//Document temdoc;
					//Rjson::Parse(temdoc,_bind_param);
                                        //temdoc.RemoveMember("captureImg");
                                        //temdoc.RemoveMember("panoramaImg");
					//string param = Rjson::ToString(temdoc);
					//LOG_INFO(ANAL_LOG,"post param to ocean!!: {}",param);
                                }
				std::string reply;
				std::string pathUrl = "http://" + cloudwalkServer + "/ocean/data/entry/camera/capture";
				bool success = HttpClientPostCloudWalk(pathUrl.c_str(), _bind_param.c_str(), tokenStr, reply);
				LOG_INFO(ANAL_LOG,"post img to ocean!! reply : {}",reply);
                                string ExtFid = Rjson::GetString("captureExtField1",&dc_t);
				_send_relate_frame(relatersp->algo_srcvec,dtInfo,"","",ExtFid,2);
			}
		}break;
		case 2:
		{
			LOG_INFO(ANAL_LOG, "recv a face feature cmd,do it!");
			string rsp;
			CharSeqReader chrd((char*)relatersp->algo_rspvec[0]->get_ptr(),relatersp->algo_rspvec[0]->get_size());
			chrd >> rsp;
			LOG_INFO(MAIN_LOG, "get target feature size {},bindparam {}",rsp.size(),relatersp->bind_param);
			Document dw;
			Rjson::Parse(dw,relatersp->bind_param);
			//int code;
			//Rjson::GetIntV(code, "code", &dw);

			//LOG_INFO(MAIN_LOG,"cmd2222222222222222222222222  recv code--{}!!!!!",code);
			bool isGlobal = false;
			if(dw.HasMember("iconId"))
			{
                                string iconid = Rjson::GetString("iconId",&dw);
                                string taskid = Rjson::GetString("taskId",&dw);
                                int    dbNo = Rjson::GetInt("dbNo",&dw);
                                if(dbNo == 1) dbNo = 5;
                                if(dbNo == 2) dbNo = 1;
				const Value* deviceinfo = Rjson::GetArray("deviceId", &dw);
                                if(deviceinfo->Size() <= 0)
                                {
									isGlobal = true;
									string deviceid = string("global-control");
									IdxPac re_tid = {};
									const char* wp;
									wp = "DeviceId,TaskId,IconId";
									string plv;
									format_string(plv,"'%s','%s','%s'",deviceid.data(),taskid.data(),iconid.data());
									re_tid = pIseApi.push_image(ise_dbname[dbNo], 4, rsp, wp, plv,false);
									if (re_tid.idx == 0)
									{
										LOG_ERROR(MAIN_LOG,"{}", "push to ise_db failed!");
									}
									else
									{
										LOG_INFO(MAIN_LOG,"push image success ---re_tid:{}", re_tid.idx);
									}
                                }
								for (int k = 0; k < deviceinfo->Size();k++)
								{
									string deviceid = Rjson::GetString(k, deviceinfo);
									IdxPac re_tid = {};
									const char* wp;
									wp = "DeviceId,TaskId,IconId";
									string plv;
									format_string(plv,"'%s','%s','%s'",deviceid.data(),taskid.data(),iconid.data());
									re_tid = pIseApi.push_image(ise_dbname[dbNo], 4, rsp, wp, plv,false);
									if (re_tid.idx == 0)
									{
										LOG_ERROR(MAIN_LOG,"{}", "push to ise_db failed!");
									}
									else
									{
										LOG_INFO(MAIN_LOG,"push image success ---re_tid:{}", re_tid.idx);
									}
								} 

				return data;
			}
                        string srcdata = relatersp->algo_srcdata;
			rmmt::ShmImage simg(relatersp->algo_srcvec[0]);
                        int64_t pts = simg.getHdr()->pts;
			cv::Mat ipmg = simg.toCvMat();
			std::vector<uchar> bak_encode;
			cv::imencode(".jpg", ipmg, bak_encode);
			string bkg_img = string(bak_encode.begin(), bak_encode.end());
                        string relateparam = relatersp->bind_param;  
                        string feat = rsp;
			std::thread([&,srcdata,bkg_img,relateparam,feat,pts](){
                            try{
                            do_face_control_alarm(srcdata,bkg_img,relateparam,feat,pts,1);      
                            }
                            catch(...)
                            {}                
                        }).detach();
			std::thread([&,srcdata,bkg_img,relateparam,feat,pts](){
                            try{
                            do_face_control_alarm(srcdata,bkg_img,relateparam,feat,pts,5);                      
                            }
                            catch(...)
                            {}
                        }).detach();
                         
		}break;
		case 3:
		{
			try{
				Document dc;
				Rjson::Parse(dc,relatersp->algo_rspdata);
				float comScore;
				Rjson::GetFloatV(comScore, "score", &dc);
				if (comScore > 0.70)
				{
                    if(relatersp->algo_srcvec.size() > 0)
					{
#if 0
						rmmt::ShmImage simg(relatersp->algo_srcvec[0]);
						cv::Mat ipmg = simg.toCvMat();
						std::vector<uchar> orig_encode;
						cv::imencode(".jpg", ipmg, orig_encode);
						string orig_img = string(orig_encode.begin(), orig_encode.end());
						string orig_img_path;
						int reUploadOrigimg = ftp_upload_image(orig_img, orig_img_path, SMALLPUSH);
						if (reUploadOrigimg == -1)
							return data;
						orig_img_path = orig_img_path.replace(0, 3, "http");
#endif
						Document dw;
						Rjson::Parse(dw,relatersp->bind_param);
						string imgurl,deviceid,uuid,snapTime,orig_img_path;
						Rjson::GetStringV(orig_img_path, "ImgUrl", &dw);
						Rjson::GetStringV(imgurl, "iconUrl", &dw);
						Rjson::GetStringV(deviceid, "deviceId", &dw);
                        Rjson::GetStringV(uuid, "UUID", &dw);
						Rjson::GetStringV(snapTime, "sanptime", &dw);
						Document dc_w = Rjson::rWriteDC();
						Rjson::rAdd(dc_w, "iconUrl", imgurl);
						Rjson::rAdd(dc_w, "deviceId",deviceid);
						Rjson::rAdd(dc_w, "ObjUrl",orig_img_path);
                        Rjson::rAdd(dc_w, "UUID",uuid);
						Rjson::rAdd(dc_w, "controlReason",m_control.controlReason);
						Rjson::rAdd(dc_w, "taskId",m_control.taskid);
						Rjson::rAdd(dc_w, "snapTime",snapTime);
						Rjson::rAdd(dc_w, "similarity",comScore);
						Rjson::rAdd(dc_w, "dataSource",m_control.dataSource);
						Rjson::rAdd(dc_w, "keyUserId",m_control.keyUserId);
						Rjson::rAdd(dc_w, "keyPlaceId",m_control.keyPlaceId);
						Rjson::rAdd(dc_w, "departmentId",m_control.departmentId);
						Rjson::rAdd(dc_w, "outputType",3);
						
                        string input_ = Rjson::ToString(dc_w);
						LOG_INFO(MAIN_LOG,"waooo!!begain to push the alarm warning MQ!!--{}",input_);
						bool bRet = pAlCt.activemq_produce(input_.data());
						if (!bRet)
						{
							bRet = pAlCt.activemq_producer_init(alarm_broker.data(), alarm_username.data(), alarm_password.data(), alarm_topic.data());
						} 
					}
				}
			}
			catch(GeneralException2& e)
			{
				LOG_INFO(MAIN_LOG, "get an error from face compare {}---{}",e.err_code(), e.err_str());
				return data;
			}
			catch(...)
			{

			}
#if 0
			data.text_len = relatersp->algo_rspdata.size();
			data.text = (char*)malloc(data.text_len + 1);
			strncpy(data.text, relatersp->algo_rspdata.data(), relatersp->algo_rspdata.size());
			data.text[data.text_len] = '\0';
			if(relatersp->algo_srcvec.size() > 0)
			{
				rmmt::ShmImage simg(relatersp->algo_srcvec[0]);
				IplImage img = simg.toCvImage();
				data.image_info->p_img = cvCreateImage(cvGetSize(&img), img.depth, img.nChannels);
				memcpy(data.image_info->p_img->imageData, img.imageData, img.imageSize);
				data.image_info->pts = simg.getHdr()->pts;
			}
#endif
	        return data;
		}break;
		default:
		break;
	}
	//LOG_INFO(MAIN_LOG,"check the rspvec size is %d", (int)relatersp->algo_rspvec.size());
#if 0
	if (relatersp->src_vec.size() > 0)
	{
#if 1
		rmmt::ShmImage simg(relatersp->src_vec[0]);
		IplImage img = simg.toCvImage();
		data.image_info->p_img = cvCreateImage(cvGetSize(&img), img.depth, img.nChannels);
		memcpy(data.image_info->p_img->imageData, img.imageData, img.imageSize);
		data.image_info->pts = simg.getHdr()->pts;
		dtrsp = relatersp->src_data;
#else
		data.binary_len = relatersp->src_vec[0]->get_size();
		data.binary = (char*)malloc(data.binary_len + 1);
		memcpy(data.binary, (char*)relatersp->src_vec[0]->get_ptr(), data.binary_len);
		data.binary[data.binary_len] = '\0';
		rmmt::ShmImage simg(relatersp->src_vec[0]);
		//pts = simg->getHdr()->pts;
#endif
	}

#endif
	return data;

}
void AlgoConnector::do_ws_response1(const string& param,const string& imgdata,const string& devid,int64_t pts,const string& captureId,int wsflg)
{
	Document dc_f;
	Rjson::Parse(dc_f,param);
        	
	cv::Mat img(cv::imdecode(cv::Mat(1, imgdata.size(),CV_8UC3,(unsigned char*)imgdata.data()),CV_LOAD_IMAGE_COLOR));
	if(!img.data) 
	{
		LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!img not valid! skip!");
	        return; 
	}
	//std::vector<int> pointsAreaV;
        //pointsAreaV.clear();
	const Value* obj_img = Rjson::GetObject("results", 0, &dc_f);
	const Value* obj_infos = Rjson::GetArray("obj_infos", obj_img);
	//for (int k = 0; k < obj_infos->Size(); ++k)
        //{ 
	//    const Value* obj = Rjson::GetObject(k, obj_infos);
	//    int area = (int)Rjson::GetFloat("rect", 2, obj) * (int)Rjson::GetFloat("rect", 3, obj);
        //    pointsAreaV.push_back(area); 
        //}
        //int kp = 0;
	//std::vector<int>::iterator kps = pointsAreaV.begin();
	//std::vector<int>::iterator xMaxIter = std::max_element(pointsAreaV.begin(), pointsAreaV.end());
	//while (kps != xMaxIter)
	//{
	//	kps++;
	//	kp++;
	//}
	for (int k = 0; k < obj_infos->Size(); ++k)
	{
		const Value* obj = Rjson::GetObject(k, obj_infos);
		//if(k == kp) 
		//{
		//        LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!skip current element {}",kp);
		//   	     continue; 
		//}

		int rect[4];
		bool aus = true;
		string reply;
		string orig_img_path;
		if(wsflg == 2)
		{
			for (int m = 0; m < 4; ++m)
			{
				rect[m] = (int)Rjson::GetFloat("rect", m, obj);
			}
			if(rect[2] < 25 || rect[3] < 25)
			{
				LOG_ERROR(ANAL_LOG,"here!!!!!!!!!!!not match size width---{},height---{}",rect[2],rect[3]);
				continue;
			}
			float scr = (float)rect[3] / (float)rect[2];
			if(scr > 1.4f) 
			{
				LOG_ERROR(ANAL_LOG,"here!!!!!!!!!!!not correct ratio---{},not warning",scr);
				continue;
			}
			if(!(rect[0] >=g_lefttopx && rect[0]+rect[2] <= g_rightbottomx && rect[1] >= g_lefttopy && rect[1]+rect[3] <= g_rightbottomy))
			{
				if(devid == string("JWJLTA980893"))
				{
					LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!out of range,not warning");
					continue;
				}
			}
			cv::Range r1(rect[0], rect[0] + rect[2]);
			cv::Range r2(rect[1], rect[1] + rect[3]);
			cv::Mat roi = cv::Mat(img, r2, r1).clone();
			std::vector<uchar> orig_encode;
			cv::imencode(".jpg", roi, orig_encode);
			roi.release();
			string orig_img = string(orig_encode.begin(), orig_encode.end());
			int reUploadOrigimg = ftp_upload_image(orig_img, orig_img_path, SMALLPUSH);
			if (reUploadOrigimg == -1)
				continue;
			orig_img_path = orig_img_path.replace(0, 3, "http");
			string mndpath;
			format_string(mndpath,"http://%s:%d/archives/faceImageCompareWithDb",mndIp.data(),mndPort);
			Document d1 = Rjson::rWriteDC();
			Rjson::rAdd(d1,"imgUrl",orig_img_path);
			Value data(kArrayType); 
			string storeId = string("700037021201000017");
			Value sid(storeId.c_str(),d1.GetAllocator()); 
			data.PushBack(sid,d1.GetAllocator());
			Rjson::rAdd(d1,"storeId",data);
			Rjson::rAdd(d1,"topN",1);
			Value par;
			par.CopyFrom(d1,d1.GetAllocator());
			Document respDoc;
			respDoc.SetObject(); 
			respDoc.AddMember("queryParam",par,respDoc.GetAllocator()); 
			string param = Rjson::ToString(respDoc); 
			LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!start to post retreive url {}-param-{}",mndpath,param);
			aus = http_post(mndpath.data(),param.data(),reply);
			LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!end   to post retreive result {}-reply-{}",aus,reply);
		}
		if(aus)
		{ 
			if(wsflg == 2)
			{
				Document d_vi;
				d_vi.Parse(reply);
				if (d_vi.HasParseError())
				{
					continue;
				}
				Value& data_arr = d_vi["data"];
				if(data_arr.Size() < 1) continue;
				if(!data_arr[0].HasMember("similarity")) continue;
				string sim = data_arr[0]["similarity"].GetString();
				float simsocre = atof(sim.c_str());
				LOG_ERROR(ANAL_LOG,"99999999999999999999999similarity--{}",simsocre);
				float susim = (float)g_sim / (float)100; 
				if(simsocre > susim)
				{
					cv::rectangle(img, cv::Rect(rect[0], rect[1], rect[2], rect[3]), cv::Scalar(0, 255, 0), 1);
					continue;
				}
			}
			{
				string bak_img_path;
				//cv::Mat tMat = cv::Mat(img).clone();
				cv::rectangle(img, cv::Rect(rect[0], rect[1], rect[2], rect[3]), cv::Scalar(0, 0, 255), 1);
				std::vector<uchar> orig_encode;
				cv::imencode(".jpg", img, orig_encode);
				string imgdata = string(orig_encode.begin(), orig_encode.end());

				int reUploadBkgimg = ftp_upload_image(imgdata, bak_img_path, BIGPUSH);
				/////////////////////////////////////////////////
				if (reUploadBkgimg == -1)
					continue;
				bak_img_path = bak_img_path.replace(0, 3, "http");
				LOG_INFO(ANAL_LOG,"999999999999999999999999999 current menjin image path {}",reUploadBkgimg);
				string communityCode;
				{
					std::string redisCommandStr; 
						redisReply *reply = (redisReply*)redisCommand(g_RedisWsbzConnector,"select %d",1);
						freeReplyObject(reply);
						reply = NULL;
						format_string(redisCommandStr, "hget deviceinfo %s",devid.data());
						reply = (redisReply*)redisCommand(g_RedisWsbzConnector, redisCommandStr.c_str());
						if (reply == NULL)
						{
							bConnectWsRedis = false;
								LOG_ERROR(ANAL_LOG,"hget communityCode false {}",devid);
								continue;
						}
						else
						{
							string repcache = string(reply->str,reply->len);
							communityCode = translateDeviceInfo(repcache);
							freeReplyObject(reply);
								reply = NULL;
								if(communityCode == "") continue;
						}
				}
				Document d2 = Rjson::rWriteDC();
				Rjson::rAdd(d2,"SmallImgUrl",orig_img_path);
				Rjson::rAdd(d2,"type",30);
				Rjson::rAdd(d2,"BigImgUrl",bak_img_path);
				Rjson::rAdd(d2,"SnapTime",pts);
				Rjson::rAdd(d2,"DeviceId",devid);
				Rjson::rAdd(d2,"communityCode",communityCode);
				string json = Rjson::ToString(d2);

				LOG_INFO(ANAL_LOG,"waooo!!begain to push the alarm warning MQ!!--{}",json);
				bool bRet = pAlCt.activemq_produce(json.data());
				if (!bRet)
				{
					bRet = pAlCt.activemq_producer_init(alarm_broker.data(), alarm_username.data(), alarm_password.data(), alarm_topic.data());
				}
				Document d3 = Rjson::rWriteDC();
				Rjson::rAdd(d3,"smallImgUrl",orig_img_path);
				Rjson::rAdd(d3,"bigImgUrl",bak_img_path);
				long time_p = pts / 1000L;
				int64_t snaptime = ts_time2(time_p);                      
				Rjson::rAdd(d3,"snapTime",snaptime);
				Rjson::rAdd(d3,"deviceId",devid);
				string capuid = create_uuid(false); 
				Rjson::rAdd(d3,"captureId",capuid);
				Rjson::rAdd(d3,"VideoType",1);
				Rjson::rAdd(d3,"communityCode",communityCode);
				Value par;
				par.CopyFrom(d3,d3.GetAllocator());
				Value data_r(kArrayType); 
				data_r.PushBack(par,d3.GetAllocator());
				Document respDoc;
				respDoc.SetObject(); 
				respDoc.AddMember("data",data_r,respDoc.GetAllocator()); 
				respDoc.AddMember("type",StringRef("followStranger"),respDoc.GetAllocator()); 
				string yeram = Rjson::ToString(respDoc); 
				LOG_INFO(ANAL_LOG,"waooo!!begain to push the feiwu warning MQ!!--{}",yeram);
				bRet = pAcCt.activemq_produce(yeram.data());
				if (!bRet)
				{
					bRet = pAcCt.activemq_producer_init(active_broker.data(),active_username.data(),active_password.data(),active_topic.data());
				}
				pFowst.kafka_client_poll(yeram.data());

			} 
		}   
	}
	img.release();
        ///////æåæ¨éæ¥è­?        
        
}
void AlgoConnector::do_ws_response(const string& param,const string& imgdata,const string& devid,int64_t pts,const string& captureId)
{
	Document dc_f;
	Rjson::Parse(dc_f,param);
        	
	cv::Mat img(cv::imdecode(cv::Mat(1, imgdata.size(),CV_8UC3,(unsigned char*)imgdata.data()),CV_LOAD_IMAGE_COLOR));
	if(!img.data) 
	{
		LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!img not valid! skip!");
	        return; 
	}
	std::vector<int> pointsAreaV;
        pointsAreaV.clear();
	const Value* obj_img = Rjson::GetObject("results", 0, &dc_f);
	const Value* obj_infos = Rjson::GetArray("obj_infos", obj_img);
	for (int k = 0; k < obj_infos->Size(); ++k)
        { 
	    const Value* obj = Rjson::GetObject(k, obj_infos);
	    int area = (int)Rjson::GetFloat("rect", 2, obj) * (int)Rjson::GetFloat("rect", 3, obj);
            pointsAreaV.push_back(area); 
        }
        int kp = 0;
	std::vector<int>::iterator kps = pointsAreaV.begin();
	std::vector<int>::iterator xMaxIter = std::max_element(pointsAreaV.begin(), pointsAreaV.end());
	while (kps != xMaxIter)
	{
		kps++;
		kp++;
	}
	for (int k = 0; k < obj_infos->Size(); ++k)
        {
	     const Value* obj = Rjson::GetObject(k, obj_infos);
             if(k == kp) 
             {
		     LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!skip current element {}",kp);
			     continue; 
             }
               
	     int rect[4];
	     for (int m = 0; m < 4; ++m)
	     {
		     rect[m] = (int)Rjson::GetFloat("rect", m, obj);
	     }
             if(devid == string("HPMDMMJHIK001920469"))
             {
                 if(rect[0] + rect[2] <= 250)
		 {
			 LOG_ERROR(ANAL_LOG,"here!!!!!!!!!!!not match width menjin---{}",rect[0] + rect[2]);
			 continue;
		 }
                 
             }
             if(rect[1] + rect[3] > 300)
             {
		LOG_ERROR(ANAL_LOG,"here!!!!!!!!!!!not match height---{}",rect[3]);
                continue;
             }
             if(rect[2] < 28 || rect[3] < 28)
             {
		LOG_ERROR(ANAL_LOG,"here!!!!!!!!!!!not match size width---{},height---{}",rect[2],rect[3]);
                continue;
             }
             float scr = (float)rect[3] / (float)rect[2];
             if(scr > 1.4f) 
             {
		LOG_ERROR(ANAL_LOG,"here!!!!!!!!!!!not correct ratio---{},not warning",scr);
                continue;
             }
             //float sdr = (float)rect[2] / (float)rect[3];
             //if(sdr > 1.4f) 
             //{
	     //   LOG_ERROR(ANAL_LOG,"here!!!!!!!!!!!not correct ratio---sdr {},not warning",sdr);
             //   continue;
             //}
	     cv::Range r1(rect[0], rect[0] + rect[2]);
	     cv::Range r2(rect[1], rect[1] + rect[3]);
	     cv::Mat roi = cv::Mat(img, r2, r1).clone();
	     std::vector<uchar> orig_encode;
	     cv::imencode(".jpg", roi, orig_encode);
             roi.release();
	     string orig_img = string(orig_encode.begin(), orig_encode.end());
	     string orig_img_path;
	     int reUploadOrigimg = ftp_upload_image(orig_img, orig_img_path, SMALLPUSH);
	     if (reUploadOrigimg == -1)
		     continue;
	     orig_img_path = orig_img_path.replace(0, 3, "http");
             string mndpath;
	     format_string(mndpath,"http://%s:%d/archives/faceImageCompareWithDb",mndIp.data(),mndPort);
	     Document d1 = Rjson::rWriteDC();
	     Rjson::rAdd(d1,"imgUrl",orig_img_path);
             Value data(kArrayType); 
             string storeId = string("700037021201000017");
             Value sid(storeId.c_str(),d1.GetAllocator()); 
             data.PushBack(sid,d1.GetAllocator());
	     Rjson::rAdd(d1,"storeId",data);
	     Rjson::rAdd(d1,"topN",1);
	     Value par;
	     par.CopyFrom(d1,d1.GetAllocator());
	     Document respDoc;
	     respDoc.SetObject(); 
             respDoc.AddMember("queryParam",par,respDoc.GetAllocator()); 
             string param = Rjson::ToString(respDoc); 
             string reply;
	     LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!start to post retreive url {}-param-{}",mndpath,param);
             bool aus = http_post(mndpath.data(),param.data(),reply);
	     LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!end   to post retreive result {}-reply-{}",aus,reply);
             if(aus)
             { 
		     Document d_vi;
		     d_vi.Parse(reply);
		     if (d_vi.HasParseError())
		     {
			 continue;
		     }
	             Value& data_arr = d_vi["data"];
                     if(data_arr.Size() < 1) continue;
                     if(!data_arr[0].HasMember("similarity")) continue;
                     string sim = data_arr[0]["similarity"].GetString();
                     float simsocre = atof(sim.c_str());
		     LOG_ERROR(ANAL_LOG,"99999999999999999999999similarity--{}",simsocre);
                     float susim = (float)g_sim / (float)100; 
                     if(simsocre > susim) 
                            continue;
                     {
                                string bak_img_path;
				cv::Mat tMat = cv::Mat(img).clone();
				cv::rectangle(tMat, cv::Rect(rect[0], rect[1], rect[2], rect[3]), cv::Scalar(0, 0, 255), 1);
				std::vector<uchar> orig_encode;
				cv::imencode(".jpg", tMat, orig_encode);
				string imgdata = string(orig_encode.begin(), orig_encode.end());
							
				int reUploadBkgimg = ftp_upload_image(imgdata, bak_img_path, BIGPUSH);
				tMat.release();
			    /////////////////////////////////////////////////
			     if (reUploadBkgimg == -1)
				     continue;
			     bak_img_path = bak_img_path.replace(0, 3, "http");
			     LOG_INFO(ANAL_LOG,"999999999999999999999999999 current menjin image path {}",reUploadBkgimg);
				 string communityCode;
                 {
					 std::string redisCommandStr; 
					 redisReply *reply = (redisReply*)redisCommand(g_RedisWsbzConnector,"select %d",1);
					 freeReplyObject(reply);
					 reply = NULL;
					 format_string(redisCommandStr, "hget deviceinfo %s",devid.data());
					 reply = (redisReply*)redisCommand(g_RedisWsbzConnector, redisCommandStr.c_str());
					 if (reply == NULL)
					 {
						 bConnectWsRedis = false;
							 LOG_ERROR(ANAL_LOG,"hget communityCode false {}",devid);
						continue;
					 }
					 else
					 {
						 string repcache = string(reply->str,reply->len);
						 communityCode = translateDeviceInfo(repcache);
						 freeReplyObject(reply);
							 reply = NULL;
						 if(communityCode == "") continue;
					 }
				 }
			     //hereå°¾éæ¥è­¦
			     Document d2 = Rjson::rWriteDC();
			     Rjson::rAdd(d2,"SmallImgUrl",orig_img_path);
			     Rjson::rAdd(d2,"type",30);
			     Rjson::rAdd(d2,"BigImgUrl",bak_img_path);
			     Rjson::rAdd(d2,"SnapTime",pts);
			     Rjson::rAdd(d2,"DeviceId",devid);
			     Rjson::rAdd(d2,"communityCode",communityCode);
			     string json = Rjson::ToString(d2);
                              
			     LOG_INFO(ANAL_LOG,"waooo!!begain to push the alarm warning MQ!!--{}",json);
			     bool bRet = pAlCt.activemq_produce(json.data());
			     if (!bRet)
			     {
				     bRet = pAlCt.activemq_producer_init(alarm_broker.data(), alarm_username.data(), alarm_password.data(), alarm_topic.data());
			     }
			     Document d3 = Rjson::rWriteDC();
			     Rjson::rAdd(d3,"smallImgUrl",orig_img_path);
			     Rjson::rAdd(d3,"bigImgUrl",bak_img_path);
				 long time_p = pts / 1000L;
				 int64_t snaptime = ts_time2(time_p);                      
			     Rjson::rAdd(d3,"snapTime",snaptime);
			     Rjson::rAdd(d3,"deviceId",devid);
                             string capuid = create_uuid(false); 
			     Rjson::rAdd(d3,"captureId",capuid);
			     Rjson::rAdd(d3,"VideoType",1);
			     Rjson::rAdd(d3,"communityCode",communityCode);
			     Value par;
			     par.CopyFrom(d3,d3.GetAllocator());
			     Value data_r(kArrayType); 
			     data_r.PushBack(par,d3.GetAllocator());
			     Document respDoc;
			     respDoc.SetObject(); 
			     respDoc.AddMember("data",data_r,respDoc.GetAllocator()); 
			     respDoc.AddMember("type",StringRef("followStranger"),respDoc.GetAllocator()); 
			     string yeram = Rjson::ToString(respDoc); 
			     LOG_INFO(ANAL_LOG,"waooo!!begain to push the feiwu warning MQ!!--{}",yeram);
			     //bRet = pAcCt.activemq_produce(yeram.data());
			     //if (!bRet)
			     //{
			     //        bRet = pAcCt.activemq_producer_init(active_broker.data(),active_username.data(),active_password.data(),active_topic.data());
			     //}
			     pFowst.kafka_client_poll(yeram.data());
                              
                     } 
             }   
        }
        ///////æåæ¨éæ¥è­?        
        
}
void AlgoConnector::do_face_control_alarm(const string& srcdata,const string& bkg_img,const string& relateparam,const string& feat,int64_t pts,int dbNo)
{
         
	Document dt;
	Rjson::Parse(dt,relateparam);
	string devId = Rjson::GetString("realDeviceId",&dt);
	int topN = 500;
	vector<ImgRecord> vRecs(topN);
	double minSim = (double)g_fcthdValue / (double)100; 
	string want_param("DeviceId,TaskId");
		string where_stmt;
		format_string(where_stmt, "DeviceId='%s' OR DeviceId='%s'",devId.data(),"global-control");
		LOG_INFO(ANAL_LOG,"begain to retrieve_image = {}",where_stmt);
		int rt = pIseApi.retrieve_image(ise_dbname[dbNo], 4,feat, want_param, where_stmt, minSim, vRecs.data(), topN);
		LOG_INFO(ANAL_LOG,"end to retrieve_image = {}", rt);
		vRecs.resize(rt);
		for (int sj = 0; sj < vRecs.size(); sj++)
		{
			ImgRecord& sr = vRecs[sj];
			//if (sr.sim < 0.75)
			//{
			//	LOG_INFO(MAIN_LOG,"waooo!!skip this target!!--{}",sr.v_params[0]);
			//	continue;
			//}
			//else if (sr.sim < 0.86)
			//{
			//	sr.sim += 0.05;
			//}
			//else{}
                        string taskid = sr.v_params[1];
                        string deviceid = sr.v_params[0];
                        string redisCache;
                        redisReply *reply = NULL;
						redisContext* p_RedisSyncConnector = NULL;
			p_RedisSyncConnector = redisConnectWithTimeout(redisIp.c_str(), redisPort, g_RedisTimeOut);
				if (p_RedisSyncConnector->err)
				{
					LOG_INFO(START_LOG,"connectRedisThread connect to redisServer fail---{}",p_RedisSyncConnector->errstr);
                    continue;
				}
			//reply = (redisReply*)redisCommand(g_RedisSyncConnector,"get %s",taskid.data());
			reply = (redisReply*)redisCommand(p_RedisSyncConnector,"select %d",redisdbNo);
#if 0
			if(dbNo == 5) 
			{
				reply = (redisReply*)redisCommand(g_RedisSyncConnector,"get %s",taskid.data());
			}
                        if(dbNo == 1)
                        {
				reply = (redisReply*)redisCommand(g_RedisSync2Connect,"get %s",taskid.data());
                        }
#endif
			if(reply == NULL)
			{
				//bConnect2Redis = false;
				LOG_ERROR(MAIN_LOG,"redis get task bind param false");
				continue;
			}
			else
			{
				freeReplyObject(reply);
				reply = NULL;
				reply = (redisReply*)redisCommand(p_RedisSyncConnector,"get %s",taskid.data());
				if(reply == NULL)
				{
					LOG_ERROR(MAIN_LOG,"redis get task bind param false");
					continue;
				}
				if(reply->type == REDIS_REPLY_NIL)
				{

					string where_delete;
					format_string(where_delete, "TaskId = '%s'",taskid.data());
					pIseApi.delete_img_rec_ws(ise_dbname[dbNo], where_delete);
					LOG_INFO(MAIN_LOG,"auto alarm cantel delete,check the ctrldb---{}",taskid);
					freeReplyObject(reply);
					reply = NULL;
					continue;

				}
				//auto ited = std::find(s_rediserr_list.begin(),s_rediserr_list.end(),reply->type);
				//if(ited == s_rediserr_list.end()) continue;
				if(reply->type == REDIS_REPLY_STRING)
				{
					LOG_INFO(MAIN_LOG,"check redis str and len---{}---{}",reply->str,reply->len);
					redisCache = string(reply->str,reply->len);
					LOG_INFO(MAIN_LOG,"check redis control info is {}",redisCache);
				}
				else if(reply->type == REDIS_REPLY_ERROR)
				{
					LOG_INFO(MAIN_LOG,"check redis str and len---{}---{}",reply->str,reply->len);
					redisCache = string(reply->str,reply->len);
					LOG_INFO(MAIN_LOG,"check redis control info is {}",redisCache);
					freeReplyObject(reply);
					reply = NULL;
					continue;
				}
				freeReplyObject(reply);
				reply = NULL;
				redisFree(p_RedisSyncConnector);
				p_RedisSyncConnector = NULL;
			}
				Document alarm2MqDoc;
				alarm2MqDoc.Parse(redisCache);
			
                         	Document::AllocatorType& allocat = alarm2MqDoc.GetAllocator();
                                string bkg_url,obj_url;
                                {
					cv::Mat ipmg(cv::imdecode(cv::Mat(1, bkg_img.size(),CV_8UC3,(unsigned char*)bkg_img.data()),CV_LOAD_IMAGE_COLOR));
					if(!ipmg.data) 
					{
						LOG_INFO(ANAL_LOG,"here!!!!!!!!!!!!!img not valid! skip!");
						continue; 
					}
					Document ds;
					Rjson::Parse(ds,srcdata);
					int rect[4];
					for (int m = 0; m < 4; ++m)
					{
						rect[m] = (int)Rjson::GetFloat("rect", m,&ds);
					}
					Cordinate input, output;
					input.left_top_x = rect[0];
					input.left_top_y = rect[1];
					input.width = rect[2];
					input.height = rect[3];

					CordinateConverse(ipmg, &input, output);

					rect[0] = output.left_top_x;
					rect[1] = output.left_top_y;
					rect[2] = output.width;
					rect[3] = output.height;
					////////////////////////////////////////

					cv::Range r1(rect[0], rect[0] + rect[2]);
					cv::Range r2(rect[1], rect[1] + rect[3]);
					cv::Mat roi = cv::Mat(ipmg, r2, r1).clone();
                                        
					std::vector<uchar> orig_encode;
					cv::imencode(".jpg", roi, orig_encode);
					roi.release();
					string obj_img = string(orig_encode.begin(), orig_encode.end());
                                        int reUploadOrigimg = ftp_upload_image(obj_img,obj_url, SMALLPUSH);
					if (reUploadOrigimg == -1)
						continue;
					obj_url= obj_url.replace(0, 3, "http");
                                        reUploadOrigimg = ftp_upload_image(bkg_img,bkg_url, BIGPUSH);
					if (reUploadOrigimg == -1)
						continue;
					bkg_url= bkg_url.replace(0, 3, "http");
                                }
				string temptime = ts_time((long)pts);
				//printf("999999999999999999999999---time--%s\n",temptime.data());
                                //int64_t time_p = pts;
				//string snapTime = ts_time(pts);
				alarm2MqDoc.AddMember("similarity",sr.sim, allocat);
				alarm2MqDoc.AddMember("bigImgUrl", bkg_url, allocat);
				alarm2MqDoc.AddMember("smallImgUrl", obj_url, allocat);
                                //string snapTimeV = std::to_string(snapTime);
				alarm2MqDoc.AddMember("snapTime", temptime, allocat);
                                alarm2MqDoc.RemoveMember("deviceId");
                                alarm2MqDoc.AddMember("deviceId",deviceid, allocat);
				std::string alarm2MqStr = Rjson::ToString(alarm2MqDoc);
                                
			//¿¿mq
			LOG_INFO(ANAL_LOG,"waooo!!begain to push the alarm warning MQ!!--{}",alarm2MqStr);
                        if(dbNo == 5)
			{
				bool bRet = pFaCt.activemq_produce(alarm2MqStr.data());
				if (!bRet)
				{
					bRet = pFaCt.activemq_producer_init(facl_broker.data(), facl_username.data(), facl_password.data(), facl_topic.data());
				}
				LOG_INFO(ANAL_LOG,"alarm---push to mq {} !", bRet == true ? "success" : "failed");
                        }
                        if(dbNo == 1)
			{
				bool bRet = pFaCt1.activemq_produce(alarm2MqStr.data());
				if (!bRet)
				{
					bRet = pFaCt1.activemq_producer_init(facl_broker1.data(), facl_username.data(), facl_password.data(), facl_topic.data());
				}
				LOG_INFO(ANAL_LOG,"alarm---push to mq {} !", bRet == true ? "success" : "failed");
                        }
		}
}
void AlgoConnector::_send_relate_frame(const rmmt::ShmVecType& req_vec,const string& req_data,const string& feat1,const string& feat2,const string& param,int do_type)
{
	using namespace Rjson;
	string input;
	if(do_type == 1)
	{
		Rjson::Doc doc({ { "cmd", "AP_FACE_FEATURE_CMD_DETECT" } });
		auto paramObj = doc.lv().add_new_obj("param");
		paramObj.add(string("flag"), FVal(1));
		input = doc.dumps();
	}
	else if(do_type == 2)
	{
		int64_t pts = 0;
		if (!req_data.empty())
		{
			Document dc;
			Rjson::Parse(dc, req_data);
			Rjson::GetInt64V(pts, "pts", &dc);
			Value param_info = Rjson::rObject();
			param_info.CopyFrom(dc, dc.GetAllocator());
			Document dc_w = Rjson::rWriteDC();
			Rjson::rAdd(dc_w, "param", param_info);
			Rjson::rAdd(dc_w, "cmd", StringRef("AP_FACE_FEATURE_CMD_FEATURE"));//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			input = Rjson::ToString(dc_w);
		}
		else
		{
			Rjson::Doc doc({ { "cmd", "AP_FACE_FEATURE_CMD_FEATURE" } });
		    input = doc.dumps();
		}
		LOG_INFO(MAIN_LOG,"check the send ipc input---{}",input.data());
	}
	else
	{
		string fet1_base64 = base64_encode(reinterpret_cast<const unsigned char*>(feat1.data()), feat1.size());
		string fet2_base64 = base64_encode(reinterpret_cast<const unsigned char*>(feat2.data()), feat2.size());
		Rjson::Doc doc({ { "cmd", "AP_FACE_FEATURE_CMD_FEATURE_COMPARE" } });
		doc.lv().add(string("feat1"), fet1_base64).add(string("feat2"), fet2_base64);
		input = doc.dumps();
	}
	byte_buffer bb;
	bb << (int)78 << input;
	//rmmt::ShmVecType vmn;
	std::string resp_data;
	rmmt::ShmVecType vmn_resp;
	try
	{
		m_algo_pkg->pIpc->do_IPC_NW(bb.data_ptr(), bb.data_size(),req_vec,resp_data, vmn_resp,do_type,param,req_data);
	}
	catch (GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC GeneralException2 :{}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == -19)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_algo_pkg->is_conn = false;
		}
		//vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (...)
	{
		LOG_INFO(MAIN_LOG,"algopkg_alg_process do_IPC exception\n");
		//vmn.clear();
		vmn_resp.clear();
		return;
	}
	//vmn.clear();
	vmn_resp.clear();
	return;
}

void AlgoConnector::_send_null_frame()
{
	rmmt::ShmVecType vmn;
	rmmt::ShmVecType vmn_resp;
        try
	{
		byte_buffer bb;
		if(m_sess_t)
		bb << (int)77 << m_sess_t->ap_sess << (int64_t)0;
		else
		bb << (int)77 << 0;
		std::string resp_data;
		if(m_sess_t)
		m_sess_t->ap_hdl->pIpc->do_IPC_NW(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp,1,"","{}");
		else
        m_algo_pkg->pIpc->do_IPC_NW(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp,1,"","{}");
	}
	catch(GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_sess_t->ap_hdl->is_conn = false;
		}
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (...)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC exception\n");
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	vmn.clear();
	vmn_resp.clear();
	return;
}

ap_output_t AlgoConnector::_send_null_frame_wait()
{
	ap_output_t data = {};
	data.errc = 0;
	rmmt::ShmVecType vmn;
	rmmt::ShmVecType vmn_resp;
    try
	{
		byte_buffer bb;
		bb << (int)77 << m_sess_t->ap_sess << (int64_t)0;
		std::string resp_data;
		m_sess_t->ap_hdl->pIpc->do_IPC(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp);
		data.text_len = resp_data.size();
		data.text = (char*)malloc(data.text_len + 1);
		strncpy(data.text, resp_data.data(), resp_data.size());
		data.text[data.text_len] = '\0';
		if (vmn_resp.size() > 0)
		{
			data.binary_len = vmn_resp[0]->get_size();
			data.binary = (char*)malloc(data.binary_len + 1);
			memcpy(data.binary, (char*)vmn_resp[0]->get_ptr(), data.binary_len);
			data.binary[data.binary_len] = '\0';
		}
	}
	catch(GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_sess_t->ap_hdl->is_conn = false;
		}
		data.errc = e.err_code();
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	catch (...)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC exception\n");
		data.errc = -20;
		vmn.clear();
		vmn_resp.clear();
		return data;
	}
	vmn.clear();
	vmn_resp.clear();
	return data;
}

void AlgoConnector::algo_sess_proc_send(inputinfo_t * input)
{
	if (input == nullptr)
	{
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	bool checky = false;
	Document dc;
	Rjson::Parse(dc, input->param1);
	int type = Rjson::GetInt("type", &dc);
	if (type == 0)
	{
		checky = true;
	}
	rmmt::ShmVecType vmn;
	rmmt::ShmVecType vmn_resp;
	try
	{
		byte_buffer bb;
		if (checky)
		{
			bb << (int)77 << m_sess_t->ap_sess << (int64_t)0;
		}
		else
		{
			if (input->n_img > 0)
			{
				std::vector<imageinfo_t> vImgDatas(*input->p_imgs, *input->p_imgs + input->n_img);
				for (auto& it : vImgDatas)
				{
					rmmt::ShmImage simg;
					simg.clone(it.p_img);

					if (it.pts < 0)
					{
						simg.getHdr()->pts = (uint64_t)0;
					}
					else
					{
						simg.getHdr()->pts = it.pts;
					}//zj 20190815 ï¿½ï¿½ï¿½ï¿½ï¿½Ð·ï¿½ï¿½ï¿½×ªï¿½Þ·ï¿½ï¿½Å²ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
					vmn.push_back(simg.get_shm());
					bb << (int)77 << m_sess_t->ap_sess << it.pts;
				}
			}
			else
			{
				bb << (int)77 << m_sess_t->ap_sess << (int64_t)0;
			}
		}
		std::string resp_data;
		m_sess_t->ap_hdl->pIpc->do_IPC_NW(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp,1,"","{}");
	}
	catch (GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_sess_t->ap_hdl->is_conn = false;
		}
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (rmmt::IPCException &e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC IPCException: {}/{}", e.ret_code, e.err_msg);
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (std::exception& e)
	{
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new stdException error!{}", e.what());
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (...)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC exception\n");
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	vmn.clear();
	vmn_resp.clear();
	return;
}

void AlgoConnector::algo_proc_send(inputinfo_t * input)
{
	if (input == nullptr)
	{
		return;
	}
	byte_buffer bb;
	bb << 78 << string(input->param1);
	rmmt::ShmVecType vmn;
	try
	{
		//ï¿½ï¿½ï¿½ï¿½ï¿½ã·¨Êµï¿½ï¿½
	    if (!m_algo_pkg->is_init)
	    {
			m_algo_pkg->is_conn = false;
			return;
		}
		if (input->n_img > 0)
		{
			std::vector<imageinfo_t> vImgDatas(*input->p_imgs, *input->p_imgs + input->n_img);
			for (auto& it : vImgDatas)
			{
				rmmt::ShmImage simg;
				simg.clone(it.p_img);
				vmn.push_back(simg.get_shm());
			}
		}
	}
	catch (GeneralException2 &e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process clone failed: {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == 32)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_algo_pkg->is_conn = false;
		}
		vmn.clear();
		return;
	}
	std::string resp_data;
	rmmt::ShmVecType vmn_resp;
	try
	{
		m_algo_pkg->pIpc->do_IPC(bb.data_ptr(), bb.data_size(), vmn, resp_data, vmn_resp);
	}
	catch (GeneralException2& e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC GeneralException2 : {}/{}", e.err_code(), e.err_str());
		if (e.err_code() == 10054 || e.err_code() == 10061 || e.err_code() == -1 || e.err_code() == -19)//ï¿½ì³£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		{
			m_algo_pkg->is_conn = false;
		}
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (rmmt::IPCException &e)
	{
		LOG_ERROR(MAIN_LOG, "algopkg_alg_process do_IPC IPCException: {}/{}", e.ret_code, e.err_msg);
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (std::exception& e)
	{
		LOG_ERROR(MAIN_LOG, "algo_pkg chan new stdException error!{}", e.what());
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	catch (...)
	{
		LOG_INFO(MAIN_LOG,"algopkg_alg_process do_IPC exception\n");
		vmn.clear();
		vmn_resp.clear();
		return;
	}
	vmn.clear();
	vmn_resp.clear();
	return;
}

ap_output_t AlgoConnector::algo_proc_recv()
{
	ap_output_t data = {};
	return  data;
}

void AlgoConnector::set_ipc_end()
{
	m_algo_pkg->pIpc->m_fQuit = 1;
}

int AlgoConnector::get_frame_load()
{
	return m_algo_pkg->pIpc->check_map();
}

void AlgoConnector::algopkg_output_release(ap_output_t* output)
{
#if 1
	if (output->image_info) {
		if (output->image_info->p_img) {
			cvReleaseImage(&output->image_info->p_img);
			output->image_info->p_img = nullptr;
		}
		delete output->image_info;
		output->image_info = nullptr;
	}
#endif
	if (output->text)
	{
		free(output->text);
		output->text = NULL;
		output->text_len = 0;
	}
	if (output->binary)
	{
		free(output->binary);
		output->binary = NULL;
		output->binary_len = 0;
	}
	if (output->errmsg)
	{
		free(output->errmsg);
		output->errmsg = NULL;
		output->errc = 0;
	}
}
