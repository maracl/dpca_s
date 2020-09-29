#include "download_image.h"
#include "bath_utils.h"
#include "curl/curl.h"
#include "byte_buffer.h"
#include "create_uuid.h"
#include <string.h>
//#include "http_his_client.h"
//#include "XLogger.h"
#include "spdlog_helper.h"
////structure
#ifndef WIN32
#include <sys/time.h>
#endif

using namespace std;

static string s_ftp_url;
static string s_ftp_username;
static string s_ftp_password;
bool bpush = true;
int bflag = BIGPUSH;
int maxNum = 1;
int countNum = 0;
int bigNum = 0;
int ImageNum = 0;
int volatile Pushidx = 0;
int volatile PushBig = 0;
int volatile PushImage = 0;
string pUrl = "";

extern int disknum;//”≤≈Ã ˝¡ø
extern std::vector<int> diskbath;//”≤≈Ã»›¡ø£®G£©
std::vector<int> pPushNum;
std::vector<string> pPushUrl;

int one_max_com_div(int a, int b)
{
	while (a != 0 && b != 0) a < b ? b -= a : a -= b;
	return a == 0 ? b : a;
}

int mul_max_com_div()
{
	int max;
	max = one_max_com_div(diskbath[0],diskbath[1]);
	for (int i = 1; i <= disknum - 1; i++) max = one_max_com_div(max,diskbath[i]);

	return max;

}


size_t http_callback_func(char *data, size_t size, size_t nmemb, std::string* userdata)
{
	unsigned long sizes = size * nmemb;
	userdata->append(data, sizes);
	return sizes;
}

int http_download_image(const std::string& url, std::string* file, long timeoutMs)
{
	CURL* curl_ctx = curl_easy_init();

	curl_easy_setopt(curl_ctx, CURLOPT_URL, url.data());
	curl_easy_setopt(curl_ctx, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, http_callback_func);
	curl_easy_setopt(curl_ctx, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl_ctx, CURLOPT_TIMEOUT, timeoutMs);

	CURLcode rc = curl_easy_perform(curl_ctx);
	if (rc) {
		printf("!!! http Failed to download: %s\n", url.data());
		return -1;
	}

	long res_code = 0;
	curl_easy_getinfo(curl_ctx, CURLINFO_RESPONSE_CODE, &res_code);
	if (!((res_code == 200 || res_code == 201) && rc != CURLE_ABORTED_BY_CALLBACK)) {
		printf("!!! Response code: %ld\n", res_code);// zj 20190910
		return -1;
	}

	curl_easy_cleanup(curl_ctx);

	return 0;
}

size_t ftp_callback_func(char *data, size_t size, size_t nmemb, std::string* userdata)
{
	unsigned long sizes = size * nmemb;
	userdata->append(data, sizes);
	return sizes;
}

int ftp_download_image(const std::string& url, std::string* file, long timeoutMs/* = 3000*/)
{
	CURL* curl_ctx = curl_easy_init();

	curl_easy_setopt(curl_ctx, CURLOPT_URL, url.data());
	//curl_easy_setopt(curl_ctx, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl_ctx, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl_ctx, CURLOPT_USERNAME, s_ftp_username.data());
	curl_easy_setopt(curl_ctx, CURLOPT_PASSWORD, s_ftp_password.data());
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, ftp_callback_func);
	curl_easy_setopt(curl_ctx, CURLOPT_TIMEOUT, timeoutMs);

	CURLcode rc = curl_easy_perform(curl_ctx);
	if (rc) {
		printf("!!! ftp Failed to download: %s\n", url.data());
		return -1;
	}

	curl_easy_cleanup(curl_ctx);
	return 0;
}

struct WriteThis {
	const char *readptr;
	size_t sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct WriteThis *upload = (struct WriteThis *)userp;
	size_t max = size*nmemb;

	if (max < 1)
		return 0;

