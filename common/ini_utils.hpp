/*
 * ini_utils.hpp
 *
 *  Created on: 2011-12-13
 *      Author: ltmit
 */

#ifndef INI_UTILS_HPP_
#define INI_UTILS_HPP_
#ifdef _MSC_VER
#pragma once
#pragma warning(disable:4996)
#define PRId64 "I64d"
#else
#define sscanf_s sscanf
#endif
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <map>
#include <string.h>

#ifndef PRId64
#define PRId64 "lld"
#endif

namespace ltmit{

struct iniValue_t {
	std::string val;

	bool toInt(int& v) const {
		return sscanf_s(val.c_str(),"%d",&v)>0;
	}

	bool toReal(double& v) const {
		return sscanf_s(val.c_str(),"%lf",&v)>0;
	}
};

typedef std::map<std::string,iniValue_t> iniSec_t;

class CIniParser {
	static void getTrimStr(const char* str,std::string& dst){
		while(isspace(*str))
			++str;
		intptr_t offset=strlen(str)-1;
		for(;offset>=0 && isspace(str[offset]); --offset) ;
		dst.assign(str,0,offset+1);
	}
//public:
	std::map<std::string,iniSec_t> iniData;
public:
	class sec_not_found{
	public:
		std::string sec,key;
		explicit sec_not_found(const std::string& s, const std::string& k) :sec(s),key(k) {}
	};
	class valtype_mismatch{
	public:
		std::string sec,key,failed_type;
		explicit valtype_mismatch(const std::string& s, const std::string& k,const std::string& ft) :
			sec(s), key(k), failed_type(ft) {}
	};

	std::string cxx_get_value(const std::string& str_sec,const std::string&str_key) const {
		iniValue_t val;
		if(get_data(str_sec,str_key,val))
			throw sec_not_found(str_sec,str_key);
		return val.val;
	}
	int cxx_getint(const std::string& str_sec,const std::string&str_key) const {
		iniValue_t val;
		if(get_data(str_sec,str_key,val))
			throw sec_not_found(str_sec, str_key);
		int retval=0;
		if(sscanf_s(val.val.c_str(),"%d",&retval)<1)
			throw valtype_mismatch(str_sec, str_key, "int");
		return retval;
	}

	int64_t cxx_getint64(const std::string& str_sec,const std::string&str_key) const {
		iniValue_t val;
		if(get_data(str_sec,str_key,val))
			throw sec_not_found(str_sec, str_key);
		int64_t retval=0;
		if(sscanf_s(val.val.c_str(),"%" PRId64,&retval)<1)
			throw valtype_mismatch(str_sec, str_key,"int64");
		return retval;
	}
	
	double cxx_getreal(const std::string& str_sec,const std::string& str_key) const {
		iniValue_t val;
		if(get_data(str_sec,str_key,val))
			throw sec_not_found(str_sec, str_key);
		double retval=0;
		if(sscanf_s(val.val.c_str(),"%lf",&retval)<1)
			throw valtype_mismatch(str_sec, str_key,"real");
		return retval;
	}
public:
	int get_data(const std::string& str_sec,const std::string& str_key,iniValue_t& val)const {
// 		try{
// 			const iniSec_t &sec=iniData.at(str_sec);
// 			val=sec.at(str_key);
// 			return 0;
// 		}catch(...) {return -1;}
		std::map<std::string,iniSec_t>::const_iterator ci=iniData.find(str_sec);
		if(ci==iniData.end())
			return -1;
		const iniSec_t& sec=ci->second;
		iniSec_t::const_iterator i=sec.find(str_key);
		if(i==sec.end())
			return -2;
		val=i->second;
		return 0;
	}

	int parseFile(const char* fpath){
		FILE* fp=fopen(fpath,"r");
		if(fp==NULL)
			return -1;
		iniData.clear();
		std::string key("");
		iniSec_t* pSec=&iniData[key];	//global section (key="")
		//assert(pSec);
		for(;;) {
			char buf[1025]={0};
			if(fgets(buf,1024,fp)==NULL)
				break;
			if(buf[0]=='#'||isspace(buf[0]))
				continue;
			if(buf[0]=='['){//section title
				char* pend=strchr(buf+1,']');
				if(pend==NULL) {
					fputs("error parse section title end ]\n",stderr);
					return 1;
				}
				*pend=0;
				getTrimStr(buf+1,key);
				//printf("key=%s\n",key.c_str());
				pSec=&iniData[key];
			}else{//section content
				if(pSec==NULL) {
					fputs("error fill section content ]\n",stderr);
					return 2;
				}
				char * pmid=strchr(buf,'=');
				if(pmid==NULL)
					continue;
				*pmid=0;	++pmid;
				getTrimStr(buf,key);
				getTrimStr(pmid,(*pSec)[key].val);
			}
		}
		fclose(fp);
		return 0;
	}
};

}////////////END of NAMESPACE/////////////////
#endif /* INI_UTILS_HPP_ */