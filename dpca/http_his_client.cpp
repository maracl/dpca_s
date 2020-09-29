#include "Util.h"
#include "http_his_client.h"
#include "curl_http_client.h"
#include "byte_buffer.h"
//#include "rapidjson/Rjson.hpp"
//#include "jsonparser.h"
//#include "jsonhelper.h"
#include "rapidjson/Rjson.hpp"
#include "spdlog_helper.h"

extern string g_client_ip;
extern int g_client_port;
extern std::string tokenStr;//token ¿¿
extern std::string appId;
extern std::string appSecret;
extern std::string cloudwalkIp;
extern int cloudwalkPort;
extern std::string cloudwalkServer;
extern std::string clusterName;

extern void FormatLogA(const char* format, ...);

//共用函数，用来发送http post请求
bool http_post(const char* url, const char* param, std::string& reply)
{
	try {
		//FormatLogA("request is %s", param);
		CurlHttpClient client;
		//client.AddHeader("Content-Type: application/json");
		client.AddHeader("Content-Type: application/json;charset=UTF-8");
		client.AddHeader("User-Agent: XXX");
		client.SetRequestWays("POST");
		client.SetTimeOut(10);
		client.SetURL(url);
		client.SetPostData(param);
		client.Execute_request();
		reply = client.GetResponse_Body();
		//FormatLogA("hisense gateway reply is %s", reply.c_str());
	}
	catch (const char*) {
		LOG_INFO(ANAL_LOG,"got a false char");
		reply = "{\"retFlag\":\"1\", \"errCode\":\"gateway connect falied\" }";
		return false;
	}
	catch (...)
	{
		LOG_INFO(ANAL_LOG,"got a false char1");
		reply = "{\"retFlag\":\"-1\", \"errCode\":\"gateway connect falied\" }";
		return false;
	}

	try {
		Document d;
		d.Parse(reply.data());
		string errCode = d["code"].GetString();
		return errCode == "200";
	}
	catch (...) {
		reply = "{\"retFlag\":\"1\", \"errCode\":\"reply invalid\" }";
		return false;
	}
}

bool http_post_deepglint(const char* url, const char* param, std::string& reply)
{
	try {
		//FormatLogA("request is %s", param);
		CurlHttpClient client;
		//client.AddHeader("Content-Type: application/json");
		client.AddHeader("Content-Type: application/json;charset=UTF-8");
		client.AddHeader("authkey: dp-auth-v0");//½¨Òé´ÓÅäÖÃÎÄ¼þ¶ÁÈ¡
		client.AddHeader("access_key: 7966023d-0eee-4104-9c2f-8efa4279442a");//½¨Òé´ÓÅäÖÃÎÄ¼þ¶ÁÈ¡
		client.AddHeader("secret_key: 9e7b1900-302b-4059-bcaa-047262f3ca3b");//½¨Òé´ÓÅäÖÃÎÄ¼þ¶ÁÈ¡
		client.SetRequestWays("POST");
		client.SetTimeOut(20);
		client.SetURL(url);
		client.SetPostData(param);
		client.Execute_request();
		reply = client.GetResponse_Body();
		//FormatLogA("hisense gateway reply is %s", reply.c_str());
	}
	catch (const char*) {
		reply = "{\"retFlag\":\"1\", \"errCode\":\"gateway connect falied\" }";
		return false;
	}
	catch (...)
	{
		reply = "{\"retFlag\":\"-1\", \"errCode\":\"gateway connect falied\" }";
		return false;
	}
	return true;
}

