#include "Util.h"//column del 17
#include "spdlog_helper.h"
#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STL_

#include <unordered_map>
#include <utility>
#include <memory>
//#include <mutex>
#include <thread>
#include <list>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <spdlog/spdlog.h>
#include <tbb/reader_writer_lock.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>
//#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "basic_utils.h"
#include "spdlog_helper.h"
#include "StreamCmdApis.h"
#include "KafkaClientAPI.h"
#include "Base64.h"
#include "jsonparser.h"
#include "jsonhelper.h"
#include "rapidjson/Rjson.hpp"
#include "video_info_dict.h"
#include "rmmt_wrap2.h"
#include "rmmt_shm_image.h"
#include "boyun_IPC_v2.h"
#include "create_uuid.h"
#include "download_image.h"
#include "bath_utils.h"
#include "fcse_error.h"
#include "../include/hiredis/hiredis.h"
#include "../include/hiredis/async.h"
#include "../include/hiredis/adapters/libevent.h"
#include <libevent/include/event2/event.h>
#include "HttpSession.h"
#include "ws_stream_sender.h"
#include "sqlite3.h"

//typedef websocketpp::server<websocketpp::config::asio> WsServerType;
//typedef WsServerType::message_ptr message_ptr;

using namespace std;
using tbb::reader_writer_lock;
using MsgChanPtr = shared_ptr<rmmt::MsgChannel>;
using tbb::reader_writer_lock;
extern std::vector<string> ise_dbname;
//sqlite3
extern sqlite3 *sql;
SimpleNotifier g_chan_nt;
string node_ip = string("hisense_6");
extern int g_bwsbz;
extern std::string mndIp;
extern std::vector<std::string> devices;
extern int mndPort;
extern IseApi  pIseApi;
///////////redis////////////////
extern int redisdbNo;
extern bool bUseRedis;
extern std::string redisIp;
extern int redisPort;
extern std::string redisUserName;
extern std::string redisPassWord;
extern timeval g_RedisTimeOut;
extern bool bConnect2Redis;
extern redisAsyncContext* g_RedisConnector;

extern atomic<int> g_globalCtlNum;//全局布控个数
extern redisContext* g_RedisSyncConnector;
extern tbb::reader_writer_lock g_Redisrwl;
extern std::string kafka_broker;
extern std::string kafka_topic;
extern std::string kafka_groupid;
extern KafkaConsumerClient         m_consumer_;
std::thread                 g_recvthr;
std::thread                 g_sendthr;
std::thread                 g_querydbthr;
std::thread                 g_controlthr;
tbb::concurrent_bounded_queue<message_ptr> receive_queue_;
static MsgChanPtr  g_chan;// = new rmmt::MsgChannel;
extern string dpca_server_addr;
//list<string> s_rediserr_list;
std::vector<int> s_rediserr_list;
unordered_map<string, int64_t> s_msg_list;
unordered_map<string, int64_t> s_wsbz_list;
//布控任务管理相关
tbb::concurrent_bounded_queue<control_ct_ptr> control_queue;

static void trim(string &s, char c)
{
	int index = 0;
	if (!s.empty())
	{
		while ((index = s.find(c, index)) != string::npos)
		{
			s.erase(index, 1);
		}
	}

}

std::string translateTime(const std::string&  taskid)
{
        auto pos = taskid.find_first_of('.');//
        if (pos == std::string::npos) return taskid;
        std::string reqid = taskid.substr(0, pos);//
        return reqid;
}

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
	redisReply *reply = (redisReply *)r;
	if (reply == NULL) return;
	//printf("redid get bind param %s\n",(char*)privdata);
	
        RedisBidId *controlptr = (RedisBidId *)privdata;
        int cmd = controlptr->type;
        delete controlptr;
        switch (cmd)
        {
            case 1:
            {
               string redisCache = string(reply->str,reply->len);
               if(!redisCache.empty())
               {
		       FaceControlBegain1(redisCache.data());
               }     
            }break;
            default:
            break;
        }
     return; 
}
void subCallback(redisAsyncContext *c, void *r, void *privdata) {
#if 1
   //while(true)
  { 	redisReply *reply = (redisReply *)r;
        //num++;
	//printf("---------------------------------check the reload num %d\n",num);
        RedisBidId *controlptr = (RedisBidId *)privdata;
        int cmd = controlptr->type;
        string controlid = controlptr->normalId;
        //delete controlptr;
	if (reply == NULL) 
        {
           //sleep(3);
           //continue;
           return;
        }
	printf("subcribe reload %d:%s\n",cmd,controlid.data());
	if ( reply->type == REDIS_REPLY_ARRAY && reply->elements == 3 ) {
                 string elestr = string(reply->element[0]->str,reply->element[0]->len);
                 string sub = string("subscribe");
                 string msg = string("message");
		 //printf("subcribe element %d:%d:%s\n",reply->elements,reply->type,elestr.data());
		if (elestr == msg) {
			printf( "Received[%s] cmd:%d\nchannel:%s\nvalue:%s\n",
					controlid.data(),
					cmd,
					reply->element[1]->str,
					reply->element[2]->str );
			switch(cmd)
			{
				case 1:
					{
					}break;
				case 2:
					{
						string where_delete;
						format_string(where_delete, "TaskId = '%s'",controlid.data());
						pIseApi.delete_img_rec_ws(ise_dbname[5], where_delete);
						LOG_INFO(MAIN_LOG,"auto alarm cantel delete,check the ctrldb---{}",controlid);
						pIseApi.delete_img_rec_ws(ise_dbname[1], where_delete);
						LOG_INFO(MAIN_LOG,"auto alarm cantel delete,check the ctrldb--1---{}",controlid);
						control_ct_ptr curCol = make_shared<RedisBidId>(controlid,2);
						control_queue.push(curCol);             
					}
				default:
					break;
                        } 
			delete controlptr;
		}
                //{
		//	freeReplyObject(reply);
		//	reply = NULL;
                //}
	}
    }
#endif
}

void InitGusetChan(const std::string& sessionid)
{
        g_chan = rmmt::MsgChannel::connect_server(dpca_server_addr);
        byte_buffer bb;                                                                                                                                                                       
        bb << sessionid;//sessionid;                                                                                                                                                          
        g_chan->send_msg(bb.data_ptr(), bb.data_size());
	std::vector<shared_ptr<rmmt::MemNode> > vn;                                                                                                                                           
	string ret = g_chan->recv_msg(vn);
	CharSeqReader chrd(ret.data(), ret.size());   
	int rc = 0;
	chrd >> rc;
	if (rc) 
	{   
		std::string tmps;
		chrd >> tmps; 
		LOG_ERROR(MAIN_LOG, "_start_vstreamer failed: {}", tmps);
		g_chan->close();  
		g_chan.reset();
                //delete g_chan;
                //g_chan = NULL;
		throw GeneralException2(rc, tmps); 
	}
	LOG_ERROR(MAIN_LOG, "_start_vstreamer succeed");
	return;
}

