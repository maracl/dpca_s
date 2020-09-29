#ifndef _ACTIVEMQ_PRODUCER_H_
#define _ACTIVEMQ_PRODUCER_H_

#include "event.h"
#include "sync.hpp"
#include <string>
#include <list>
#include "fast_sync_cls.hpp"
// #include "tbb/concurrent_queue.h"
// #include "tbb/reader_writer_lock.h"

using namespace std;

class AmqClient:no_copy
{

public:
	AmqClient();
	~AmqClient();

	bool s_init_success;

	string s_broker;
	string s_username;
	string s_password;
	string s_topic;

	bool s_loopStart;// = false;
	list<string> s_msg_list;
	//tbb::concurrent_bounded_queue<string> s_msg_queue;
	ThreadLocker s_msg_locker;
	event_t		s_msg_event;

	int activemq_producer_loop(void* param);

	int activemq_producer_init(const char* broker, const char* username, const char* password, const char* topic);

	bool activemq_produce(const char* msg);

};


#endif // _ACTIVEMQ_PRODUCER_H_