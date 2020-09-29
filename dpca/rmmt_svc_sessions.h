#ifndef _RMMT_SVC_SESSIONS_H
#define _RMMT_SVC_SESSIONS_H

#include <string>
#include "bath_utils.h"
#include "businesspkg_common.h"

using std::string;
struct bp_alg_sess_t
{
	AlgoConnector algo;
	int errcode;
	bp_alg_sess_t() {
		errcode = 0;
	}   
};

void svc_session_server_start(const std::string& addr);

#endif