	if (upload->sizeleft) {
		size_t copylen = max;
		if (copylen > upload->sizeleft)
			copylen = upload->sizeleft;
		memcpy(ptr, upload->readptr, copylen);
		upload->readptr += copylen;
		upload->sizeleft -= copylen;
		return copylen;
	}

	return 0;
}

// year-month-day-hour
void get_cur_date(string& date)
{
#ifdef __linux__
	struct timeval _tv;
	gettimeofday(&_tv, 0);
	struct tm _tm;
	localtime_r(&_tv.tv_sec, &_tm);
	format_string(date, "%d_%02d_%02d_%02d", _tm.tm_year + 1900, _tm.tm_mon + 1, _tm.tm_mday, _tm.tm_hour);
#endif
}

int ftp_upload_image(const void* data, size_t sz, const std::string& url_path, std::string& ftp_url,long timeoutMs /*= 3*/)
{
	curl_global_init(CURL_GLOBAL_ALL);

	CURL* curl = curl_easy_init();
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl, CURLOPT_USERNAME, s_ftp_username.data());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, s_ftp_password.data());
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutMs);

	string url;
// 	string cur_date;
// 	get_cur_date(cur_date);
// 	if (disknum == 1)
// 	{
// 		format_string(url, "%s/%s", s_ftp_url.data(), url_path.data());
// 	}
// 	else
// 	{
// 
// 	}
	format_string(url, "%s/structure/%s", s_ftp_url.data(), url_path.data());
	ftp_url = url;

	struct WriteThis upload;
	upload.readptr = (const char *)data;
	upload.sizeleft = sz;

	curl_easy_setopt(curl, CURLOPT_URL, url.data());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
	CURLcode rc = curl_easy_perform(curl);
	if (rc) {
		printf("!!! ftp Failed to upload: %s\n", url.data());
		return -1;
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return 0;
}

int ftp_upload_image(const std::string& file, std::string& path,int pflag, long timeoutMs )
{
// 	if (bpush)
// 	{
// 		pflag = SMALLPUSH;
// 		bpush = false;
// 	}
	curl_global_init(CURL_GLOBAL_ALL);
	CURL* curl = curl_easy_init();
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl, CURLOPT_USERNAME, s_ftp_username.data());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, s_ftp_password.data());
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutMs);

	string url;
	string cur_date;
	get_cur_date(cur_date);

	if (disknum == 1)
	{
		format_string(url, "%s/structure/%s/%s.jpg", s_ftp_url.data(), cur_date.data(), create_uuid(true).data());
	}
	else
	{
		if (pflag == SMALLPUSH)
		{
			countNum++;
			//url = pPushUrl[Pushidx];
			if (Pushidx >= disknum || Pushidx < 0)
			{
				Pushidx = 0;
			}
			format_string(url, "%s/%s/%s.jpg", pPushUrl[Pushidx].data(), cur_date.data(), create_uuid(true).data());
			if (Pushidx >= disknum || Pushidx < 0)
			{
				Pushidx = 0;
			}
			if (countNum >= pPushNum[Pushidx])
			{
				countNum = 0;
				Pushidx++;
			}
			if (Pushidx >= disknum || Pushidx < 0)
			{
				Pushidx = 0;
			}
		}
		else if (pflag == BIGPUSH)
		{
			bigNum++;
			//url = pPushUrl[Pushidx];
			if (PushBig >= disknum || PushBig < 0)
			{
				PushBig = 0;
			}
			format_string(url, "%s/%s/%s.jpg", pPushUrl[PushBig].data(), cur_date.data(), create_uuid(true).data());
			if (PushBig >= disknum || PushBig < 0)
			{
				PushBig = 0;
			}
			if (bigNum >= pPushNum[PushBig])
			{
				bigNum = 0;
				PushBig++;
			}
			if (PushBig >= disknum || PushBig < 0)
			{
				PushBig = 0;
			}
		}
		else
		{
			ImageNum++;
			//url = pPushUrl[Pushidx];
			if (PushImage >= disknum || PushImage < 0)
			{
				PushImage = 0;
			}
			format_string(url, "%s/%s/%s.jpg", pPushUrl[PushImage].data(), cur_date.data(), create_uuid(true).data());
			if (PushImage >= disknum || PushImage < 0)
			{
				PushImage = 0;
			}
			if (ImageNum >= pPushNum[PushImage])
			{
				ImageNum = 0;
				PushImage++;
			}
			if (PushImage >= disknum || PushImage < 0)
			{
				PushImage = 0;
			}
		}
// 		else
// 		{
// 			format_string(url, "%s/%s/%s.jpg", pUrl.data(), cur_date.data(), create_uuid(true).data());
// 		}
		//url = pPushUrl[Pushidx];

		//bflag = pflag;
	}

	struct WriteThis upload;
	upload.readptr = file.data();
	upload.sizeleft = file.size();

	curl_easy_setopt(curl, CURLOPT_URL, url.data());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
	CURLcode rc = curl_easy_perform(curl);
