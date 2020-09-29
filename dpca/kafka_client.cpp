#include "Util.h"
#include "kafka_client.h"
//#include "rdkafka.h"
//#include "XLogger.h"
#include "spdlog_helper.h"

//#include "thread.h"
// #include "event.h"
// #include "sync.hpp"
// #include <string>
// #include <list>

using namespace std;

//extern void FormatLogA(const char* format, ...);

// static rd_kafka_t* s_rk;
// static rd_kafka_topic_t* s_rkt;
// static rd_kafka_conf_t* s_conf;
// static char s_errstr[512];
// static bool s_kafka_init = false;
// 
// static list<string> s_msg_list;
// static ThreadLocker s_msg_locker;
// static event_t		s_msg_event;
// 
// static int gSuccNum = 0;
// static int gTotalNum = 0;
extern bool kafka_check;


//static pthread_t thread;

//static int unInitNUm = 0;

static void dr_msg_cb(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque)
{
	if (rkmessage->err != 0)
	{
		LOG_INFO(MAIN_LOG,"%% Message delivery failed: code:{}, mag: {}", rkmessage->err, rd_kafka_err2str(rkmessage->err));
		if (rkmessage->err == RD_KAFKA_RESP_ERR__ALL_BROKERS_DOWN)
		{
			//FormatLogA("---> kafka all brokers down! <---");
			kafka_check = false;
		}
	}
}


bool KafkaClient::kafka_client_reinit()
{
	s_kafka_init = false;
	//thread_destroy(thread);线程销毁
	if (s_kafka_thread.joinable())
	{
		s_kafka_thread.join();
	}
	//******************************
	rd_kafka_flush(s_rk, 2 * 1000 /* wait for max 2 seconds */);
	/* Destroy topic object */
	rd_kafka_topic_destroy(s_rkt);
	/* Destroy the producer instance */
	rd_kafka_destroy(s_rk);
	//******************************
	if (0 == kafka_client_init(kafka_servers.data(), kafka_topic.data()))
	{
		//FormatLogA("kafka reconnect succeed!");
		return true;
	}
	else
	{
		//FormatLogA("kafka reconnect failed!");
		return false;
	}
}



int KafkaClient::kafka_client_loop(void* param)
{
	if (!s_rkt)
		return -1;

	while (true) {
		event_wait(&s_msg_event);
		list<string> msg_list;
		{

			AutoThreadLocker locker(s_msg_locker);
			msg_list.swap(s_msg_list);
		}

		for (auto iter = msg_list.begin(); iter != msg_list.end(); ++iter)
		{
			string& msg = *iter;

			gTotalNum++;
			int rt = rd_kafka_produce(s_rkt, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY, (void*)msg.c_str(), msg.size(), NULL, 0, NULL);
			if (rt == -1) {
				//FormatLogA("%% Failed to produce to topic, code :%d, msg, %s: %s ------>succ/total(%d/%d)<------", rd_kafka_last_error(),
					//rd_kafka_topic_name(s_rkt), rd_kafka_err2str(rd_kafka_last_error()), gSuccNum, gTotalNum);

				if (rd_kafka_last_error() == RD_KAFKA_RESP_ERR__QUEUE_FULL) {
					rd_kafka_poll(s_rk, 100);
					continue;
				}
			}
			else
			{
				gSuccNum++;
				//FormatLogA("%d Enqueued message (%zd bytes) for topic %s  ------>succ/total(%d/%d)<------", rt, msg.size(), rd_kafka_topic_name(s_rkt), gSuccNum, gTotalNum);
			}

			rd_kafka_poll(s_rk, 0);
		}

		event_reset(&s_msg_event);

		rd_kafka_poll(s_rk, 0);
	}
}

int KafkaClient::kafka_client_init(const char* servers, const char* topic)
{
	kafka_servers = servers;
	kafka_topic = topic;
	if (s_kafka_init)
		return -1;

	s_conf = rd_kafka_conf_new();

	if (rd_kafka_conf_set(s_conf, "bootstrap.servers", servers,
		s_errstr, sizeof(s_errstr)) != RD_KAFKA_CONF_OK) {
		return -1;
	}


	rd_kafka_conf_set(s_conf, "batch.num.messages", "10000", s_errstr, sizeof(s_errstr));//10
	//rd_kafka_conf_set(s_conf, "message.timeout.ms", "10000", s_errstr, sizeof(s_errstr));//5000
	rd_kafka_conf_set(s_conf, "queue.buffering.max.messages", "500000", NULL, 0);
	rd_kafka_conf_set(s_conf, "message.send.max.retries", "10", NULL, 0);
	rd_kafka_conf_set(s_conf, "retry.backoff.ms", "500", NULL, 0);
	rd_kafka_conf_set(s_conf, "session.timeout.ms", "10000", NULL, 0);
	rd_kafka_conf_set(s_conf, "topic.metadata.refresh.interval.ms", "10000", s_errstr, sizeof(s_errstr));//
	rd_kafka_conf_set(s_conf, "socket.max.fails", "0", s_errstr, sizeof(s_errstr));//
	rd_kafka_conf_set(s_conf, "log.connection.close", "false", s_errstr, sizeof(s_errstr));

	rd_kafka_conf_set_dr_msg_cb(s_conf, dr_msg_cb);

	if (!kafka_check)
	{
		s_kafka_init = false;
	}
	kafka_check = true;
	s_rk = rd_kafka_new(RD_KAFKA_PRODUCER, s_conf, s_errstr, sizeof(s_errstr));
	if (!s_rk) {
		//FormatLogA("%% Failed to create new producer: %s", s_errstr);
		return -1;
	}

	//if (rd_kafka_brokers_add(s_rk, "node1:9092") == 0) {
	//	FormatLogA("%% No valid brokers specified");
	//	return -1;
	//}
	s_rconf = rd_kafka_topic_conf_new();
	rd_kafka_topic_conf_set(s_rconf, "message.timeout.ms", "10000", s_errstr, sizeof(s_errstr));
	rd_kafka_topic_conf_set(s_rconf, "request.timeout.ms", "10000", s_errstr, sizeof(s_errstr));
	//rd_kafka_topic_conf_set(s_rkt, "acks","",NULL,0);

	s_rkt = rd_kafka_topic_new(s_rk, topic, s_rconf);
	if (!s_rkt) {
		//FormatLogA("%% Failed to create topic object: %s", rd_kafka_err2str(rd_kafka_last_error()));
		rd_kafka_destroy(s_rk);
		return -1;
	}


	event_create(&s_msg_event);

	//thread_create(&thread, kafka_client_loop, NULL);线程创建
	s_kafka_thread = std::thread([&]() {
		kafka_client_loop(NULL);
	});
	s_kafka_thread.detach();
	//////////////////////////////////////////////////////////////////////////
	s_kafka_init = true;
	unInitNUm = 0;

	return 0;
}

bool KafkaClient::kafka_client_poll(const char* msg)
{
	if (!s_kafka_init)
	{
		if (++unInitNUm > 100)
		{
			bool bSucc = kafka_client_reinit();
			if (!bSucc)
				return false;
		}
	}

	{
		AutoThreadLocker locker(s_msg_locker);
		s_msg_list.push_back(msg);
	}

	event_signal(&s_msg_event);

	return true;
}

KafkaClient::KafkaClient()
{
	gSuccNum = 0;
	gTotalNum = 0;
	unInitNUm = 0;
	s_kafka_init = false;

}

KafkaClient::~KafkaClient()
{
}
