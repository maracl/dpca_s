#include "activemq_producer.h"
#include "Util.h"
#include "common_interface.h"
#include "sync.hpp"
#include "thread.h"


#include <activemq/activemq/library/ActiveMQCPP.h>  
#include <activemq/decaf/lang/Thread.h>  
#include <activemq/decaf/lang/Runnable.h>  
#include <activemq/decaf/util/concurrent/CountDownLatch.h>  
#include <activemq/decaf/lang/Integer.h>  
#include <activemq/decaf/lang/Long.h>  
#include <activemq/decaf/lang/System.h>  
#include <activemq/activemq/core/ActiveMQConnectionFactory.h>  
#include <activemq/activemq/util/Config.h>  
#include <activemq/cms/Connection.h>  
#include <activemq/cms/Session.h>  
#include <activemq/cms/TextMessage.h>  
#include <activemq/cms/BytesMessage.h>  
#include <activemq/cms/MapMessage.h>  
#include <activemq/cms/ExceptionListener.h>  
#include <activemq/cms/MessageListener.h> 

#include "spdlog_helper.h"

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;

// static bool s_init_success;
// 
// static string s_broker;
// static string s_username;
// static string s_password;
// static string s_topic;
// 
// bool s_loopStart = false;
// static list<string> s_msg_list;
// static ThreadLocker s_msg_locker;
// static event_t		s_msg_event;

int AmqClient::activemq_producer_loop(void* param)
{
	//activemq::library::ActiveMQCPP::initializeLibrary();

	Connection*      connection;
	Session*         session;
	Destination*     destination;
	MessageProducer* producer;

	try {
		// Create a ConnectionFactory  
		auto_ptr<ConnectionFactory> connectionFactory(ConnectionFactory::createCMSConnectionFactory(s_broker));

		// Create a Connection  
		connection = connectionFactory->createConnection(s_username, s_password);
		connection->start();

		session = connection->createSession(Session::AUTO_ACKNOWLEDGE);

		destination = session->createTopic(s_topic);

		// Create a MessageProducer from the Session to the Topic or Queue  
		producer = session->createProducer(destination);
		producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
	}
	catch (CMSException& e) {
		e.printStackTrace();
		LOG_ERROR(MAIN_LOG,"mq init false:{}", e.getMessage().data());
		s_loopStart = false;
		s_init_success = false;
		return -1;
	}
	catch (...)
	{

	}

	LOG_INFO(MAIN_LOG,"{}", "mq init true");
	s_init_success = true;

	while (true) {
		//event_wait(&s_msg_event);
		list<string> msg_list;
		{
			AutoThreadLocker locker(s_msg_locker);
			if (!s_msg_list.empty())
			{
				msg_list.swap(s_msg_list);
				s_msg_list.clear();
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}

		for (auto iter = msg_list.begin(); iter != msg_list.end(); ++iter)
		{
			//FormatLogA("send message to mq : %s", iter->data());
			try
			{
				TextMessage* message = session->createTextMessage(*iter);
				//tag = true;
				producer->send(message);
				delete message;
			}
			catch (...) {
				LOG_CRITICAL(MAIN_LOG,"{}", "active mq is exception!!!!!");
				s_init_success = false;
				break;
			}
		}
		if (s_init_success == false)
		{
			break;
		}
		//event_reset(&s_msg_event);
	}

	//ÊÍ·Å×ÊÔ´  
	try {
		if (destination != NULL) delete destination;
	}
	catch (CMSException& e) { e.printStackTrace(); }
	destination = NULL;

	try {
		if (producer != NULL) delete producer;
	}
	catch (CMSException& e) { e.printStackTrace(); }
	producer = NULL;

	// Close open resources.  
	try {
		if (session != NULL) session->close();
		if (connection != NULL) connection->close();
	}
	catch (CMSException& e) { e.printStackTrace(); }

	try {
		if (session != NULL) delete session;
	}
	catch (CMSException& e) { e.printStackTrace(); }
	session = NULL;

	try {
		if (connection != NULL) delete connection;
	}
	catch (CMSException& e) { e.printStackTrace(); }
	connection = NULL;

	//activemq::library::ActiveMQCPP::shutdownLibrary();
	s_loopStart = false;
}

int AmqClient::activemq_producer_init(const char* broker, const char* username, const char* password, const char* topic)
{
	if (s_loopStart)
	{
		return 1;
	}
	s_broker = broker;
	s_username = username;
	s_password = password;
	s_topic = topic;
	//s_msg_queue.set_capacity(66);
	event_create(&s_msg_event);
	//pthread_t thread;
	//thread_create(&thread, activemq_producer_loop, NULL);
	//thread_create(&thread,activemq_producer_loop,NULL);
	std::thread([&]() {
		activemq_producer_loop(NULL);
	}).detach();
	s_loopStart = true;
	return 0;
}

bool AmqClient::activemq_produce(const char* msg)
{
	if (!s_init_success)
	{
		LOG_ERROR(MAIN_LOG,"{}", "mq not inited");
		return s_init_success;
	}

	{
		AutoThreadLocker locker(s_msg_locker);
		s_msg_list.push_back(msg);
		//event_signal(&s_msg_event);
		//s_msg_queue.push(msg);
	}

	//event_signal(&s_msg_event);

	return s_init_success;
}

AmqClient::AmqClient()
{
	s_loopStart = false;
	s_init_success = false;
}

AmqClient::~AmqClient()
{

}
