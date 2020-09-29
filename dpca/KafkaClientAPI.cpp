#include <assert.h>
#include <cstring>
#include "rapidjson/Rjson.hpp"
//#include "glog/logging.h"
#include "spdlog_helper.h"
#include "KafkaClientAPI.h"
#include "Util.h"


/*CONSUMER API START*/
static void kafka_log_cb(const rd_kafka_t *rk,int level,const char *fac,const char *buf)
{
//	LOG(INFO) << "RDKAKFKA-" << level << "-" << fac << " : " << rd_kafka_name(rk) << " : " << buf;
	LOG_INFO(ANAL_LOG,"RDKAKFKA-{}-{} : {} : {}",level,fac,rd_kafka_name(rk),buf);
}

static void producer_msg_cb(rd_kafka_t *rk,const rd_kafka_message_t *rkmessage,void *opaque) 
{
	if (rkmessage->err)
		//LOG(ERROR) << "Message delivery failed: " << rd_kafka_err2str(rkmessage->err);
	LOG_ERROR(ANAL_LOG,"Message delivery failed: {}",rd_kafka_err2str(rkmessage->err));
	else
	LOG_ERROR(ANAL_LOG,"Message delivered success! lenth :{}",rkmessage->len);
		//LOG(INFO) << "Message delivered success! lenth :" << rkmessage->len;
}


int KafkaConsumerClient::KafkaConsumerClientConstructor(const char *hosts, const char *topic,const char *group_id)
{
	s_brockers = hosts;
	s_topic = topic;
	s_groupID = group_id;
	s_rk = NULL;
	s_rkc = rd_kafka_conf_new();
	s_rktc = rd_kafka_topic_conf_new();
	memset(s_errS, 0, sizeof(s_errS));
	LOG_INFO(ANAL_LOG,"USE KAFFKA CONSUMER API VERSION : {}",rd_kafka_version_str());
	//LOG(INFO) << "USE KAFFKA CONSUMER API VERSION : " << rd_kafka_version_str();
	return 0;
}

