#ifndef _CURL_HTTP_CLIENT_
#define _CURL_HTTP_CLIENT_

#include "curl/curl.h"

struct Curl_Http;

class CurlHttpClient
{
public:
	CurlHttpClient();
	~CurlHttpClient();
	void SetURL(const char *URL);
	void AddHeader(const char *header);
	void SetRequestWays(const char *ways);
	void SetPostData(const char *PostData);
	void SetTimeOut(long time);
	int GetResponse_Code(long *code);
	const char* GetResponse_Head();
	const char* GetResponse_Body();
	void Execute_request();
    void Get_LastUrl(char **urlstr);

private:
	Curl_Http *http;
};

#endif // _CURL_HTTP_CLIENT_
