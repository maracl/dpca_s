#include <regex>
#include "Util.h"
#include "spdlog_helper.h"
#include "ini_utils.hpp"
#include "rmmt_wrap2.h"
#include "ws_stream_sender.h"
#include "rmmt_svc_sessions.h"
#include "StreamCmdApis.h"
#include "KafkaClientAPI.h"
#include "activemq_producer.h"
#include <activemq/activemq/library/ActiveMQCPP.h>
#include "download_image.h"
#include "kafka_client.h"
#include "../include/hiredis/hiredis.h"
#include "../include/hiredis/async.h"
#include "../include/hiredis/adapters/libevent.h"
#include <libevent/include/event2/event.h>
#include "sqlite3.h"
#include <tbb/reader_writer_lock.h>
#include "http_his_client.h"

#ifdef WIN32
#include <cv2.4.13all.hpp>
#include <windows.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <iterator>
#include <stdarg.h>
#include <sys/resource.h>
#endif

#ifdef USE_ZOOKEEPER
#include "zookeeper/zookeeper.h"
#include "zookeeper/zookeeper.jute.h"
#endif


//sqlite3
sqlite3 *sql;

int http_port = 8788;
int g_sim; 
int g_bwsbz; 
std::string face_url = "ip://127.0.0.1:4006";
string g_VidHeader;
int    g_VidpNum;
string g_VidStr;
bool g_mainNode = false;
IseApi  pIseApi;
extern void add_algopkg(const char* name, const char* rmmt_url);
std::string glb_rmmt_server = "ip://127.0.0.1:7700";
std::string vst_server_addr = "ip://0.0.0.0:3005";
string dpca_server_addr = "ip://127.0.0.1:5205";

bool kafka_check = true;
int ws_sport = 8084;
std::string log_level = "debug";	 //日志级别
int log_max_size = 30 * 1024 * 1024; //单个日志文件最大默认30M
int log_max_count = 15;				 //日志最大个数,默认为15个

std::string extr_ip,extr_ip1;
int extr_port,extr_port1;
std::string ise_ip;
int ise_port;
std::vector<string> ise_dbname;
string ise_null,ise_veh,ise_bik,ise_ped,ise_fac,ise_crl;
//////////////////////////////////////////////////////////////////////////
std::string alarm_broker;
std::string alarm_username;
std::string alarm_password;
std::string alarm_topic;
AmqClient pAlCt;//聚好联mq
std::string active_broker;
std::string active_username;
std::string active_password;
std::string active_topic;
AmqClient pAcCt;//尾随mq
std::string facl_broker;
std::string facl_broker1;
std::string facl_username;
std::string facl_password;
std::string facl_topic;
AmqClient pFaCt;//布控告警mq
AmqClient pFaCt1;//布控告警mq

std::string face_servers;
std::string face_topic;
KafkaClient pFacCt;//人脸
std::string clus_servers;
std::string clus_topic;
KafkaClient pClust;//人脸
std::string follow_servers;
std::string follow_topic;
KafkaClient pFowst;//人脸

std::string kafka_broker;
std::string kafka_topic;
std::string kafka_groupid;
KafkaConsumerClient  m_consumer_;

ExtrApi pExtrApi_Prd;
ExtrApi pExtrApi_Bik;
//布控报警阈值 /100 为相似度
int g_fcthdValue;
/////////////////redis relate////////////////////
bool bUseRedis;
int  redisdbNo;
std::string redisIp;
int redisPort;
std::string redisUserName;
std::string redisPassWord;
timeval g_RedisTimeOut = { 3, 0 };
bool bConnect2Redis = false;
bool bConnectWsRedis = false;
redisContext* g_RedisSyncConnector;
//redisContext* g_RedisSync2Connect;
redisContext* g_RedisWsbzConnector;
redisAsyncContext* g_RedisConnector;
tbb::reader_writer_lock g_Redisrwl;
struct event_base *g_base;// = event_base_new();//新建一个libevent事件处理
//redisAsyncContext* g_RedisAsyncConnector;
//control
atomic<int> g_globalCtlNum(0);//全局布控个数
/////////////////云从
std::string appId;
std::string appSecret;
std::string cloudwalkIp;
int cloudwalkPort;
std::string cloudwalkServer;
std::string clusterName;
std::string tokenStr;//token 字段
std::string mndIp;
int mndPort;
int g_lefttopx;
int g_lefttopy;
int g_rightbottomx;
int g_rightbottomy;
std::vector<std::string> devices;
std::vector<std::string> parse_dev_list(const std::string& in, const std::string& delim)
{
    std::regex re{ delim };
    // 调用 std::vector::vector (InputIterator first, InputIterator last,const allocator_type& alloc = allocator_type())
    //     // 构造函数,完成字符串分割
    return std::vector<std::string> {
	    std::sregex_token_iterator(in.begin(), in.end(), re, -1),
		    std::sregex_token_iterator()
    };
}