// 	if (rc) {
// 		printf("!!! ftp Failed to upload: %s\n", url.data());
// 		return -1;
// 	}
// 
// 	curl_easy_cleanup(curl);
// 	path = url;
// 	return 0;
	//≤‚ ‘À´Õ¯À´∆ΩÃ®
#if 0
	try {
		string rp;
		post_doubleplatc_file(url, rp);
	}
	catch (...)
	{
		CRITICAL("{}", "do doubleplatc file false!!unhandled!");
	}
#endif
	//////////////////////////////////////////////////////////////////////////
	if (rc == CURLE_OK)
	{
		curl_easy_cleanup(curl);
		path = url;
		return 0;
	}
	else if (rc == CURLE_REMOTE_DISK_FULL)
	{
		curl_easy_cleanup(curl);
		for (int i = 0; i < disknum; i++)
		{
			format_string(url, "%s/%s/%s.jpg", pPushUrl[i].data(), cur_date.data(), create_uuid(true).data());
			CURLcode rr = mul_max_com_div(file, url);
			if (rr == CURLE_OK)
			{
				path = url;
				return 0;
			}

		}
		printf("!!! check ftp all Failed to upload: %s\n", url.data());
		return -1;

	}
	else {
		printf("!!! ftp Failed to upload: %s\n", url.data());
		curl_easy_cleanup(curl);
		path = url;
		return -1;
	}
}

CURLcode mul_max_com_div(const std::string& file, std::string url, long timeoutMs)
{
	CURL* curl = curl_easy_init();
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, (long)1);
	curl_easy_setopt(curl, CURLOPT_USERNAME, s_ftp_username.data());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, s_ftp_password.data());
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutMs);
	//////////////////////////////////////////////////////////////////////////
	string cur_date;
	get_cur_date(cur_date);
	//////////////////////////////////////////////////////////////////////////
	struct WriteThis upload;
	upload.readptr = file.data();
	upload.sizeleft = file.size();

	curl_easy_setopt(curl, CURLOPT_URL, url.data());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, &upload);
	CURLcode rc = curl_easy_perform(curl);//
	curl_easy_cleanup(curl);
	return rc;
}

void ftp_init(const std::string& url, const std::string& username, const std::string& password)
{
	curl_global_init(CURL_GLOBAL_ALL);
	s_ftp_url = url;
	s_ftp_username = username;
	s_ftp_password = password;
	if (disknum!=1)
	{
		maxNum = mul_max_com_div();
		for (int i = 0; i < disknum; i++)
		{
			int n = diskbath[i] / maxNum;
			string purl;
			if (i != 0)
			{
				format_string(purl, "%s/structure%d", s_ftp_url.data(), i+1);
			}
			else
			{
				format_string(purl, "%s/structure", s_ftp_url.data());
			}
			pPushUrl.push_back(purl);
			pPushNum.push_back(n);
		}
	}

}
