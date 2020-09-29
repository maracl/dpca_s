#ifndef _KAFKA_CLIENT_H_
#define _KAFKA_CLIENT_H_

//#include "rdkafka.h"
#include "common_interface.h"
#include "event.h"
#include "sync.hpp"
#include <thread>
#include <string>
#include <list>
#include "KafkaClientAPI.h"

using namespace std;
class KafkaClient :no_copy
{
public:
	KafkaClient();
	~KafkaClient();

public:
	rd_kafka_t* s_rk;
	rd_kafka_topic_t* s_rkt;
	rd_kafka_topic_conf_t* s_rconf;
	rd_kafka_conf_t* s_conf;
	char s_errstr[512];
	bool s_kafka_init;// = false;

	list<string> s_msg_list;
	ThreadLocker s_msg_locker;
	event_t		s_msg_event;

	std::thread  s_kafka_thread;
	std::string kafka_servers;
	std::string kafka_topic;

	int gSuccNum;// = 0;
	int gTotalNum;// = 0;
	int unInitNUm;
public:
	int kafka_client_init(const char* servers, const char* topic);

	bool kafka_client_poll(const char* msg);

	bool kafka_client_reinit();

	//void dr_msg_cb(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque);

	int kafka_client_loop(void * param);

};



#endif // _KAFKA_CLIENT_H_