std::string ftp_url;
std::string ftp_username;
std::string ftp_password;
int disknum;//硬盘数量
std::vector<int> diskbath;//硬盘容量（G）
std::vector<int> parse_id_list(std::string src)
{
	std::vector<int> vids;
	for (size_t p1 = 0;;) {
		size_t p2 = src.find(',', p1);
		int v = 0;
		if (p2 != std::string::npos) {
			src[p2] = 0;
			if (sscanf(src.data() + p1, "%d", &v) > 0) {
				//_checkValidGpuid(v);
				vids.push_back(v);
			}   
			p1 = p2 + 1;
		}   
		else {
			if (sscanf(src.data() + p1, "%d", &v) > 0) {
				//_checkValidGpuid(v);
				vids.push_back(v);
			}   
			break;
		}   
	}//   
	return vids;
}
//设置连接回调函数
void connectCallback(const redisAsyncContext *c, int status) {
	if (status != REDIS_OK) {
		printf("in connectCallback Error: %s\n", c->errstr);
		return;
	}
	printf("Connected...\n");
}

//              //设置断开连接回调函数
void disconnectCallback(const redisAsyncContext *c, int status) {
        //event_base_loopexit(g_base,NULL);
				redisAsyncFree(g_RedisConnector);	
                        	g_RedisConnector = redisAsyncConnect(redisIp.c_str(), redisPort);
	if (status != REDIS_OK) {
		printf("in disconnectCallback  Error: %s\n", c->errstr);
		return;
	}
	printf("Disconnected...\n");
}
static void update_token_thread_cloud(void)
{
	while (true)
	{
            try{
		std::string param = "{\"appId\":\"" + appId + "\",\"appSecret\":\"" + appSecret + "\"}";
		std::string pathUrl = "http://" + cloudwalkServer + "/ocean/auth/token";
		std::string reply;

		if (HttpClientPostCloudWalk(pathUrl.c_str(), param.c_str(), "", reply))
		{
			LOG_INFO(ANAL_LOG,"cloud walk reply is : {}", reply);
			Document replyDoc;
			replyDoc.Parse(reply);

			if (replyDoc.HasMember("data"))
			{
				Value& dataV = replyDoc["data"];
				if (dataV.IsObject())
				{
					tokenStr = dataV["refreshToken"].GetString();
				}
			}

			LOG_INFO(ANAL_LOG,"update_token_thread token:{}", tokenStr);
		}
		else
		{
			LOG_INFO(ANAL_LOG,"{}", "update_token_thread /ocean/auth/token failed");
		}
              }
              catch(GeneralException2& e)
              {
		    LOG_INFO(ANAL_LOG,"cloud get token error:code: {},msg:{} !", e.err_code(), e.err_str());
              }
              catch(...)
              {
		    LOG_INFO(ANAL_LOG,"cloud get token unhandled error!!");
              }
		std::this_thread::sleep_for(std::chrono::minutes(28));	
	}
}


int db_init()
{
        //CamListCache.clear();
        sql = NULL;
        char exeLocation[1024] = {0};
        //int result = readlink("/proc/self/exe", exeLocation, sizeof(exeLocation));
        int result = readlink("/proc/self/exe", exeLocation,1024 - 1);
		if(result < 0 || (result >= 1023)) 
		{
			printf("exe path check error the path size %d\n!",result);
			return -1;
		}
        //exeLocation[strlen(exeLocation)] = '\0';
        exeLocation[result] = '\0';
        char *t_ptr = NULL;
        t_ptr = strrchr(exeLocation, '/');
        if (t_ptr == NULL)
                return -1;
        *t_ptr = 0;
        strcat(exeLocation, "/control.db");
        if (sqlite3_open_v2(exeLocation,&sql, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE,NULL) != SQLITE_OK)
        {
				LOG_ERROR(START_LOG, "open database file failed!");
                return -1;
        }
        //char *errMsg = NULL;
        //if (sqlite3_exec(sql, "SELECT * FROM TaskList;", sqlite_exec_callback, NULL, &errMsg) != SQLITE_OK)
        //{
        //        LOG(ERROR) << "sqlite query error : " << errMsg;
        //        return -1;
        //}
        //LOG(INFO) << "database init sucess!";
        //if (errMsg != NULL)
        //{
        //        sqlite3_free(errMsg);
        //        errMsg = nullptr;
        //}
        return 0;
}



