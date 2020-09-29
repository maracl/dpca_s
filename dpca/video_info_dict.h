#ifndef _VIDEO_INFO_DICT_H_
#define _VIDEO_INFO_DICT_H_

#include <string>

using namespace std;

long time_ts(const char* time);
string ts_time(long ts);

long long ts_time2(long ts);

string video_info_find(const string& devide, const string& start, const string& end);

#endif // _VIDEO_INFO_DICT_H_