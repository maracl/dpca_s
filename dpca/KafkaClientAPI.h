#ifndef _KAFKA_CLIENT_API_H_
#define _KAFKA_CLIENT_API_H_	
#include <string>
#include "rdkafka/rdkafka.h"


typedef struct message_t
{
	int32_t					m_partition;
	std::string				payload;
	//char					payload[1024 * 10];
	size_t					m_len;
	int64_t					offset;
	rd_kafka_resp_err_t		err;
}message_t;
typedef std::shared_ptr<message_t> message_ptr;//zj 20200428 图片流



class KafkaConsumerClient
{
private:
	std::string							s_brockers;
	std::string							s_topic;
	std::string							s_groupID;
	rd_kafka_conf_t						*s_rkc;
	rd_kafka_topic_conf_t				*s_rktc;
	rd_kafka_topic_partition_list_t		*s_rktp_list;
	rd_kafka_t							*s_rk;
	rd_kafka_resp_err_t					s_err;
	char								s_errS[512];
public:	
	KafkaConsumerClient() {}
	~KafkaConsumerClient(){
		rd_kafka_flush(s_rk, 2 * 1000 /* wait for max 2 seconds */);
		//rd_kafka_topic_partition_list_destroy(s_rktp_list);
		rd_kafka_consumer_close(s_rk);
		rd_kafka_destroy(s_rk);
		//rd_kafka_conf_destroy(s_rkc);
	}
	int KafkaConsumerClientConstructor(const char *hosts, const char *topic, const char *group_id);
	KafkaConsumerClient(KafkaConsumerClient & src);
	int KafkaConsumerInit();
	int KafkaConsume(message_t *t_msg);
};

class KafkaProducerClient
{
private:
	std::string							s_brockers;
	std::string							s_topic;
	rd_kafka_conf_t						*s_rkc;
	rd_kafka_t							*s_rk;
	rd_kafka_topic_t					*s_rkt;
	rd_kafka_conf_res_t					s_err;
	char								s_errS[512];
public:
	KafkaProducerClient(){}
	int KafkaProducerClientConstructor(const char * hosts,const char * t_topic);
	~KafkaProducerClient(){
		rd_kafka_conf_destroy(s_rkc);
		rd_kafka_destroy(s_rk);
		rd_kafka_topic_destroy(s_rkt);
	}
	int KafkaProducerInit();
	int KafkaProduce(const std::string msg);
};

#endif // _KAFKA_CLIENT_API_H_	
