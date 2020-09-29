#ifndef _DOWNLOAD_IMAGE_H_
#define _DOWNLOAD_IMAGE_H_

#include <string>
#include "curl/curl.h"

int http_download_image(const std::string& url, std::string* file, long timeoutMs = 3);

void ftp_init(const std::string& url, const std::string& username, const std::string& password);

int ftp_download_image(const std::string& url, std::string* file, long timeoutMs=3);

int ftp_upload_image(const void* fileData, size_t sz, const std::string& url_path, std::string& ftp_url,long timeoutMs = 3);

int ftp_upload_image(const std::string& file, std::string& path,int pflag, long timeoutMs = 3);

int one_max_com_div(int a,int b);
 
int mul_max_com_div();

CURLcode mul_max_com_div(const std::string& file, std::string url, long timeoutMs = 5);

#endif // _DOWNLOAD_IMAGE_H_