bool HttpClientPostCloudWalk(const char* url,const char* param, const string& token, std::string& reply, int timeOut)
{
	try {
		CurlHttpClient client;
		client.AddHeader("Content-Type: application/json;charset=utf-8");
		if (!token.empty())
			client.AddHeader(("Authorization: Bearer " + token).c_str());
		client.AddHeader("Connection: Keep-Alive / close");
		
		client.SetRequestWays("POST");
		client.SetTimeOut(timeOut);
		client.SetURL(url);
		client.SetPostData(param);
		client.Execute_request();
		reply = client.GetResponse_Body();
	}
              catch(GeneralException2& e)
              {
		    LOG_INFO(ANAL_LOG,"http post error:code: {},msg:{} !", e.err_code(), e.err_str());
              }
	catch (const char*) {
		//reply = "{\"retFlag\":\"1\", \"errCode\":\"gateway connect falied\" }";
		return true;
	}
	catch (...)
	{
		//reply = "{\"retFlag\":\"-1\", \"errCode\":\"gateway connect falied\" }";
		return true;
	}

	try {
		Document d;
		d.Parse(reply.data());
		string errCode = d["code"].GetString();
                //string errId("70121003");
                //if(errCode == errId)
                //{
		//	Document p;
		//	p.Parse(param);
		//	string devid  = p["deviceId"].GetString();
		//	LOG_INFO(ANAL_LOG,"deviceid not register it is :{}",devid.data());
		//	std::thread([&,devid](){
                //                      try{
                //                        //string dbCode = string("0000000000000000000");
                //                        string deviceName = string("金水源小区点位");
                //                        string longitude = string("120");
                //                        string latitude = string("60");
		//			std::string pathUrl = "http://" + cloudwalkServer + "/ocean/api/device/camera/add";
		//			//std::string param1 = "{\"deviceName\":\"" + deviceName + "\",\"cameraType\":\"VMS_GB\",\"para\":\"" + devid + "\",\"lng\":\"" + longitude + "\",\"lat\":\"" + latitude + "\",\"gbCode\":\"" + dbCode + "\",\"refDeviceCameraId\":\"" + devid + "\"}";
		//			std::string param1 = "{\"deviceName\":\"" + deviceName + "\",\"cameraType\":\"VIRTUAL\",\"para\":\"" + devid + "\",\"lng\":\"" + longitude + "\",\"lat\":\"" + latitude + "\",\"refDeviceCameraId\":\"" + devid + "\"}";
		//			LOG_INFO(ANAL_LOG, "http camera add param :{}", param1.data());
		//			std::string reply;
		//			if (!HttpClientPostCloudWalk(pathUrl.c_str(), param1.c_str(), tokenStr, reply))
		//			{
		//				LOG_INFO(ANAL_LOG, "OnSubmitTask device regist falied cloudwalk");
		//				return;// ERROR_DEVICE_REGISTER;
		//			}
		//			LOG_INFO(ANAL_LOG, "http camera add reply :{}", reply);
		//			Document replyDoc;
		//			replyDoc.Parse(reply);
		//			if (!replyDoc.HasMember("data"))
		//			{
		//				LOG_INFO(ANAL_LOG, "OnSubmitTask device regist falied cloudwalk data not found");
		//				return;// ERROR_DEVICE_REGISTER;
		//			}
		//			string logicDeviceId =	replyDoc["data"].GetString();
                //                        
		//			{
		//				std::string pathUrl = "http://" + cloudwalkServer + "/ocean/api/device/engine/bind";
		//				string param1 = "{\"deviceId\":\"" + logicDeviceId + "\",\"engineType\":1}";
		//			LOG_INFO(ANAL_LOG, "http engine bind param :{}", param1.data());
		//				std::string reply;
		//				if (!HttpClientPostCloudWalk(pathUrl.c_str(), param1.c_str(), tokenStr, reply))
		//				{
		//					std::string pathUrl = "http://" + cloudwalkServer + "/ocean/api/device/camera/delete";
		//					std::string param1 = "{\"deviceId\":\"" + logicDeviceId + "\"}";
		//					std::string reply;
		//					HttpClientPostCloudWalk(pathUrl.c_str(), param1.c_str(), tokenStr, reply);
		//					
		//					LOG_ERROR(ANAL_LOG, "OnSubmitTask device /ocean/api/device/engine/bind falied cloudwalk");
		//					return;// ERROR_DEVICE_REGISTER;
		//				}
		//				LOG_INFO(ANAL_LOG, "http egine bind reply:{}", reply);
		//			}
		//			pathUrl = "http://" + cloudwalkServer + "/ocean/api/increment/cluster/append";
                //                        string clusterId = string("2e754481e934408383659eafec52fa1b");
		//			param1 = "{\"taskId\":\"" + clusterId + "\",\"detailIncreParams\":[{\"groupId\":\"" + logicDeviceId + "\",\"groupType\":0}]}";
		//			LOG_INFO(ANAL_LOG, "http cluster append param :{}", param1.data());
		//			reply = "";
		//			if (!HttpClientPostCloudWalk(pathUrl.c_str(), param1.c_str(), tokenStr, reply))
		//			{
		//				//É¾³ýÉè±¸
		//				std::string pathUrl = "http://" + cloudwalkServer + "/ocean/api/device/camera/delete";
		//				std::string param1 = "{\"deviceId\":\"" + logicDeviceId + "\"}";
		//				std::string reply;
		//				HttpClientPostCloudWalk(pathUrl.c_str(), param1.c_str(), tokenStr, reply);
		//				LOG_DEBUG(ANAL_LOG, "OnSubmitTask /api/increment/cluster/appende failed");
		//				return;// ERROR_DEVICE_REGISTER;
		//			}	
		//			LOG_INFO(ANAL_LOG, "http cluster append reply:{}", reply);
                //                     }
		//		catch(GeneralException2& e)
		//		{
		//		   LOG_INFO(ANAL_LOG, "begin register deviceid catch error {}---{}!!!!", e.err_code(), e.err_str());
		//		   printf("begin register deviceid catch error %d---%s!!!!\n", e.err_code(), e.err_str());
		//		}
		//	}).detach();
                //    
                //}
		//return errCode == "00000000";
		return true;
	}
	catch (...) {
		//reply = "{\"retFlag\":\"1\", \"errCode\":\"reply invalid\" }";
		return true;
	}
	return true;
}

