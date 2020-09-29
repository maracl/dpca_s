#include "curl_http_client.h"
#include <iostream>
#include <string.h>

using std::string;

size_t default_writecallback(void* buffer, size_t size, size_t nmemb, void* param)
{
	string *str = (string *)param;
	str->append((const char*)buffer, size*nmemb);
	return size*nmemb;
}

struct Curl_Http
{
	CURL *curl;
	CURLcode res;
	long retcode;
	int IS_AFTER_EXECUTE;
	struct curl_slist *headers;
	string response_body;
	string response_head;
	size_t (*write_callback)(void *buffer, size_t size, size_t nmemb, void *param);
};


CurlHttpClient::CurlHttpClient()
{
	http = new Curl_Http;
	http->curl = curl_easy_init();
	http->headers = NULL;
	http->response_body = "";
	http->response_head = "";
	http->IS_AFTER_EXECUTE = 0;
	http->write_callback = default_writecallback;

	curl_easy_setopt(http->curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(http->curl, CURLOPT_TIMEOUT, 3);
}

CurlHttpClient::~CurlHttpClient()
{
	if (http->headers != NULL)
		curl_slist_free_all(http->headers);

	if (http->curl != NULL)
		curl_easy_cleanup(http->curl);

	delete http;
	http = NULL;
}

void CurlHttpClient::SetURL(const char *URL)
{
	curl_easy_setopt(http->curl, CURLOPT_URL, URL);
	if (strstr(URL, "https://") != NULL)    //如果url是https类型的，就增加对https协议的支持
	{
		curl_easy_setopt(http->curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(http->curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}
}

void CurlHttpClient::AddHeader(const char *header)
{
	http->headers = curl_slist_append(http->headers, header);
}

void CurlHttpClient::SetRequestWays(const char *ways)
{
	curl_easy_setopt(http->curl, CURLOPT_CUSTOMREQUEST, ways);
}

void CurlHttpClient::SetPostData(const char *PostData)
{
	curl_easy_setopt(http->curl, CURLOPT_POSTFIELDS, PostData);
}

void CurlHttpClient::SetTimeOut(long timeout)
{
	curl_easy_setopt(http->curl, CURLOPT_CONNECTTIMEOUT, timeout);
	curl_easy_setopt(http->curl, CURLOPT_TIMEOUT, timeout);
}

void CurlHttpClient::Execute_request()
{
	curl_easy_setopt(http->curl, CURLOPT_HTTPHEADER, http->headers);
	curl_easy_setopt(http->curl, CURLOPT_WRITEFUNCTION, http->write_callback);
	curl_easy_setopt(http->curl, CURLOPT_WRITEDATA, (void *)&(http->response_body));
	curl_easy_setopt(http->curl, CURLOPT_HEADERDATA, (void *)&(http->response_head));
	http->res = curl_easy_perform(http->curl);

	if (http->res != CURLE_OK)
		throw curl_easy_strerror(http->res);

	http->IS_AFTER_EXECUTE = 1;
}

int CurlHttpClient::GetResponse_Code(long *code)
{
	return curl_easy_getinfo(http->curl, CURLINFO_RESPONSE_CODE, code);
}

const char* CurlHttpClient::GetResponse_Head()
{
	if (http->IS_AFTER_EXECUTE == 1)
		return (http->response_head).c_str();
	else
		return NULL;
}

const char* CurlHttpClient::GetResponse_Body()
{
	if (http->IS_AFTER_EXECUTE == 1)
		return (http->response_body).c_str();
	else
		return NULL;
}

void CurlHttpClient::Get_LastUrl(char **urlstr)
{
	curl_easy_getinfo(http->curl, CURLINFO_EFFECTIVE_URL, urlstr);
}