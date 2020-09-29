#ifndef _ISE_PROXY_H_
#define _ISE_PROXY_H_
#include <string>
#include "StreamCmdApis.h"

using namespace std;

typedef enum E_ISE_TYPE
{
	eTypeVehicle = 1,
	eTypeBike = 2,
	eTypePerson = 3,
	eTypeFace = 4
}E_ISE_TYPE;

string get_db_fields_format(int type);

void ise_init(const string& ip, int port);

string ise_create_db();

void ise_del_db_data(const string& db_name);

IdxPac ise_push_data(const string& db_name, int type, const string& feat, const string& value_feilds,bool isCluster = false);

//////////////////////////////////////////////////////////////////////////
void extr_init(const string& ip, int port);

string extr_feat_person(uint32_t w, uint32_t h, uint32_t pitch,const char* pixdata);


#endif // !_ISE_PROXY_H_