#ifdef USE_ZOOKEEPER
zhandle_t *zh = nullptr;
std::string host;
std::string path;
int interval = 5000;
#endif


int load_config(const char* fpath,std::string& errmsg)
{
	using namespace ltmit;
	CIniParser par;
	if (par.parseFile(fpath)) {
		errmsg = "ini file parse failed!";
		return -1;
	}
	try{
		ws_sport = par.cxx_getint("server", "ws_port");
		vst_server_addr = par.cxx_get_value("server", "vst_addr");
		face_url = par.cxx_get_value("server", "face_url");
                http_port = par.cxx_getint("server", "http_port");
		
		log_level = par.cxx_get_value("log", "level");
		log_max_size = par.cxx_getint("log", "max_size_mb") * 1048576;
		log_max_count = par.cxx_getint("log", "max_count");

		//防控报警推送配置
		alarm_broker = par.cxx_get_value("alarmmq", "broker");
		alarm_username = par.cxx_get_value("alarmmq", "username");
		alarm_password = par.cxx_get_value("alarmmq", "password");
		alarm_topic = par.cxx_get_value("alarmmq", "topic");
		active_broker = par.cxx_get_value("activemq", "broker");
		active_username = par.cxx_get_value("activemq", "username");
		active_password = par.cxx_get_value("activemq", "password");
		active_topic = par.cxx_get_value("activemq", "topic");

		facl_broker = par.cxx_get_value("faclmq", "broker");
		facl_broker1 = par.cxx_get_value("faclmq", "broker1");
		facl_username = par.cxx_get_value("faclmq", "username");
		facl_password = par.cxx_get_value("faclmq", "password");
		facl_topic = par.cxx_get_value("faclmq", "topic");

		extr_ip = par.cxx_get_value("extr", "ip");
		extr_port = par.cxx_getint("extr", "port");
		extr_ip1 = par.cxx_get_value("extr1", "ip");
		extr_port1 = par.cxx_getint("extr1", "port");

		ise_ip = par.cxx_get_value("ise", "ip");
		ise_port = par.cxx_getint("ise", "port");
		ise_dbname.reserve(6);
		ise_null = string("");
		ise_veh = par.cxx_get_value("ise", "vehicle");
		ise_bik = par.cxx_get_value("ise", "bike");
		ise_ped = par.cxx_get_value("ise", "pedestrain");
		ise_fac = par.cxx_get_value("ise", "face");
		ise_crl = par.cxx_get_value("ise", "ctrl");
		ise_dbname.push_back(ise_null);
		ise_dbname.push_back(ise_veh); 
		ise_dbname.push_back(ise_bik); 
		ise_dbname.push_back(ise_ped); 
		ise_dbname.push_back(ise_fac); 
		ise_dbname.push_back(ise_crl);

		//ftp内容
		ftp_url = par.cxx_get_value("ftp", "url");
		ftp_username = par.cxx_get_value("ftp", "username");
		ftp_password = par.cxx_get_value("ftp", "password");
		disknum = par.cxx_getint("ftp", "disk_num");
		diskbath = parse_id_list(par.cxx_get_value("ftp", "disk_bath"));

		//kafka
		face_servers = par.cxx_get_value("kafka", "servers");
		face_topic = par.cxx_get_value("kafka", "topic");
		clus_servers = par.cxx_get_value("kafka2", "servers");
		clus_topic = par.cxx_get_value("kafka2", "topic");
		follow_servers = par.cxx_get_value("kafka3", "servers");
		follow_topic = par.cxx_get_value("kafka3", "topic");
                /////////////////////////////////////////////////////////////
		kafka_broker = par.cxx_get_value("kafka1", "broker");
		kafka_topic = par.cxx_get_value("kafka1", "topic");
                kafka_groupid = par.cxx_get_value("kafka1", "groupid");
        
		g_VidHeader = par.cxx_get_value("vidheader", "vid");
		g_VidpNum = par.cxx_getint("vidheader", "vnum");
		g_mainNode = par.cxx_getint("vidheader", "bmain");
		string stmp = g_VidHeader;
		string pp;
		format_string(pp, "%d", 0);
		for (int i = 0; i < g_VidpNum; i++)
		{
			stmp = stmp + pp;
		}
		g_VidStr = stmp;
                dpca_server_addr = par.cxx_get_value("dpca", "ip");
		//////////////////////redis////////////////////2020.3.17 madongsheng
		int flag = par.cxx_getint("redis", "isRedis");
		bUseRedis = flag > 0;
		redisIp = par.cxx_get_value("redis", "ip");
		redisdbNo = par.cxx_getint("redis", "dbnum");
		redisPort = par.cxx_getint("redis", "port");
		redisUserName = par.cxx_get_value("redis", "username");
		redisPassWord = par.cxx_get_value("redis", "password");
		appId = par.cxx_get_value("cloudwalk", "appId");
		appSecret = par.cxx_get_value("cloudwalk", "appSecret");
		cloudwalkIp = par.cxx_get_value("cloudwalk", "ip");
		cloudwalkPort = par.cxx_getint("cloudwalk", "port");
		cloudwalkServer = cloudwalkIp + ":" + std::to_string(cloudwalkPort);
		clusterName = par.cxx_get_value("cloudwalk", "clusterName");
		mndIp = par.cxx_get_value("mnd", "ip");
		mndPort = par.cxx_getint("mnd", "port");
		g_sim = par.cxx_getint("mnd", "sim");
		g_lefttopx = par.cxx_getint("rect", "lx");
		g_lefttopy = par.cxx_getint("rect", "ly");
		g_rightbottomx = par.cxx_getint("rect", "rx");
		g_rightbottomy = par.cxx_getint("rect", "ry");
		devices = parse_dev_list(par.cxx_get_value("device", "dev"),",");
        g_bwsbz = par.cxx_getint("mnd", "bwsalarm");
		g_fcthdValue = par.cxx_getint("mnd","facecolsim");

#ifdef USE_ZOOKEEPER
		host = par.cxx_get_value("engine", "host");
		path = par.cxx_get_value("engine", "path");
		interval = par.cxx_getint("engine", "interval");
#endif
	}catch(CIniParser::sec_not_found& e) {
		format_string(errmsg, "INI sec not found! sec=%s, key=%s", e.sec.data(), e.key.data());
		return -2;
	}catch(CIniParser::valtype_mismatch&e ) {
		format_string(errmsg, "INI valtype convert fail! cannot convert [%s].%s to %s", 
			e.sec.data(), e.key.data(),e.failed_type.data());
		return -3;
	}catch(...) {
		format_string(errmsg, "other exception!");
		return -10;
	}
	return 0;
}