int sqlite_exec_callback(void *para, int nCount, char **pValue, char** pName)
{
    std::string ControlId = pValue[0];
    long stapts = atol(pValue[1]);
    long stopts = atol(pValue[2]);
    //std::string hisId,hikId;
    //LOG(INFO) << "check cfg his id : " << hisId << "---hik id : " << hikId;
    //{
	//    tbb::reader_writer_lock::scoped_lock locker(s_hiskey_locker);
    //        g_hisKeyIdMap.insert(std::make_pair(hisId,hikId));
    //}
    //{
	//    tbb::reader_writer_lock::scoped_lock locker(s_hikkey_locker);
    //        g_hikKeyIdMap.insert(std::make_pair(hikId,hisId));
    //}
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long curpts = tv.tv_sec;
	if(stapts - curpts < 60)
	{
		control_ct_ptr curCol = make_shared<RedisBidId>(ControlId,1);
        control_queue.push(curCol);             
	}
	if(stopts - curpts < 10)
	{
		control_ct_ptr curCol = make_shared<RedisBidId>(ControlId,2);
        control_queue.push(curCol);             
	}
	return 0;
}

void InitWebsocketServerAndStartLoop(int sport)
{
     //s_rediserr_list.push_back(REDIS_ERR);
     //s_rediserr_list.push_back(REDIS_OK);
     //s_rediserr_list.push_back(REDIS_REPLY_STRING);
     //s_rediserr_list.push_back(REDIS_REPLY_ARRAY);
     //s_rediserr_list.push_back(REDIS_REPLY_INTEGER);
     //s_rediserr_list.push_back(REDIS_REPLY_NIL);
     //s_rediserr_list.push_back(REDIS_REPLY_STATUS);
     //s_rediserr_list.push_back(REDIS_REPLY_ERROR); 
     g_controlthr = std::thread([&]{
			 while(true)
			 {
			    
				 control_ct_ptr curCol = make_shared<RedisBidId>();
				 control_queue.pop(curCol);
				 LOG_ERROR(MAIN_LOG,"start pop to control queue,type is {}",curCol->type);           
                 if(curCol->type == 1)
				 {
					 string ScheduleId;
					 format_string(ScheduleId,"%s_N%d",curCol->normalId.data(),1);
					 string redisCommandStr;
					 format_string(redisCommandStr, "get %s", ScheduleId.data());
					 LOG_ERROR(MAIN_LOG,"start6666666666666666666666666666666666666666\n");           
					 redisReply *rep = (redisReply*)redisCommand(g_RedisSyncConnector,"select %d",redisdbNo);
					 freeReplyObject(rep);
					 rep = NULL;
					 rep =(redisReply*)redisCommand(g_RedisSyncConnector,redisCommandStr.c_str());
					 LOG_ERROR(MAIN_LOG,"end--6666666666666666666666666666666666666666\n");           
					 if(rep == NULL) 
					 {
					      LOG_ERROR(MAIN_LOG,"6666---cmd 1 get return false");   
					 }
					 else  
					 {
						 LOG_ERROR(MAIN_LOG,"rep str {}",rep->str);
						 string repcache = string(rep->str,rep->len);
						 LOG_ERROR(MAIN_LOG,"cmd 1 get return true {}",repcache.data());           
						 if(!repcache.empty())
						 {
							 FaceControlBegain1(repcache.data());
						 }     
						 freeReplyObject(rep);
						 rep = NULL;
					 }
				 }
	             else if(curCol->type == 2)
				 {
					 LOG_INFO(MAIN_LOG,"delete db ...");
					 char *errMsg = NULL;
					 char sqlCmd[256];
					 memset(sqlCmd, 0, sizeof(sqlCmd));
					 sprintf(sqlCmd, "delete from ControlInfo where TaskID = '%s';",curCol->normalId.data());
					 if (sqlite3_exec(sql, sqlCmd, NULL, NULL, &errMsg) != SQLITE_OK)
					 {   
						 LOG_ERROR(MAIN_LOG,"delete  failed : {}",errMsg);
						 //throw GeneralException2(-66,errMsg);
					 } 
					 if (errMsg != NULL)
					 {
						 sqlite3_free(errMsg);
						 errMsg = nullptr;
					 }
					 std::string redisCommandStr; 
					 format_string(redisCommandStr, "del %s", curCol->normalId.data());
					 redisReply *reply = (redisReply*)redisCommand(g_RedisSyncConnector,"select %d",redisdbNo);
					 freeReplyObject(reply);
					 reply = NULL;
					 reply = (redisReply*)redisCommand(g_RedisSyncConnector, redisCommandStr.c_str());
					 if (reply == NULL)
					 {
						 bConnect2Redis = false;
						 LOG_ERROR(ANAL_LOG,"OnFaceControlCancel hdel FaceControlTask:SessionHash failed taskId:{}",curCol->normalId.data());
					 }
					 else
					 {
						 freeReplyObject(reply);
						 reply = NULL;
					 }
				 }
				 else{}
			 }
	 });
	 g_querydbthr = std::thread([&]{
			 while(true)
			 {
				 char *errMsg = NULL;
				 if (sqlite3_exec(sql, "SELECT * FROM ControlInfo;", sqlite_exec_callback, NULL, &errMsg) != SQLITE_OK)
				 {
				    LOG_ERROR(MAIN_LOG,"sqlite query error : {}",errMsg);
				    //return -1;
				 }
				 if (errMsg != NULL)
				 {
					 sqlite3_free(errMsg);
					 errMsg = nullptr;
				 }
				 sleep(60);
			 }
     });
	 g_recvthr = std::thread([&]{
#if 1
		  LOG_INFO(MAIN_LOG, "666666666666666");
			  //BusinessSession * bs = (BusinessSession *)arg;
			  message_t tmp_message;            
			  while (true)                 
			  {
                              try{
			  	//message_t *tmp_message = new message_t;
			  	//LOG_INFO(MAIN_LOG, "thread 1111 begin!");
			  	message_ptr tmp_message = make_shared<message_t>();
			  	if (!(m_consumer_).KafkaConsume(tmp_message.get()))                                                                              
			  	{
					//LOG(INFO) << "consuming messgae : %%partition : " << tmp_message->m_partition << " %%offset : "<< tmp_message->offset;     
					while (!(receive_queue_).try_push(tmp_message))                                                                        
					{
						usleep(100);
					}
			  	}
			  	else                            
			  	        continue;                   
			  	//LOG_INFO(MAIN_LOG, "thread 1111 end!");
                              }
                              catch(...)
                              {
						usleep(100);
                                        continue; 
                              }
			  }  
#else
	  LOG_INFO(MAIN_LOG, "check dir begin");
	  listDir(g_path.data());
	  //is_run_ = 0;
	  //byte_buffer bb;
	  //bb << (int)81 << string("eof");
		  //g_chan->send_msg(bb.data_ptr(), bb.data_size());
#endif
	  });
	  g_sendthr = std::thread([&]{
			  while (true)
			  {
                             try{
#if 1
			  	message_ptr tmp_ptr = make_shared<message_t>();
			  	while (!receive_queue_.try_pop(tmp_ptr))
			  	{
					//LOG_INFO(MAIN_LOG, "thread 222222 get the info from kafka {}!");
					usleep(100);
			  	}
			  	Document d_vi;
			  	d_vi.Parse(tmp_ptr->payload);
			  	if (d_vi.HasParseError())
			  	{
					continue;
			  	}
#if 0
                                int flag = d_vi["code"].GetInt();
                                if(!(flag == 1 || flag == 2)) continue;
#else
                                string flag = d_vi["code"].GetString();
                                if(!(flag == "1" || flag == "2")) continue;
#endif
			  	//LOG_INFO(MAIN_LOG, "get the info from kafka {}!",tmp_ptr->payload);
			        Value& data_arr = d_vi["data"];//Rjson::GetObject("data",0,&d_vi);
			  	for (int i = 0; i < data_arr.Size(); ++i)
			  	{
			  	        //if(!(data_arr[i].HasMember("RLZPXXBZ") || data_arr[i].HasMember("MJJCJLXXBZ"))) continue;
                                        string flagCode;
                                        if (data_arr[i].HasMember("RLZPXXBZ"))
                                           flagCode = data_arr[i]["RLZPXXBZ"].GetString();
                                        else if (data_arr[i].HasMember("MJJCJLXXBZ"))
                                           flagCode = data_arr[i]["MJJCJLXXBZ"].GetString();
                                        else
                                           continue;
                                        //list<string>::iterator iter;
                                        //iter = std::find(s_msg_list.begin(),s_msg_list.end(),flagCode);
                                        //if(iter != s_msg_list.end())
                                        //{
                                        //    continue;
                                        //}
                                        //else
                                        //{
                                        //    s_msg_list.push_back(flagCode);
					//    //{
					//    //        list<string>::iterator iter;
					//    //        for(iter = s_msg_list.begin(); iter != s_msg_list.end() ;iter++)
                                        //    //        {
					//    //    	    LOG_ERROR(MAIN_LOG, "here!!!!!!!!!!!!!!!check list {}!",*iter);
                                        //    //            
                                        //    //        }
					//    //}
                                        //    if(s_msg_list.size() > 1000)
                                        //      s_msg_list.pop_front();
                                        //}
			  	        //if(!data_arr[i].HasMember("TP")) continue;
			  	        //if(!(data_arr[i].HasMember("CJSBXXBZ") || data_arr[i].HasMember("MJDBH"))) continue;
			  	        if(!data_arr[i].HasMember("CJSJ")) continue;
                                        string deviceid = "00000000000000000000";
                                        if(data_arr[i].HasMember("CJSBXXBZ"))
                                        deviceid = data_arr[i]["CJSBXXBZ"].GetString();
                                        else if(data_arr[i].HasMember("MJDBH"))
                                        deviceid = data_arr[i]["GLDMJ_CJSBXXBZ"].GetString();
                                        else 
                                        continue;
			  	        //string deviceid = data_arr[i]["CJSBXXBZ"].GetString();
			  	        //auto it  = std::find(m_deiVec.begin(),m_deiVec.end(),deviceid);
			  	        //if(it == m_deiVec.end()) continue;
			  	        string img64;// = data_arr[i]["TP"].GetString();//NLSX TP
					if(data_arr[i].HasMember("TP"))
                                        {
						img64 = data_arr[i]["TP"].GetString();
                                                data_arr[i].RemoveMember("TP");
						if(data_arr[i].HasMember("NLSX"))
                                                data_arr[i].RemoveMember("NLSX");
						if(data_arr[i].HasMember("KMRLZP"))
                                                data_arr[i].RemoveMember("KMRLZP");
                                        }
                                        else if(data_arr[i].HasMember("NLSX"))
                                        {
						//string sid = create_uuid(false);
						img64 = data_arr[i]["NLSX"].GetString();
                                                data_arr[i].RemoveMember("NLSX");
						if(data_arr[i].HasMember("KMRLZP"))
                                                data_arr[i].RemoveMember("KMRLZP");
                                                {
#if 0
                                                        string smallimg = data_arr[i]["NLSX"].GetString();
							data_arr[i].RemoveMember("NLSX");
							string smalldata = base64_decode(smallimg);
							cv::Mat img(cv::imdecode(cv::Mat(1, smalldata.size(),CV_8UC3,(unsigned char*)smalldata.data()),CV_LOAD_IMAGE_COLOR));
							string path = fmt::format("/home/test_zj/testimg/{}_smallcap.jpg",sid);
                                                        if(img.data)
                                                        {
                                                           cv::imwrite(path.data(),img); 
                                                        }
#endif
							//string small_path;
							//int upd = ftp_upload_image(smalldata, small_path, SMALLPUSH);
							//if (upd != -1)
							//small_path = small_path.replace(0, 3, "http");
							//LOG_INFO(MAIN_LOG, "999999999999999999999999999999999999999999999999999  must check img valid big path {}",small_path);

                                                }
#if 0
						string bigdata = base64_decode(img64);
						cv::Mat mat(cv::imdecode(cv::Mat(1, bigdata.size(),CV_8UC3,(unsigned char*)bigdata.data()),CV_LOAD_IMAGE_COLOR));
						string path1 = fmt::format("/home/test_zj/testimg/{}_bigcap.jpg",sid);
						if(mat.data)
						{
							cv::imwrite(path1.data(),mat); 
						}
#endif
						//string big_path;
						//int upd = ftp_upload_image(bigdata, big_path, SMALLPUSH);
						//if (upd != -1)
						//	big_path = big_path.replace(0, 3, "http");
						//LOG_INFO(MAIN_LOG, "999999999999999999999999999999999999999999999999999  must check img valid big path {}",big_path);
                                                
                                        }
                                        else if(data_arr[i].HasMember("KMRLZP"))
                                        {
						img64 = data_arr[i]["KMRLZP"].GetString();
                                                data_arr[i].RemoveMember("KMRLZP");
                                        } 
                                        else
                                        continue;
					Document facedoc;
					facedoc.CopyFrom(data_arr[i],d_vi.GetAllocator());
					string tempStr = Rjson::ToString(facedoc); 
					LOG_INFO(MAIN_LOG, "get single info from kafka {}!",tempStr);
			  	        string imgdata = base64_decode(img64);
			  	        cv::Mat img(cv::imdecode(cv::Mat(1, imgdata.size(),CV_8UC3,(unsigned char*)imgdata.data()),CV_LOAD_IMAGE_COLOR));
			  	        rmmt::ShmImage simg;
			  	        simg.clone(img);
					if (simg.checkImgValid() != 0)
                                        {
						LOG_ERROR(MAIN_LOG, "warning!!!!!!!!!!!!!!!!!!img is not valid");
                                                continue;
                                        }
			  	        rmmt::ShmVecType vmn;
			  	        vmn.push_back(simg.get_shm());
			  	        string snaptime = translateTime(data_arr[i]["CJSJ"].GetString());
			  	        trim(snaptime, ' ');
				        trim(snaptime, '-');
			  	      	trim(snaptime, ':');
                                        int64_t currt_t = time_ts(snaptime.data());
                                        auto itdo = s_msg_list.find(deviceid);
                                        if(itdo != s_msg_list.end())
                                        {
                                           int64_t temp_time = itdo->second;
                                           if(currt_t == temp_time) 
                                           {
                                               continue;
                                           }
                                           else
                                           {
                                              s_msg_list[deviceid] = currt_t;
                                           }
                                        }
                                        else
                                        {
                                           s_msg_list.insert(make_pair(deviceid,currt_t));
                                        }
			  	      	int64_t snapTime = currt_t * 1000L;
					simg.getHdr()->pts = currt_t;//videoPts * 1000 + mPs;
                                        //int64_t snapTime = ts_time2(temppts); 
			  	        if(g_chan && g_chan->is_valid())
			  	        {
                                                //一直接受数据不用结束
			  	      	  	//if(temppts > m_endPts && m_endPts > 0) 
			  	      	  	//{
			  	      	  	//        byte_buffer bb;
			  	      	  	//        //bb << (int)81 << string("eof");
			  	      	  	//        bb << (int)2;// << string("eof");
			  	      	  	//        g_chan->send_msg(bb.data_ptr(), bb.data_size());
			  	      	  	//        continue;
			  	      	  	//}
			  	      	  	string info = "{}";
                                                string virtualDevId = string("c56f9e8b25d04594b825b43bddeaf704");
			  	      	  	if(data_arr[i].HasMember("RLZPXXBZ"))
			  	      	  	{
 							Document dd = Rjson::rWriteDC();
							auto iter = std::find(devices.begin(),devices.end(),deviceid);
							if(iter != devices.end())
						        //if(deviceid == "JWJLTA980893")
							{
#if 1
                                                                if(g_bwsbz == 1)
								Rjson::rAdd(dd,"WSBZ",2);
#endif
						     	} 
                                                        string captureExtField1;
							string faceid = create_uuid(false);
                     					//if(data_arr[i].HasMember("SPTXXXYYSXDXBZ"))
                                                        //faceid = data_arr[i]["SPTXXXYYSXDXBZ"].GetString();
							//else
							//faceid = flagCode; 
							{
								Document d1 = Rjson::rWriteDC();
								Rjson::rAdd(d1, "RLZPXXBZ",faceid);
								Rjson::rAdd(d1, "TaskId",StringRef("hisense_face"));
								Rjson::rAdd(d1, "VideoType",1);
								Rjson::rAdd(d1, "realDeviceId",deviceid);
								captureExtField1 = Rjson::ToString(d1);
							}
							Document doc;
							doc.CopyFrom(data_arr[i],d_vi.GetAllocator());
							string jhlStr = Rjson::ToString(doc); 
							//Rjson::rAdd(dd, "code", 1);
							
							Rjson::rAdd(dd, "deviceId", virtualDevId);
							Rjson::rAdd(dd, "captureTime", snapTime);
							//Rjson::rAdd(dd, "FaceID", faceid);
							Rjson::rAdd(dd, "captureExtField1", captureExtField1);
							Rjson::rAdd(dd, "captureId", faceid);
							//Rjson::rAdd(dd,"captureExtField2",1);//VideoType
							//Rjson::rAdd(dd,"captureExtField4",jhlStr);//VideoType
							info = Rjson::ToString(dd);
                                                }
                                                else if(data_arr[i].HasMember("MJJCJLXXBZ"))
                                                {
						    string faceid = data_arr[i]["MJJCJLXXBZ"].GetString();
                                                    string direction = data_arr[i]["KMFSDM"].GetString();
                                                    string inStr = string("进");
                                                    string outStr = string("出");
                                                    int dict = 99;
                                                    if(direction == inStr) dict = 14;
                                                    else if(direction == outStr) dict = 15;
                                                    else;
                                                    string r5 = string("");
                                                    if(data_arr[i].HasMember("KMJG"))
                                                    {
                                                       r5 = data_arr[i]["KMJG"].GetString();
                                                       data_arr[i].RemoveMember("KMJG");
                                                    }
						    string captureExtField1;// = Rjson::ToString(d1);
                                                    {
 							Document d1 = Rjson::rWriteDC();
							Rjson::rAdd(d1, "Direction",dict);
							Rjson::rAdd(d1, "TaskId",StringRef("hisense_face"));
							Rjson::rAdd(d1, "VideoType",6);
							Rjson::rAdd(d1, "realDeviceId",deviceid);
							Rjson::rAdd(d1, "R5",r5);
                                                        captureExtField1 = Rjson::ToString(d1);
                                                    }
                                                     
                                                    Document data;
                                                    data.SetObject();
                                                    Document doc;
                                                    doc.CopyFrom(data_arr[i],d_vi.GetAllocator());
                                                    string jhlStr = Rjson::ToString(doc); 
                                                    {
                                                        //人脸认证是否通过标识
						        if(data_arr[i].HasMember("XM"))
							{
								auto iter = std::find(devices.begin(),devices.end(),deviceid);
								if(iter != devices.end() && g_bwsbz == 1)
								{
									string tmp = data_arr[i]["XM"].GetString();
									if(tmp != "")
									{
#if 1
										data.AddMember("WSBZ",1,d_vi.GetAllocator());//direction
#endif
										auto itd = s_wsbz_list.find(deviceid);
										if(itd != s_wsbz_list.end())
										{
											int64_t temp_time = itd->second;
											if(currt_t > temp_time) 
											{
												s_wsbz_list[deviceid] = currt_t;
											}
										}
										else
										{
											s_wsbz_list.insert(make_pair(deviceid,currt_t));
										}

									}
									else
									{
										auto itp = s_wsbz_list.find(deviceid);
										if(itp != s_wsbz_list.end())
										{
											int64_t temp_time = itp->second;
											if(currt_t - temp_time > 0 && currt_t - temp_time < 8) 
											{
												data.AddMember("WSBZ",3,d_vi.GetAllocator());//direction
											}
										}
										else
										{
											data.AddMember("WSBZ",3,d_vi.GetAllocator());//direction
											s_wsbz_list.insert(make_pair(deviceid,currt_t));
										}

									}
								}
						     	} 
                                                        
                                                    }
                                                    //data.AddMember("code",2,d_vi.GetAllocator());
                                                    data.AddMember("captureExtField1",captureExtField1,d_vi.GetAllocator());//direction
                                                    //data.AddMember("captureExtField3",dict,d_vi.GetAllocator());//direction
                                                    data.AddMember("deviceId",virtualDevId,d_vi.GetAllocator());
                                                    //data.RemoveMember("MJDBH");
                                                    data.AddMember("captureTime",snapTime,d_vi.GetAllocator());
                                                    //data.RemoveMember("CJSJ");
						    //data.AddMember("captureExtField2",6,d_vi.GetAllocator());//VideoType
                                                    data.AddMember("captureId",faceid,d_vi.GetAllocator());
                                                    //string tad = string("hisense_face"); 
                                                    //data.AddMember("captureExtField1",tad,d_vi.GetAllocator());
                                                    LOG_ERROR(MAIN_LOG,"99999999999999999 check the jhlstr size {}",jhlStr.size());
                                                    if(jhlStr.size() < 128)
                                                    {
							    data.AddMember("captureExtField2",jhlStr,d_vi.GetAllocator());//direction
                                                            //string ref;
                                                            //ref.clear();
							    //data.AddMember("captureExtField3",ref,d_vi.GetAllocator());//direction
							    //data.AddMember("captureExtField4",ref,d_vi.GetAllocator());//direction
							    //data.AddMember("captureExtField5",ref,d_vi.GetAllocator());//direction
						    }
						    else if(jhlStr.size() >= 128 && jhlStr.size() < 512)
                                                    {
                                                        string temp;
                                                        temp.swap(jhlStr);
                                                        int count = 2; 
                                                        for(int i = 2 ;i < 6 && temp.size() >= 128; i++)
                                                        {
                                                                string filed;
                                                                format_string(filed,"captureExtField%d",i);
                                                                string sub_temp = temp.substr(0,127);
                                                                temp = temp.substr(127);
 							        data.AddMember(StringRef(filed.c_str()),sub_temp,d_vi.GetAllocator());//direction
								count = i; 
                                                        } 
                                                        if(temp.size() > 0 && count + 1 <= 5)
                                                        {
                                                                string filed;
                                                                format_string(filed,"captureExtField%d",count + 1);
 							        data.AddMember(StringRef(filed.c_str()),temp,d_vi.GetAllocator());//direction
 							}
                                                    }
                                                    else
                                                    {}
                                                    //data.AddMember("captureExtField4",jhlStr,d_vi.GetAllocator());
                                                    info = Rjson::ToString(data); 
                                                }
                                                else
                                                  continue;
						byte_buffer bb;
						bb  << (int)1 << (int)1 <<info;//deviceid << snaptime;
                                                //printf("info from kafka {}",info.data());
						g_chan->send_msg(bb.data_ptr(), bb.data_size(),vmn.data(),vmn.size());
                                                g_chan_nt.wait();
			  	        }
                                        else
	  			        LOG_ERROR(MAIN_LOG, "channel is not valid");
			  	}
#else
	  			string filename;
	  			data_queue.pop(filename);
	  			//cv::Mat img = cv::imread(filename.data()); 
	  			LOG_INFO(MAIN_LOG,"start load image {}",filename.data());
	  			cv::Ptr<IplImage> img1 = cvLoadImage(filename.data());
	  			LOG_INFO(MAIN_LOG,"end load image {}",filename.data());
	  			rmmt::ShmImage simg;
	  			simg.clone(img1);
	  			rmmt::ShmVecType vmn;
	  			vmn.push_back(simg.get_shm());
	  			struct timeval tv;
	  			gettimeofday(&tv, NULL);
	  			long pts = tv.tv_sec;
	  			long long snaptime = ts_time2(pts);

	  			//int cmId = rand() % 5;
	  			string deviceid = g_device;//string("37021200001320000028");//devname[cmId];//string("00000000000000000000");
	  			//string bigurl = filename;
	  			//replace_string(filename,"small","big");
	  			Document dd = Rjson::rWriteDC();
	  			Rjson::rAdd(dd, "code", 1);
	  			Rjson::rAdd(dd, "deviceId", deviceid);
	  			Rjson::rAdd(dd, "snaptime", (int64_t)snaptime);
	  			Rjson::rAdd(dd, "taskId", m_taskId);
	  			string info = Rjson::ToString(dd);
	  			LOG_INFO(MAIN_LOG,"do check the target info {}",info.data());
	  			try{ 
	  			        byte_buffer bb;
	  			        bb << (int)6 << info;//deviceid << snaptime;
	  			        g_chan->send_msg(bb.data_ptr(), bb.data_size(),vmn.data(),vmn.size());
	  			}
	  			catch (GeneralException2& e)
	  			{
	  			        LOG_ERROR(MAIN_LOG, "on_svc_cli exception: {},{}", e.err_code(), e.err_str());
	  			}
#endif
		  }
		  catch(...)
	          {
		     usleep(100);
		     continue;
	          }
              }
	  });
}
void FaceControlBegain(const char* request, size_t req_size, bp_http_hdl_t* hdl)
{
	string response_msg;
    try{
        tbb::reader_writer_lock::scoped_lock_read lck(g_Redisrwl);
	Document reqDoc;
	reqDoc.Parse(request);
	if (reqDoc.HasParseError())
	{
		LOG_ERROR(MAIN_LOG, "OnExecuteControlBegin reqStr parse error");
		//response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,HasParseError").add("node_ip", node_ip).json();
		response_msg = "{\"errCode\":\"22001\",\"retFlag\":\"failed,HasParseError\",\"message\":\"failed\"}";
			throw GeneralException2(22001,"failed,HasParseError");;
	}
	if (!reqDoc.HasMember("queryParam"))
	{
		LOG_ERROR(MAIN_LOG, "OnExecuteControlBegin 'queryParam' not found");
		//response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,queryParam not found").add("node_ip", node_ip).json();
		response_msg = "{\"errCode\":\"22002\",\"retFlag\":\"failed,queryParam not found\",\"message\":\"failed\"}";
			throw GeneralException2(22002,"failed,queryParam not found");;
	}
	Value& queryParamV = reqDoc["queryParam"];
	Document requestDoc;
	requestDoc.CopyFrom(queryParamV,reqDoc.GetAllocator());
	string taskId = requestDoc["taskId"].GetString();
	if (!requestDoc.HasMember("dbNo"))
	{
		response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,dbNo not found").add("node_ip", node_ip).json();
	//response_msg = "{\"errCode\":\"220013\",\"retFlag\":\"failed,taskId not found\",\"message\":\"failed\"}";
		throw GeneralException2(22002,"failed,dbNo not found");;
	}
        int dbNo = requestDoc["dbNo"].GetInt();
        if(!(dbNo == 1 || dbNo == 2))
        {
	    throw GeneralException2(22002,"failed,dbNo not correct");;
        }
        /////////////////////////////////////////////
		string where_delete;
		format_string(where_delete, "TaskId = '%s'",taskId.data());
		pIseApi.delete_img_rec_ws(ise_dbname[5], where_delete);
		LOG_INFO(MAIN_LOG,"auto alarm cantel delete,check the ctrldb---{}",taskId);
		pIseApi.delete_img_rec_ws(ise_dbname[1], where_delete);
		LOG_INFO(MAIN_LOG,"auto alarm cantel delete,check the ctrldb--1---{}",taskId);
		control_ct_ptr curCol = make_shared<RedisBidId>(taskId,2);
		control_queue.push(curCol);             
        ////////////////////////////////////////////
	//string ControlId;
	//format_string(ControlId,"%s_N%d",taskId.data(),1);
	string ScheduleId;
	format_string(ScheduleId,"%s_N%d",taskId.data(),1);
	std::string beginTime;
	if (queryParamV.HasMember("beginTime"))
	{
		beginTime = queryParamV["beginTime"].GetString();
		LOG_DEBUG(MAIN_LOG, "OnArchivesRetreive beginTime:{}", beginTime.data());
	}
	std::string endTime;
	if (queryParamV.HasMember("endTime"))
	{
		endTime = queryParamV["endTime"].GetString();
		LOG_DEBUG(MAIN_LOG, "OnArchivesRetreive endTime:{}", endTime.data());
	}
	//trim(beginTime, ' ');
	//trim(beginTime, '-');
	//trim(beginTime, ':');
	long endpts = time_ts(endTime.data());
	long temppts = time_ts(beginTime.data());
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t curpts = tv.tv_sec;
    int64_t subpts = temppts - curpts;
    {
	  //在这里增加redis定时布控规则
		redisContext* p_RedisSyncConnector = NULL;
		p_RedisSyncConnector = redisConnectWithTimeout(redisIp.c_str(), redisPort, g_RedisTimeOut);
		if (p_RedisSyncConnector->err)
		{
			LOG_INFO(START_LOG,"connectRedisThread connect to redisServer fail---{}",p_RedisSyncConnector->errstr);
			throw GeneralException2(ERROR_MISS_PARAM,p_RedisSyncConnector->errstr);
		}
		string redisCommandStr;
		format_string(redisCommandStr, "set %s %s", ScheduleId.data(),request);
		redisReply *reply = (redisReply*)redisCommand(p_RedisSyncConnector,"select %d",redisdbNo);
		freeReplyObject(reply);
		reply = NULL;
		reply = (redisReply*)redisCommand(p_RedisSyncConnector,redisCommandStr.c_str());
		if (reply == NULL)
		{
			response_msg = "{\"errCode\":\"22005\",\"retFlag\":\"failed,redis realControl\",\"message\":\"failed\"}";
				LOG_ERROR(MAIN_LOG, "OnFaceControlBegain hset FaceControlTask:SessionHash failed realControl:{}", taskId.data());
				throw GeneralException2(22005,"OnFaceControlBegain hset FaceControlTask:SessionHash failed realControl");;
		}
#if 0
		format_string(redisCommandStr, "set %s %s", ControlId.data(), "Scheduledtasks");
		redisReply *reply = (redisReply*)redisCommand(g_RedisSyncConnector,"select %d",redisdbNo);
		freeReplyObject(reply);
		reply = NULL;
		//reply = (redisReply*)redisCommand(g_RedisSyncConnector,redisCommandStr.c_str());
		if (reply != NULL)
		{
			response_msg = "{\"errCode\":\"22003\",\"retFlag\":\"failed,redis error\",\"message\":\"failed\"}";
			LOG_ERROR(MAIN_LOG, "OnFaceControlBegain hset FaceControlTask:SessionHash failed taskId:{}", taskId.data());
			throw GeneralException2(22003,"OnFaceControlBegain hset FaceControlTask:SessionHash failed");;
		}
		else
		{
			freeReplyObject(reply);
			reply = NULL;
			format_string(redisCommandStr, "expire %s %lld", ControlId.data(),subpts);
			reply = (redisReply*)redisCommand(g_RedisSyncConnector,"select %d",redisdbNo);
			freeReplyObject(reply);
			reply = NULL;
			reply = (redisReply*)redisCommand(g_RedisSyncConnector,redisCommandStr.c_str());
			if (reply == NULL)
		    {
				//bConnect2Redis = false;
				//response_msg = jsonobject().add("errCode", ERROR_OK).add("retFlag", "failed,redis scheduled").add("taskId", taskId).json();
				response_msg = "{\"errCode\":\"22004\",\"retFlag\":\"failed,redis scheduled\",\"message\":\"failed\"}";
				LOG_ERROR(MAIN_LOG, "OnFaceControlBegain hset FaceControlTask:SessionHash failed scheduled:{}", taskId.data());
			    throw GeneralException2(22004,"OnFaceControlBegain hset FaceControlTask:SessionHash failed scheduled");;
			}
			else
			{
			freeReplyObject(reply);
			reply = NULL;
				format_string(redisCommandStr,"subscribe __keyspace@32__:%s", ControlId.data());
                                RedisBidId *bindStr = new RedisBidId(ControlId,1);
				int rep = redisAsyncCommand(g_RedisConnector,subCallback,bindStr,redisCommandStr.c_str());

				if (rep != 0)
			        {
					LOG_ERROR(MAIN_LOG, "face control subscribe not correct:{}", taskId.data());
				}
			}
		}
#endif
		freeReplyObject(reply);
		reply = NULL;
		redisFree(p_RedisSyncConnector);
		p_RedisSyncConnector = NULL;
	}
	if(subpts < 60) //subpts = 10L;
	{
		control_ct_ptr curCol = make_shared<RedisBidId>(taskId,1);
		LOG_INFO(MAIN_LOG, "start push ScheduleId:{}",ScheduleId.data());
        control_queue.push(curCol);             
		LOG_INFO(MAIN_LOG, "end   push ScheduleId:{}",ScheduleId.data());
	}
	else
	{
		LOG_INFO(MAIN_LOG,"insert into db ...");
		char *errMsg = NULL;
		char sqlCmd[256];
		memset(sqlCmd, 0, sizeof(sqlCmd));
		sprintf(sqlCmd, "insert into ControlInfo (TaskID,BeginTime,EndTime) values ('%s',%lld,%lld);",taskId.data(),temppts,endpts);
		if (sqlite3_exec(sql, sqlCmd, NULL, NULL, &errMsg) != SQLITE_OK)
		{   
			LOG_ERROR(MAIN_LOG,"insert failed : {}",errMsg);
			throw GeneralException2(-66,errMsg);
		} 
		if (errMsg != NULL)
		{
			sqlite3_free(errMsg);
			errMsg = nullptr;
		}
	}
	{
		Document respDoc;
		Document::AllocatorType& allocator = respDoc.GetAllocator();
		respDoc.SetObject();

		respDoc.AddMember("code", "200", allocator);
		respDoc.AddMember("message", "", allocator);

		Value data(kObjectType);
		Value taskIdV(taskId.c_str(), allocator);
		data.AddMember("taskId", taskIdV, allocator);
		Value dataV(kArrayType);
		dataV.PushBack(data, allocator);
		respDoc.AddMember("data", dataV, allocator);
		respDoc.AddMember("success", true, allocator);
		response_msg  = Rjson::ToString(respDoc); 
		write_resp(hdl, response_msg.data(), response_msg.size());
		return;
	}
    }
    catch (GeneralException2& e)
    {
	    response_msg = jsonobject().add("errCode", e.err_code()).add("retFlag", e.err_str()).add("node_ip", node_ip).json();
    }
    catch(...)
    {
	    response_msg = jsonobject().add("errCode",99).add("retFlag","unhandled error").add("node_ip", node_ip).json();
    }
    write_resp(hdl, response_msg.data(), response_msg.size());          
}
#if 1
void FaceControlBegain1(const char* request)
{
        tbb::reader_writer_lock::scoped_lock_read lck(g_Redisrwl);
	LOG_INFO(MAIN_LOG,"OnFaceControlBegain {}", request);
	if (request == nullptr)
	{
		LOG_ERROR(MAIN_LOG, "{}","OnFaceControlBegain request null");
		return;
	}
	std::string response_msg;
	
	std::string taskId;//布控id
        int         dbNo;
	//std::string sessId;//结构化任务id
	std::string controlReason;
	std::string departmentId;
	std::string keyPlaceId;
	std::string keyUserId;
	std::string dataSource;
	try
	{
		Document reqDoc;
		reqDoc.Parse(request);
		if (reqDoc.HasParseError())
		{
			LOG_ERROR(MAIN_LOG, "OnExecuteControlBegin reqStr parse error");
			response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,HasParseError").add("node_ip", node_ip).json();
		//response_msg = "{\"errCode\":\"22001\",\"retFlag\":\"failed,HasParseError\",\"message\":\"failed\"}";
			return;
		}
		if (!reqDoc.HasMember("queryParam"))
		{
			LOG_ERROR(MAIN_LOG, "OnExecuteControlBegin 'queryParam' not found");
			response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,queryParam not found").add("node_ip", node_ip).json();
		//response_msg = "{\"errCode\":\"22002\",\"retFlag\":\"failed,HasParseError\",\"message\":\"failed\"}";
			return;
		}
                Value& queryParamV = reqDoc["queryParam"];
		Document requestDoc;
		//requestDoc.Parse(request);
		requestDoc.CopyFrom(queryParamV,reqDoc.GetAllocator());
		//if (requestDoc.HasParseError())
		//{
		//	response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,HasParseError").add("node_ip", node_ip).json();
		//	goto result;
		//}
		if (!requestDoc.HasMember("taskId"))
		{
			response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,taskId not found").add("node_ip", node_ip).json();
		//response_msg = "{\"errCode\":\"220013\",\"retFlag\":\"failed,taskId not found\",\"message\":\"failed\"}";
			return;
		}
		//if (!requestDoc.HasMember("sessId"))
		//{
		//	response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,sessId not found").add("node_ip", node_ip).json();
		//	goto result;
		//}
		
		taskId = requestDoc["taskId"].GetString();
		dbNo = requestDoc["dbNo"].GetInt();
		//sessId = requestDoc["sessId"].GetString();
		std::string beginTime;
		if (queryParamV.HasMember("beginTime"))
		{
			beginTime = queryParamV["beginTime"].GetString();
			LOG_DEBUG(MAIN_LOG, "OnArchivesRetreive beginTime:{}", beginTime.data());
		}

		std::string endTime;
		if (queryParamV.HasMember("endTime"))
		{
			endTime = queryParamV["endTime"].GetString();
			LOG_DEBUG(MAIN_LOG, "OnArchivesRetreive endTime:{}", endTime.data());
		}
		//trim(endTime, ' ');
		//	trim(endTime, '-');
		//	trim(endTime, ':');
			long temppts = time_ts(endTime.data());
		struct timeval tv;
		gettimeofday(&tv, NULL);
		int64_t curpts = tv.tv_sec;
		int64_t subpts = temppts - curpts;
		if(subpts < 10) 
                { 
			response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,endtime not correct").add("node_ip", node_ip).json();
			return;
                }

		if (requestDoc.HasMember("controlReason"))
			controlReason = requestDoc["controlReason"].GetString();
		if (requestDoc.HasMember("departmentId"))
			departmentId = requestDoc["departmentId"].GetString();
		if (requestDoc.HasMember("keyPlaceId"))
			keyPlaceId = requestDoc["keyPlaceId"].GetString();
		if (requestDoc.HasMember("keyUserId"))
			keyUserId = requestDoc["keyUserId"].GetString();
		if (requestDoc.HasMember("dataSource"))
			dataSource = std::to_string(requestDoc["dataSource"].GetInt());
		
		Document alarm2redisDoc;
		alarm2redisDoc.SetObject();
		Document::AllocatorType& allocator = alarm2redisDoc.GetAllocator();
		
		Value controlReasonV(controlReason.c_str(), allocator);
		//Value taskIdV(taskId.c_str(), allocator);
		Value dataSourceV(dataSource.c_str(), allocator);
		Value keyUserIdV(keyUserId.c_str(), allocator);
		Value keyPlaceIdV(keyPlaceId.c_str(), allocator);
		Value departmentIdV(departmentId.c_str(), allocator);
		
		alarm2redisDoc.AddMember("controlReason", controlReasonV, allocator);
		alarm2redisDoc.AddMember("taskId", taskId, allocator);
		alarm2redisDoc.AddMember("dbNo",dbNo,allocator);
		alarm2redisDoc.AddMember("dataSource", dataSourceV, allocator);
		alarm2redisDoc.AddMember("keyUserId", keyUserIdV, allocator);
		alarm2redisDoc.AddMember("keyPlaceId", keyPlaceIdV, allocator);
		alarm2redisDoc.AddMember("departmentId", departmentIdV, allocator);
		alarm2redisDoc.AddMember("outputType", 3, allocator);
			
                Value deviceIdVec(kArrayType);
		if (requestDoc.HasMember("deviceId"))
		{
			const Value& deviceIdV = requestDoc["deviceId"];
			if (deviceIdV.HasMember("camera"))
			{
				const Value& cameraV = deviceIdV["camera"];
				for (int i = 0; i < cameraV.Size(); i++)
				{
                                        string cam = cameraV[i].GetString();
                                        Value camera(cam.c_str(),allocator);     
                                        deviceIdVec.PushBack(camera,allocator);  
				}
			}
			if (deviceIdV.HasMember("crossing"))
			{
				const Value& crossingV = deviceIdV["crossing"];
				for (int i = 0; i < crossingV.Size(); i++)
				{
                                        string cro = crossingV[i].GetString();
                                        Value crossing(cro.c_str(),allocator);     
                                        deviceIdVec.PushBack(crossing,allocator);  
				}
			}
			if (deviceIdV.HasMember("social"))
			{
				const Value& socialV = deviceIdV["social"];
				for (int i = 0; i < socialV.Size(); i++)
				{
                                        string soc = socialV[i].GetString();
                                        Value social(soc.c_str(),allocator);     
                                        deviceIdVec.PushBack(social,allocator);  
				}
			}

			//if(deviceIdVec.Empty())
			//{
                        //                Value empty("empty",allocator);     
                        //                deviceIdVec.PushBack(empty,allocator);  
			//}
		}
		//else
		//{
		//	Value empty("empty",allocator);     
		//	deviceIdVec.PushBack(empty,allocator);  
		//}
		if(deviceIdVec.Empty())
                {
 			g_globalCtlNum.fetch_add(1);
			int gnum = g_globalCtlNum.load(std::memory_order_seq_cst);
			redisReply *reply = (redisReply*)redisCommand(g_RedisSyncConnector,"select %d",redisdbNo);
				if(reply == NULL)
				{
					bConnect2Redis = false;
						LOG_ERROR(MAIN_LOG,"redis set hisense control false");
						response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "catch an redis false").add("node_ip", node_ip).json();
						return;
				}
				else
				{
					freeReplyObject(reply);
						reply = NULL;
				}
			 reply = (redisReply*)redisCommand(g_RedisSyncConnector,"set %s %d","hisense-global-face-control",gnum);
                        if(reply == NULL)
                        {
				bConnect2Redis = false;
				LOG_ERROR(MAIN_LOG,"redis set hisense control false");
				response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "catch an redis false").add("node_ip", node_ip).json();
				return;
                        }
                        else
                        {
					   freeReplyObject(reply);
					   reply = NULL;
                        }
                    
                    
                }
		alarm2redisDoc.AddMember("deviceId",deviceIdVec, allocator);

		if (!requestDoc.HasMember("faceImg"))
		{
			LOG_ERROR(MAIN_LOG, "On3ExecuteControlBegin 'faceUrl' not found");
			response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,HasParseError").add("node_ip", node_ip).json();
				//response_msg = "{\"errCode\":\"22004\",\"retFlag\":\"faceUrl' not found\",\"message\":\"failed\"}";
				return;
		}
		Value& faceImgV = requestDoc["faceImg"];
		if (faceImgV.Size() <= 0)
		{
			LOG_ERROR(MAIN_LOG, "On3ExecuteControlBegin 'faceImg' is empty");
			response_msg = jsonobject().add("errCode", ERROR_INVALID_PARAM).add("retFlag", "failed,img not found").add("node_ip", node_ip).json();
				//response_msg = "{\"errCode\":\"22005\",\"retFlag\":\"faceImg' is empty\",\"message\":\"failed\"}";
				return;
		}
                std::string alarm2redisStr;
		for (int i = 0; i < faceImgV.Size(); i++)
		{
			std::string imgUrlPsn;
			imgUrlPsn = faceImgV[i].GetString();
			if (imgUrlPsn.empty())
				continue;

			std::string imgData;
			std::string linkType = imgUrlPsn.substr(0, 3);
			if (linkType == "ftp")
			{
				if (ftp_download_image(imgUrlPsn, &imgData))
				{
					LOG_ERROR(MAIN_LOG, "On3ExecuteControlBegin ftp download false url:{}", imgUrlPsn.data());
					continue;
				}
			}
			else if (linkType == "htt")
			{
				if (http_download_image(imgUrlPsn, &imgData))
				{
					LOG_ERROR(MAIN_LOG, "On3ExecuteControlBegin http download false url:{}", imgUrlPsn.data());
					continue;
				}
			}
			else
			{
				LOG_ERROR(MAIN_LOG, "On3ExecuteControlBegin imgUrl error false url:{}", imgUrlPsn.data());
				continue;
			}
			cv::Mat img(cv::imdecode(cv::Mat(1, imgData.size(),CV_8UC3,(unsigned char*)imgData.data()),CV_LOAD_IMAGE_COLOR));
			rmmt::ShmImage simg;
			simg.clone(img);
			if (simg.checkImgValid() != 0)
			{
				LOG_ERROR(MAIN_LOG, "warning!!!!!!!!!!!!!!!!!!control img is not valid");
				continue;
			}
			rmmt::ShmVecType vmn;
			vmn.push_back(simg.get_shm());
			//Value imr(imgUrlPsn.data(),allocator); 
			alarm2redisDoc.AddMember("iconUrl",imgUrlPsn,allocator);
			string uuid = create_uuid(false);
			//Value uid(uuid.data(),allocator);
			alarm2redisDoc.AddMember("iconId",uuid,allocator);
			alarm2redisStr = Rjson::ToString(alarm2redisDoc);
			byte_buffer bb;
			bb  << (int)1 << (int)5 << alarm2redisStr;//
			g_chan->send_msg(bb.data_ptr(), bb.data_size(),vmn.data(),vmn.size());
		}
		//redis会覆盖已有的键值对，不用查重
		string redisCommandStr;
		format_string(redisCommandStr, "set %s %s", taskId.data(), alarm2redisStr.data());
		redisReply *reply = (redisReply*)redisCommand(g_RedisSyncConnector,redisCommandStr.c_str());
		if (reply == NULL)
		{
			//bConnect2Redis = false;
			response_msg = jsonobject().add("errCode", ERROR_OK).add("retFlag", "failed,redis error").add("taskId", taskId).add("node_ip", node_ip).json();
		        //response_msg = "{\"errCode\":\"22006\",\"retFlag\":\"failed,redis error\",\"message\":\"failed\"}";
			LOG_ERROR(MAIN_LOG, "OnFaceControlBegain hset FaceControlTask:SessionHash failed taskId:{}", taskId.data());
			//string img64 = data_arr[i]["TP"].GetString();
			return;
		}
                else
                {
		freeReplyObject(reply);
		reply = NULL;
			format_string(redisCommandStr, "expire %s %lld", taskId.data(),subpts);
			reply = (redisReply*)redisCommand(g_RedisSyncConnector,redisCommandStr.c_str());
			if (reply == NULL)
			{
					//bConnect2Redis = false;
						//response_msg = jsonobject().add("errCode", ERROR_OK).add("retFlag", "failed,redis scheduled").add("taskId", taskId).json();
						response_msg = "{\"errCode\":\"22004\",\"retFlag\":\"failed,redis scheduled\",\"message\":\"failed\"}";
						LOG_ERROR(MAIN_LOG, "OnFaceControlBegain hset FaceControlTask:SessionHash failed scheduled:{}", taskId.data());
						return;
			}
                        else
                        {
				format_string(redisCommandStr,"subscribe __keyspace@32__:%s", taskId.data());
                                RedisBidId *bindStr = new RedisBidId(taskId,2);
				int rep = redisAsyncCommand(g_RedisConnector,subCallback,bindStr,redisCommandStr.c_str());

				if (rep != 0)
			        {
					LOG_ERROR(MAIN_LOG, "face control subscribe not correct:{}", taskId.data());
						return;
				}
                        }
                }
		
		freeReplyObject(reply);
		reply = NULL;
		//{
		//	tbb::reader_writer_lock::scoped_lock lck(s_face_ctrl_lock);
		//	s_pvc_face_ctrl_t.insert(make_pair(sessId, taskId));
		//}
		response_msg = jsonobject().add("errCode", ERROR_OK).add("retFlag", "success").add("taskId", taskId).add("node_ip", node_ip).json();
		//response_msg = "{\"errCode\":\"0\",\"retFlag\":\"success\",\"message\":\"correct\"}";
	}
	catch (GeneralException2& e)
	{
		response_msg = jsonobject().add("errCode", e.err_code()).add("retFlag", e.err_str()).add("node_ip", node_ip).json();
	}
	
