#ifndef _HTTP_HIS_CLIENT_H_
#define _HTTP_HIS_CLIENT_H_

#include <string>
using std::string;


bool http_post(const char* url, const char* param, std::string& reply);
//post param :1 图搜入库参数 2 图片url
void post_doubleplatc_iseinfo(const string& plv,const string& feat,const string& dbname, string& reply);

//post param 文件路径
void post_doubleplatc_file(const string& filepath, string& reply);

void post_doubleplatc_faceAlert(const string& Alert, string& reply);

void post_doubleplatc_face(const string& filepath, string& reply);

void post_doubleplatc_TransmitImgFace(const string& filepath, string& reply);

void post_glint(const string& param, string& reply);
bool HttpClientPostCloudWalk(const char* url, const char* param, const string& token, std::string& reply, int timeOut = 20);
#endif // _HTTP_HIS_CLIENT_H_