#ifdef USE_ZOOKEEPER
void showUsage(const char *name)
{
	printf("usage:\n");
	printf("For zookeeper startup, use : %s -z\n\n", name);
}

void main_watcher(zhandle_t *zkh, int type, int state, const char *path, void* context)
{
	/*
	* zookeeper_init might not have returned, so we
	* use zkh instead.
	*/
	printf("Something happened.\n");
	printf("type: %d\n", type);
	printf("state: %d\n", state);
	printf("path: %s\n", path);
	printf("watcherCtx: %s\n", (char *)context);
}

bool init_zookeeper()
{
	//zookeeper watch
	zh = zookeeper_init(host.data(), main_watcher, interval, 0, NULL, 0);
	if (zh == NULL)
	{
		fprintf(stderr, "Error when connecting to zookeeper servers...\n");
		return false;
	}

	int ret = zoo_create(zh, path.data(), NULL, -1, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, NULL, 0);
	if (ret)
	{
		fprintf(stderr, "Error %d for %s\n", ret, "create");
		return false;
	}
	else
	{
		fprintf(stderr, "zoo_create create node success\n");
	}

	return true;
}

void tell_zookeeper()
{
	char ok[128] = "{\"state\":\"done\"}\0";
	if (zh != nullptr)
	{
		zoo_set(zh, path.data(), ok, sizeof(ok), -1);
	}
}

#endif

