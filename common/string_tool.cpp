#include "string_tool.h"

#ifdef _WIN32
#include <regex>
#else
#include <regex.h>
#endif

//注意：当字符串为空时，也会返回一个空字符串
// 提取子串

#ifndef _WIN32
char* getsubstr(const char *s, regmatch_t *pmatch)
{
	static char buf[100] = { 0 };
	memset(buf, 0, sizeof(buf));
	memcpy(buf, s + pmatch->rm_so, pmatch->rm_eo - pmatch->rm_so);
	return buf;
}
#endif

char** ch_search(string data, const char *pattern, int* size)
{
	int nCount = 0;
	char **ret = (char**)calloc(100, sizeof(char *));
#ifdef _WIN32
	regex reg(pattern);
	sregex_iterator beg(data.cbegin(), data.cend(), reg);
	sregex_iterator end;

	for (; beg != end; ++beg){
	//for_each(beg, end, [](const smatch& m){
		ret[nCount] = (char*)calloc(beg->length() + 1, sizeof(char));
		memcpy(ret[nCount], beg->str(0).data(), beg->length());
		nCount++;
	}
#else
	const char* pBuf = data.c_str();

	regmatch_t pmatch;
	regex_t reg;
	regcomp(&reg, pattern, REG_EXTENDED); 	//编译正则表达式
	
	int offset = 0;
	while (offset < strlen(pBuf))
	{
		int status = regexec(&reg, pBuf + offset, 1, &pmatch, 0);
		if (status == REG_NOMATCH)
		{
			//printf("No Match\n");
			break;
		}
		else if (pmatch.rm_so != -1)
		{
			char *p = getsubstr(pBuf + offset, &pmatch);
			//printf("Match:[%d, %d]: %s\n", offset + pmatch.rm_so + 1, offset + pmatch.rm_eo, p);
			ret[nCount] = (char*)calloc(pmatch.rm_eo - pmatch.rm_so + 1, sizeof(char));     //分配子串长度+1的内存空间  
			memcpy(ret[nCount], p, pmatch.rm_eo - pmatch.rm_so);
			nCount++;
		}
		offset += pmatch.rm_eo;
	}
	regfree(&reg); 		//释放正则表达式
#endif
	*size = nCount;
	return ret;
}


void splits(std::string& s, std::string& delim, std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret->push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		ret->push_back(s.substr(last, index - last));
	}
}

int amend_rect(std::string &str, double v)
{
	int size;
	char pattern[] = "[0-9]+,[0-9]+,[0-9]+,[0-9]+";
	char**ret = ch_search(str, pattern, &size);
	for (int i = 0; i < size; ++i)
	{
		//cout << "data[" << i << "] ---> " << ret[i];	
		std::string strs = ret[i];
		std::string delim = ",";
		vector<std::string>strv;
		splits(strs, delim, &strv);
		double r0 = v * (double)atoi(strv[0].data());
		double r1 = v * (double)atoi(strv[1].data());
		double r2 = v * (double)atoi(strv[2].data());
		double r3 = v * (double)atoi(strv[3].data());
		char strFina[50] = { '\0' };
		sprintf(strFina, "%0.f,%0.f,%0.f,%0.f", r0, r1, r2, r3);
		//cout << " ---> " << strFina << endl;
		str.replace(str.find(strs), strs.length(), strFina);

		free(ret[i]);
	}
	free(ret);
	return size;
}