//`result:	return;//write_resp(hdl, response_msg.data(), response_msg.size());
}
#endif
void FaceControlCancel(const char* request, size_t req_size, bp_http_hdl_t* hdl)
{
	LOG_INFO(ANAL_LOG,"OnFaceGy_PushRecong {}", request);
	if (request == nullptr || req_size == 0)
	{
		LOG_ERROR(ANAL_LOG,"{}", "OnFaceGy_PushRecong request null");
		return;
	}
	string taskId;
	string response_msg;
	try
	{
		//jsonparser parser(request);
		Document requestDoc;
		requestDoc.Parse(request);
		//if (!parser.valid())
		if (requestDoc.HasParseError())
		{
			throw GeneralException2(ERROR_INVALID_PARAM, "the param is not valid!");
		}
		if (!requestDoc.HasMember("queryParam"))
                //if (!parser.isMember("queryParam"))
		//if (!parser.get("queryParam", taskId))
		{
			throw GeneralException2(ERROR_MISS_PARAM, "error missing params!");
		}
		Value& queryParamV = requestDoc["queryParam"];
		Document rDoc;
		rDoc.CopyFrom(queryParamV,requestDoc.GetAllocator());
		Value& taskIdV = rDoc["taskId"];
		redisContext* p_RedisSyncConnector = NULL;
		p_RedisSyncConnector = redisConnectWithTimeout(redisIp.c_str(), redisPort, g_RedisTimeOut);
		if (p_RedisSyncConnector->err)
		{
			LOG_INFO(START_LOG,"connectRedisThread connect to redisServer fail---{}",p_RedisSyncConnector->errstr);
			throw GeneralException2(ERROR_MISS_PARAM,p_RedisSyncConnector->errstr);
		}
		std::string redisCommandStr;
		redisReply *reply = (redisReply*)redisCommand(p_RedisSyncConnector,"select %d",redisdbNo);
		freeReplyObject(reply);
		reply = NULL;
		for (int i = 0; i < taskIdV.Size(); i++)
                {
                        string taskId = taskIdV[i].GetString(); 
			format_string(redisCommandStr, "del %s", taskId.data());
			reply = (redisReply*)redisCommand(p_RedisSyncConnector, redisCommandStr.c_str());
			if (reply == NULL)
			{
				bConnect2Redis = false;
				LOG_ERROR(ANAL_LOG,"OnFaceControlCancel hdel FaceControlTask:SessionHash failed taskId:{}", taskId.data());
			}
			else
			{
				freeReplyObject(reply);
				reply = NULL;
			}
                        
                }
		//taskId = rDoc["taskId"].GetString();
		///////////////////////2020.3.18 madongsheng/////////
		//string where_delete;
		//format_string(where_delete, "TaskId = '%s'", taskId.data());
		//pIseApi.delete_img_rec_ws(ise_dbname[5], where_delete);
		//INFO("alarm cantel delete,check the ctrldb---{}", taskId);
		redisFree(p_RedisSyncConnector);
		p_RedisSyncConnector = NULL;
		//////////////////////////////////////////
		response_msg = jsonobject().add("code",200).add("message", "success").add("node_ip", node_ip).json();
	}
	catch (GeneralException2& e)
	{
		response_msg = jsonobject().add("code", e.err_code()).add("message", e.err_str()).add("node_ip", node_ip).json();
	}
	write_resp(hdl, response_msg.data(), response_msg.size());
     
}