void post_doubleplatc_iseinfo(const string& plv, const string& feat,const string& dbname, string& reply)
{
	//string requestPath;
	//format_string(requestPath,"http://%s:%d/doubleplat/getImageAnalysisAndPush",g_client_ip.data(),g_client_port);
	//jsonobject obj;
	//obj.add("plv",plv);
	//obj.add("feature",feat);
	//obj.add("dbname", dbname);
	//string param = obj.json();
	//http_post(requestPath.data(), param.data(), reply);
}

void post_doubleplatc_file(const string& filepath, string& reply)
{
	//string requestPath;
	//format_string(requestPath,"http://%s:%d/doubleplat/TransmitImgStruct", g_client_ip.data(), g_client_port);
	//jsonobject obj;
	//obj.add("imgUrl", filepath);
	//string param = obj.json();
	//http_post(requestPath.data(), param.data(), reply);
}

void post_doubleplatc_face(const string& filepath, string& reply)
{
	//string requestPath;
	//format_string(requestPath, "http://%s:%d/doubleplat/TransmitImgFace", g_client_ip.data(), g_client_port);
	//jsonobject obj;
	//obj.add("imgUrl", filepath);
	//string param = obj.json();
	//http_post(requestPath.data(), param.data(), reply);
}

void post_doubleplatc_faceAlert(const string& Alert, string& reply)
{
	//string requestPath;
	//format_string(requestPath, "http://%s:%d/doubleplat/AlertFace", g_client_ip.data(), g_client_port);
	//http_post(requestPath.data(), Alert.data(), reply);
}

void post_doubleplatc_TransmitImgFace(const string& filepath, string& reply)
{
	//string requestPath;
	//format_string(requestPath, "http://%s:%d/doubleplat/TransmitImgFace", g_client_ip.data(), g_client_port);
	//jsonobject obj;
	//obj.add("imgUrl", filepath);
	//string param = obj.json();
	//http_post(requestPath.data(), param.data(), reply);
}

void post_glint(const string& param, string& reply)
{}