int KafkaConsumerClient::KafkaConsumerInit()
{
	rd_kafka_conf_set_log_cb(s_rkc, kafka_log_cb);	
	assert(!rd_kafka_conf_set(s_rkc, "internal.termination.signal", "29"/*SIGIO*/, NULL, 0));
	/*设置重提offset位置*/
//	if (rd_kafka_topic_conf_set(s_rktc, "offset.store.method", "broker", s_errS,sizeof(s_errS)))
//	{
//		LOG(ERROR) << "set offset.store.method failed : " << s_errS;
//		memset(s_errS, 0, sizeof(s_errS));
//		return -1;
//	}
	if (rd_kafka_conf_set(s_rkc, "auto.offset.reset", "latest", s_errS, sizeof(s_errS)))
	{
//		LOG(ERROR) << "set auto.offset.reset failed : " << s_errS;
	    LOG_ERROR(ANAL_LOG,"set auto.offset.reset failed : {}",s_errS);
		memset(s_errS, 0, sizeof(s_errS));
		return -1;
	}	
	if (rd_kafka_conf_set(s_rkc, "max.partition.fetch.bytes","10485760", s_errS, sizeof(s_errS)))
	{
//		LOG(ERROR) << "set auto.offset.reset failed : " << s_errS;
	    LOG_ERROR(ANAL_LOG,"set auto.offset.reset failed : {}",s_errS);
		memset(s_errS, 0, sizeof(s_errS));
		return -1;
	}	
	/*设置消费者groupID*/
	if (rd_kafka_conf_set(s_rkc, "group.id", s_groupID.data(), s_errS, sizeof(s_errS)))
	{
	    LOG_ERROR(ANAL_LOG,"set group id failed : {}",s_errS);
		//LOG(ERROR) << "set group id failed : " << s_errS;
		memset(s_errS, 0, sizeof(s_errS));
		return -1;
	}
	rd_kafka_conf_set_default_topic_conf(s_rkc, s_rktc);
	//rd_kafka_conf_set(s_rkc,"enable.partition.eof","true",NULL,0);
	/*创建消费者客户端*/
	if (!(s_rk = rd_kafka_new(RD_KAFKA_CONSUMER,		//创建kafka消费者客户端
		s_rkc,
		s_errS,
		sizeof(s_errS)))) {
			//LOG(ERROR) << "create consumer client failed : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"create consumer client failed : {}",s_errS);
			return -1;
	}
	/*指定kafka集群*/
	if (rd_kafka_brokers_add(s_rk, s_brockers.data()) == 0) {				//指定kafka集群
//		LOG(ERROR) << "no valid brokers specified when configure consumer.";
	    LOG_ERROR(ANAL_LOG,"no valid brokers specified when configure consumer.");
		return -1;
	}
	/*获取主题分区*/
	const rd_kafka_metadata_t *t_container = NULL;
	if ((s_err = rd_kafka_metadata(s_rk, 1, NULL, &t_container, 1000)))
	{
		//LOG(ERROR) << "failed to querry topic-partitions : " << rd_kafka_err2str(s_err);
	        LOG_ERROR(ANAL_LOG,"failed to querry topic-partitions : {}",rd_kafka_err2str(s_err));
		rd_kafka_metadata_destroy(t_container);
		return -1;
	}
	s_rktp_list = rd_kafka_topic_partition_list_new(0);
	/*查询要订阅的主题分区详情*/
	for (int i = 0; i < t_container->topic_cnt; ++i)
	{
		if (!strcmp(s_topic.data(), (t_container->topics[i]).topic))
		{
			for (int j = 0; j < (t_container->topics[i]).partition_cnt; ++j)
			{
				rd_kafka_topic_partition_list_add(s_rktp_list, s_topic.data(),
					(t_container->topics[i]).partitions[j].id);
				/*设置消费起始位置*/
//				if (rd_kafka_topic_partition_list_set_offset(s_rktp_list,
//					s_topic.data(), (t_container->topics[i]).partitions[j].id,
//					300000) != RD_KAFKA_RESP_ERR_NO_ERROR)
//					std::cout << "set offset failed!" << std::endl;
			}
		}
	}
	rd_kafka_metadata_destroy(t_container);
	/*指定订阅分区*/
	if ((s_err = rd_kafka_assign(s_rk, s_rktp_list))) 
	{
		//LOG(ERROR) << "failed to assign partition : " << rd_kafka_err2str(s_err);
	        LOG_ERROR(ANAL_LOG,"failed to assign partition : {}",rd_kafka_err2str(s_err));
		return -1;
	}
	
	/*提交偏移*/
//	if ((s_err = rd_kafka_commit(s_rk, s_rktp_list, 0)))
//	{
//		LOG(ERROR) << "commit failed : " << rd_kafka_err2str(s_err);
//		return -1;
//	}
	/*poll重定向*/
	if((s_err = (rd_kafka_poll_set_consumer(s_rk))))
		return -1;
	return 0;
}
int KafkaConsumerClient::KafkaConsume(message_t *t_message)
{
	rd_kafka_message_t *rkmessage = NULL;
	LOG_INFO(MAIN_LOG, "begin a rdkafka pool!");
	rkmessage = rd_kafka_consumer_poll(s_rk, 1000);
	LOG_INFO(MAIN_LOG, "end a rdkafka pool!");
	if (rkmessage)
	{
		switch (rkmessage->err)
		{
		case RD_KAFKA_RESP_ERR_NO_ERROR:
		{
			//char tmp_buf[1024 * 1024 * 8];
			char* tmp_buf = (char*)malloc(rkmessage->len + 1);
			strncpy(tmp_buf, (char*)rkmessage->payload, rkmessage->len);
			tmp_buf[rkmessage->len] = '\0';
			t_message->err = rkmessage->err;
			t_message->payload = string(tmp_buf,rkmessage->len);
			t_message->m_len = rkmessage->len;
			t_message->m_partition = rkmessage->partition;
			t_message->offset = rkmessage->offset;
			rd_kafka_message_destroy(rkmessage);
			free(tmp_buf);
			tmp_buf = nullptr;
			return 0;
                }
			break;	
		case RD_KAFKA_RESP_ERR_UNKNOWN_TOPIC_OR_PART:
			//LOG(ERROR) << "consume failed : unkonw topic or part!";
	        LOG_ERROR(ANAL_LOG,"consume failed : unkonw topic or part!");
			break;
		default:
			//LOG(ERROR) << "consume failed! : " << rkmessage->err;	
	        LOG_ERROR(ANAL_LOG,"consume failed! : {}",rkmessage->err);
			break;
		}
	}
	return -1;
}
/*CONSUMER API END*/