int PathCheck(const char *path)
{
	int iRet = 0;
	if (access(path, 0) != 0)
	{
#ifdef WIN32
		iRet = mkdir(path);
#else
		iRet = mkdir(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
		return iRet;
	}else{
		return 0;
	}
}
void LogBasicSet()
{
	spdlog::flush_on(spdlog::level::debug);
	spdlog::set_pattern("[%Y/%m/%d %H:%M:%S %e] (%t) | [%l] [%@] %v");
	int rt = PathCheck(LOG_PATH);
	spdlog::basic_logger_mt(START_LOG, START_LOG_FILE, false);
}

int InitLogs(std::string& errmsg)
{
	try
	{
		spdlog::set_level(spdlog::level::from_str(log_level));
		spdlog::rotating_logger_mt(MAIN_LOG, MAIN_LOG_FILE, log_max_size, log_max_count);
		spdlog::rotating_logger_mt(ANAL_LOG, ANAL_LOG_FILE, log_max_size, log_max_count);
		spdlog::rotating_logger_mt(TRIFLE_LOG_BC_PT, HTTP_LOG_FILE, log_max_size, log_max_count);
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		format_string(errmsg, "Log initialization failed: %s\n", ex.what());
		return -1;
	}

	return ERROR_OK;
}

#include "HttpSession.h"
extern void FaceControlBegain(const char* request, size_t req_size, bp_http_hdl_t* hdl);
extern void FaceControlCancel(const char* request, size_t req_size, bp_http_hdl_t* hdl);
//#include "ws_stream_sender.h"
int main(int argc, char* argv[])
{
	using namespace std;
	LogBasicSet();
	LOG_INFO(START_LOG, "================== dpca ==================");
	LOG_INFO(START_LOG, "version: {}", VERSION_STRING);
	fmt::print("version: {}\n", VERSION_STRING);

	std::string emsg;
	int rt = load_config("dpca.cfg", emsg);
	if (rt != ERROR_OK)
	{
		LOG_ERROR(START_LOG, "cannot load 'dpca.cfg', {}", emsg);
		fmt::print("cannot load 'dpca.cfg', {}\n", emsg);
		exit(rt);
	}
        for (auto iter = devices.begin(); iter != devices.end(); iter++)
        {
		LOG_INFO(START_LOG, "check the devices : {}", *iter);
        }
	rt = InitLogs(emsg);
	if (rt != ERROR_OK)
	{
		LOG_ERROR(START_LOG, "init logs failed, {}", emsg);
		fmt::print("init logs failed, {}\n", emsg);
		exit(rt);
	}

#ifdef USE_ZOOKEEPER
	bool useZookeeper = false;
	if (argc < 2)
	{
		showUsage(argv[0]);
	}
	else
	{
		if (strcmp(argv[1], "-z") == 0)
		{
			useZookeeper = true;
		}
	}

	if (useZookeeper)
	{
		if (!init_zookeeper())
		{
			printf("init zookeeper failed\n");
			return -1;
		}
	}
#endif

	try
	{
		rmmt::global_init(glb_rmmt_server);
		svc_session_server_start(vst_server_addr);
		LOG_INFO(START_LOG, "start vstreamer server @{}", vst_server_addr);
		LOG_INFO(START_LOG, "starting websocket server @port={}..", ws_sport);
		fmt::print("start vstreamer server @{}\n", vst_server_addr.c_str());
		fmt::print("starting websocket server @port={}\n", ws_sport);
		add_algopkg(AP_CNN_FACE, face_url.data());//face_feature 人脸特征和结构化
		activemq::library::ActiveMQCPP::initializeLibrary();
		int rt = pAlCt.activemq_producer_init(alarm_broker.data(), alarm_username.data(), alarm_password.data(), alarm_topic.data());
		LOG_INFO(START_LOG,"alarm mq init ret: {} , broker:{}, user:{}, pswd:{} , topic:{}", rt, alarm_broker.data(), alarm_username.data(), alarm_password.data(), alarm_topic.data());
		rt = pAcCt.activemq_producer_init(active_broker.data(), active_username.data(), active_password.data(), active_topic.data());
		LOG_INFO(START_LOG,"alarm mq init ret: {} , broker:{}, user:{}, pswd:{} , topic:{}", rt, active_broker.data(), active_username.data(), active_password.data(), active_topic.data());
		rt = pFaCt.activemq_producer_init(facl_broker.data(), facl_username.data(), facl_password.data(), facl_topic.data());
		LOG_INFO(START_LOG,"alarm mq init ret: {} , broker:{}, user:{}, pswd:{} , topic:{}", rt, facl_broker.data(), facl_username.data(), facl_password.data(), facl_topic.data());
		rt = pFaCt1.activemq_producer_init(facl_broker1.data(), facl_username.data(), facl_password.data(), facl_topic.data());
		LOG_INFO(START_LOG,"alarm mq init ret: {} , broker:{}, user:{}, pswd:{} , topic:{}", rt, facl_broker1.data(), facl_username.data(), facl_password.data(), facl_topic.data());
		rt = pExtrApi_Prd.SetNetAddr(extr_ip.data(), extr_port);
		if (rt < 0)
		{   
			puts("init extr server failed!");
		}
		rt = pExtrApi_Bik.SetNetAddr(extr_ip1.data(), extr_port1);
		if (rt < 0)
		{   
			puts("init extr server failed!");
		}
		rt = pIseApi.SetNetAddr(ise_ip.data(), ise_port);
		if (rt < 0)
		{
			puts("init ise server failed!");
		}
		try{
			pIseApi.create_db(ise_dbname[5],"DeviceId String,TaskId String,IconId String","DeviceId,TaskId,IconId");
			sleep(2);
			printf("succeed!!create ise iconnnnnn!");
		}
		catch(GeneralException2& e)
		{
			LOG_INFO(START_LOG,"any ise iconnnn init error!!!---{}---{}",e.err_code(),e.err_msg());
		}
		try{
			pIseApi.create_db(ise_dbname[1],"DeviceId String,TaskId String,IconId String","DeviceId,TaskId,IconId");
			sleep(2);
			printf("succeed!!create ise iconnnnnn!");
		}
		catch(GeneralException2& e)
		{
			LOG_INFO(START_LOG,"any ise iconnnn init error!!!---{}---{}",e.err_code(),e.err_msg());
		}
		ftp_init(ftp_url.data(), ftp_username.data(), ftp_password.data());
		LOG_INFO(START_LOG,"{}", "ftp init!!!");
		rt = pFacCt.kafka_client_init(face_servers.data(), face_topic.data());
		LOG_INFO(START_LOG,"kafka init rt {}",rt);
		if (rt != 0)
			LOG_ERROR(START_LOG,"kafka init failed, broker:{}, topic:{}", face_servers.data(), face_topic.data());
		else
		{
			LOG_INFO(START_LOG,"kafka init succeed, broker:{}, topic:{}", face_servers.data(), face_topic.data());
		}
		rt = pClust.kafka_client_init(clus_servers.data(),clus_topic.data());
		LOG_INFO(START_LOG,"kafka init rt {}",rt);
		if (rt != 0)
			LOG_ERROR(START_LOG,"kafka init failed, broker:{}, topic:{}", clus_servers.data(), clus_topic.data());
		else
		{
			LOG_INFO(START_LOG,"kafka init succeed, broker:{}, topic:{}", clus_servers.data(), clus_topic.data());
		}
		rt = pFowst.kafka_client_init(follow_servers.data(),follow_topic.data());
		LOG_INFO(START_LOG,"kafka init rt {}",rt);
		if (rt != 0)
			LOG_ERROR(START_LOG,"kafka init failed, broker:{}, topic:{}", follow_servers.data(), follow_topic.data());
		else
		{
			LOG_INFO(START_LOG,"kafka init succeed, broker:{}, topic:{}", follow_servers.data(), follow_topic.data());
		}
		  LOG_INFO(START_LOG,"begin to start kafka consumer");
		m_consumer_.KafkaConsumerClientConstructor(kafka_broker.data(),kafka_topic.data(),kafka_groupid.data());
		  LOG_INFO(START_LOG,"end to start kafka consumer");
		  if (m_consumer_.KafkaConsumerInit())
		  {
			  LOG_INFO(START_LOG, "failed to start kafka consumer");
			  //throw GeneralException2(-1,"ret flag is null"); 
			  //return -1;
		  }
		  if (db_init() != 0)
		  {
			  LOG_INFO(START_LOG,"database init failed!");
			  return -1;
		  }
		  static std::thread _upt(update_token_thread_cloud); //更新token
		  if(bUseRedis)
		  {
		static std::thread redisWsTh([&]() {
			do
			{
connecttab:	g_RedisWsbzConnector = redisConnectWithTimeout(redisIp.c_str(), redisPort, g_RedisTimeOut);
				if (g_RedisWsbzConnector->err)
				{
					bConnectWsRedis = false;
					LOG_INFO(START_LOG,"{}", "connectRedisThread connect to redisServer fail");
					sleep(3);
				}
				else 
				{
					LOG_INFO(START_LOG,"{}", "connectRedisThread connect to redisServer success");
					bConnectWsRedis = true;
					break;
				}
				redisFree(g_RedisWsbzConnector);
				g_RedisWsbzConnector = NULL;
				
			} while (!bConnectWsRedis);
	
			while (true)
			{
				if (!bConnectWsRedis)
				{
					LOG_INFO(START_LOG,"{}", "connectRedisThread Redis disconnect");
					goto connecttab;
				}
				else 
				{
					sleep(10);
				}
			}
	
			redisFree(g_RedisWsbzConnector);	
			g_RedisWsbzConnector = NULL;
			}
			);
#if 0
		static std::thread redisSync2Th([&]() {
			do
			{
connectsynctab:	g_RedisSync2Connect = redisConnectWithTimeout(redisIp.c_str(), redisPort, g_RedisTimeOut);
				if (g_RedisSync2Connect->err)
				{
					bConnect2Redis = false;
					LOG_INFO(START_LOG,"connectRedisThread connect to redisServer fail---{}",g_RedisSync2Connect->errstr);
					sleep(3);
				}
				else 
				{
					LOG_INFO(START_LOG,"{}", "connectRedisThread connect to redisServer success");
					bConnect2Redis = true;
					redisReply *reply = (redisReply *)redisCommand(g_RedisSync2Connect,"CONFIG set notify-keyspace-events %s","Kx");
					   freeReplyObject(reply);
					   reply = NULL;
					LOG_INFO(START_LOG,"connect to redisServer set keyspace event return {}",rt);
					reply = (redisReply*)redisCommand(g_RedisSync2Connect,"select %d",redisdbNo);
					LOG_INFO(START_LOG,"connect to redisServer set channel return");
					if(reply == NULL)
                                        {
						bConnect2Redis = false;
						LOG_ERROR(START_LOG,"global redis select database false");
                                        }
                                        else
                                        {
					   freeReplyObject(reply);
					   reply = NULL;
					reply = (redisReply*)redisCommand(g_RedisSync2Connect,"get %s","hisense-global-face-control");
					//LOG_INFO(START_LOG,"connect to redisServer set keyspace event return {}",rt);
					if(reply == NULL)
                                        {
                                               int gnum = g_globalCtlNum.load(std::memory_order_seq_cst);
					       reply = (redisReply*)redisCommand(g_RedisSync2Connect,"set %s %d","hisense-global-face-control",gnum);
						       if(reply == NULL)
						       {
							       bConnect2Redis = false;
								       LOG_ERROR(START_LOG,"global redis select database false");
						       }
                                                       else
                                                       {
							       freeReplyObject(reply);
								       reply = NULL;
 							}
 					}
                                        else
                                        {
                                                string rediscache = string(reply->str,reply->len);
                                                int num = atoi(rediscache.data());
                                                g_globalCtlNum.store(num);
                                                
						freeReplyObject(reply);
						reply = NULL;

                                        }
                                        
                                        }
					break;
				}
				redisFree(g_RedisSync2Connect);
				//redisAsyncFree(g_RedisSync2Connect);	
				g_RedisSync2Connect = NULL;
				
			} while (!bConnect2Redis);
	
				while (true)
				{
					if (!bConnect2Redis)
					{
						LOG_INFO(START_LOG,"{}", "connectRedisThread Redis disconnect");
						goto connectsynctab;
					}
					else 
					{
						sleep(10);
					}
				}
	
				redisFree(g_RedisSync2Connect);	
				//redisAsyncFree(g_RedisSync2Connect);	
				g_RedisSync2Connect = NULL;
                    
                });
#endif
		static std::thread redisSyncTh([&]() {
			do
			{
connectsynctab:	g_RedisSyncConnector = redisConnectWithTimeout(redisIp.c_str(), redisPort, g_RedisTimeOut);
				if (g_RedisSyncConnector->err)
				{
					bConnect2Redis = false;
					LOG_INFO(START_LOG,"connectRedisThread connect to redisServer fail---{}",g_RedisSyncConnector->errstr);
					sleep(3);
				}
				else 
				{
					LOG_INFO(START_LOG,"{}", "connectRedisThread connect to redisServer success");
					bConnect2Redis = true;
					redisReply *reply = (redisReply *)redisCommand(g_RedisSyncConnector,"CONFIG set notify-keyspace-events %s","Kx");
					   freeReplyObject(reply);
					   reply = NULL;
					LOG_INFO(START_LOG,"connect to redisServer set keyspace event return {}",rt);
					reply = (redisReply*)redisCommand(g_RedisSyncConnector,"select %d",redisdbNo);
					LOG_INFO(START_LOG,"connect to redisServer set channel return");
					if(reply == NULL)
					{
						bConnect2Redis = false;
						LOG_ERROR(START_LOG,"global redis select database false");
					}
					else
					{
						freeReplyObject(reply);
						reply = NULL;
						reply = (redisReply*)redisCommand(g_RedisSyncConnector,"get %s","hisense-global-face-control");
						//LOG_INFO(START_LOG,"connect to redisServer set keyspace event return {}",rt);
						if(reply == NULL)
						{
							int gnum = g_globalCtlNum.load(std::memory_order_seq_cst);
							reply = (redisReply*)redisCommand(g_RedisSyncConnector,"set %s %d","hisense-global-face-control",gnum);
							if(reply == NULL)
							{
								bConnect2Redis = false;
								LOG_ERROR(START_LOG,"global redis select database false");
							}
							else
							{
								freeReplyObject(reply);
								reply = NULL;
							}
						}
						else
						{
							string rediscache = string(reply->str,reply->len);
							int num = atoi(rediscache.data());
							g_globalCtlNum.store(num);

							freeReplyObject(reply);
							reply = NULL;

						}

					}
					break;
				}
				redisFree(g_RedisSyncConnector);
				//redisAsyncFree(g_RedisSyncConnector);	
				g_RedisSyncConnector = NULL;
				
			} while (!bConnect2Redis);
	
				while (true)
				{
					if (!bConnect2Redis)
					{
						LOG_INFO(START_LOG,"{}", "connectRedisThread Redis disconnect");
						goto connectsynctab;
					}
					else 
					{
						sleep(10);
					}
				}
	
				redisFree(g_RedisSyncConnector);	
				//redisAsyncFree(g_RedisSyncConnector);	
				g_RedisSyncConnector = NULL;
                    
                });
                  //同步长链接保持线程
		static std::thread redisTh([&]() {
#if 1
                        int startflag = 0;
                        {
connecttab:			tbb::reader_writer_lock::scoped_lock lck(g_Redisrwl);
                                if(startflag == 1){
				redisAsyncFree(g_RedisConnector);	
                                }
                        	g_RedisConnector = redisAsyncConnect(redisIp.c_str(), redisPort);
 			}
				if (g_RedisConnector->err)
                                {
					LOG_INFO(START_LOG,"connectRedisThread connect to redisServer fail---{}",g_RedisConnector->errstr);
                                        //throw GeneralException2(666,"general redis connect error!");
                                        //sleep(3);
                                        startflag = 0;
                                }
                                else
                                {
                                        if(startflag == 1) event_base_free(g_base);
  					g_base = event_base_new();//新建一个libevent事件处理
					startflag = 1;
                                        redisLibeventAttach(g_RedisConnector,g_base);//将连接添加到libevent事件处理
					redisAsyncSetConnectCallback(g_RedisConnector,connectCallback);//设置连接回调
					redisAsyncSetDisconnectCallback(g_RedisConnector,disconnectCallback);//设置断开连接回调	
 					event_base_dispatch(g_base); //开始libevent循环。注意在这一步之前redis是不会进行连接的
  				}
                                sleep(5);
				goto connecttab;
#else
#endif
			});
		  }

#ifdef USE_ZOOKEEPER
		if (useZookeeper)
		{
			tell_zookeeper();
		}
#endif 
		  printf("5555555555555555555555");
                InitGusetChan("tianyue_face");
		  printf("6666666666666666666666");
		InitWebsocketServerAndStartLoop(ws_sport);
		  printf("7777777777777777777777");
               ///////////////////////http////////////////////////////
               register_http("/executeControl/begin",FaceControlBegain);
               register_http("/executeControl/end",FaceControlCancel);
               http_service_start(http_port);
               LOG_INFO(START_LOG, "http server started!");
#ifdef __linux
	struct rlimit fd_limits;
	fd_limits.rlim_cur = fd_limits.rlim_max = 65000;
	if (setrlimit(RLIMIT_NOFILE, &fd_limits)) {
		perror("setrlimit:");
		return -1;
	}
#endif
	LOG_INFO(START_LOG, "bc_str server started!");
	fmt::print("{}","bc_str server started!\n");
#ifdef __linux
	pause();
#else
	while (true)
		Sleep(100000);
#endif
// 
// 	while (true)
// 		system_sleep(10000);
	LOG_INFO(START_LOG, "exit main!");
	return 0;
                
	}
	catch (...) 
	{
		LOG_ERROR(START_LOG, "Service Error !");
	}

	return 0;
}