/*PRODUCER API START*/
int KafkaProducerClient::KafkaProducerClientConstructor(const char * hosts, const char * t_topic)
{
	s_brockers = hosts;
	s_topic	= t_topic;
	s_rk = NULL;
	s_rkc = rd_kafka_conf_new();
	s_rkt = NULL;
	memset(s_errS, 0, sizeof(s_errS));
	return 0;
}
int KafkaProducerClient::KafkaProducerInit()
{
	if ((s_err = rd_kafka_conf_set(s_rkc, "bootstrap.servers", s_brockers.data(), s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify brockers : " << s_errS;
	    LOG_ERROR(ANAL_LOG,"failed to specify brockers : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	if ((rd_kafka_conf_set(s_rkc, "batch.num.messages", "10", s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify batch.num.messages : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"failed to specify batch.num.messages : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	if ((rd_kafka_conf_set(s_rkc, "queue.buffering.max.messages", "30000", s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify queue.buffering.max.messages : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"failed to specify queue.buffering.max.messages : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	if ((rd_kafka_conf_set(s_rkc, "message.timeout.ms", "5000", s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify message.timeout.ms : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"failed to specify message.timeout.ms : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	if ((rd_kafka_conf_set(s_rkc, "message.send.max.retries", "3", s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify message.send.max.retries : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"failed to specify message.send.max.retries : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	if ((rd_kafka_conf_set(s_rkc, "retry.backoff.ms", "500", s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify retry.backoff.ms : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"failed to specify retry.backoff.ms : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	if ((rd_kafka_conf_set(s_rkc, "topic.metadata.refresh.interval.ms", "10000", s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify topic.metadata.refresh.interval.ms : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"failed to specify topic.metadata.refresh.interval.ms : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	if ((rd_kafka_conf_set(s_rkc, "socket.max.fails", "0", s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify socket.max.fails : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"failed to specify socket.max.fails : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	if ((rd_kafka_conf_set(s_rkc, "log.connection.close", "false", s_errS, sizeof(s_errS))))
	{
		//LOG(ERROR) << "failed to specify log.connection.close : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"failed to specify log.connection.close : {}",s_errS);
		return -1;
	}
	memset(s_errS, 0, sizeof(s_errS));
	/*设置发送消息回调*/
	rd_kafka_conf_set_dr_msg_cb(s_rkc, producer_msg_cb);
	s_rk = rd_kafka_new(RD_KAFKA_PRODUCER, s_rkc, s_errS, sizeof(s_errS));
	if (s_rk == NULL)
	{
		//LOG(ERROR) << "create producer failed : " << s_errS;
	        LOG_ERROR(ANAL_LOG,"create producer failed : {}",s_errS);
		return -1;
	}
	s_rkt = rd_kafka_topic_new(s_rk, s_topic.data(), NULL);
	if (s_rkt == NULL)
	{
		//LOG(ERROR) << "create producer topic failed!";
	        LOG_ERROR(ANAL_LOG,"create producer topic failed!");
		return -1;
	}
	return 0;
}

int KafkaProducerClient::KafkaProduce(const std::string msg)
{
retry:
	if (rd_kafka_produce(s_rkt, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY, (void*)msg.data(), msg.length(), NULL, 0, s_errS) == -1)
		//LOG(ERROR) << s_errS << rd_kafka_topic_name(s_rkt) << " : " << rd_kafka_err2str(rd_kafka_last_error());
	        LOG_ERROR(ANAL_LOG,"{}{}:{}",s_errS,rd_kafka_topic_name(s_rkt),rd_kafka_err2str(rd_kafka_last_error()));
	else
		//LOG(INFO) << "push ok!";
	        LOG_INFO(ANAL_LOG,"push ok!");
	memset(s_errS, 0, sizeof(s_errS));
	if (rd_kafka_last_error() == RD_KAFKA_RESP_ERR__QUEUE_FULL) 
	{
		rd_kafka_poll(s_rk, 1000);
		goto retry;
	}
	return 0;
}

/*PRODUCER API END*/


