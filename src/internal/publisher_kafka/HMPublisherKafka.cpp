// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.

#include "HMPublisherKafka.h"
#include "HMLogBase.h"

using namespace std;


void DeliveryReportCb::dr_cb(RdKafka::Message &message)
{
    /* If message.err() is non-zero the message delivery failed permanently
     * for the message. */
    if (message.err())
    {
        HMLog(HM_LOG_ERROR, "Message delivery failed: %s", message.errstr().c_str());
    }
    else
    {
        HMLog(HM_LOG_DEBUG,"Message delivered to topic %s: partition:%d: Offset:%ll", message.topic_name().c_str(),message.partition(), message.offset());
    }
}

HMPublisherKafka::HMPublisherKafka(HMKafkaConfig& config, string topic) : m_topic(topic), m_config(config)
{
    unique_ptr<RdKafka::Conf> confptr;
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    confptr.reset(conf);
    string errstr;
    if(confptr->set("metadata.broker.list", m_config.brokers, errstr) != RdKafka::Conf::CONF_OK)
    {
        HMLog(HM_LOG_CRITICAL, "Failed to add brokers to the configuration: %s",
                errstr.c_str());
    }

    if (conf->set("dr_cb", &m_dr_cb, errstr) != RdKafka::Conf::CONF_OK)
    {
        HMLog(HM_LOG_CRITICAL,
                "Failed to register callbacks to the configuration: %s",
                errstr.c_str());
    }

    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer)
    {
        HMLog(HM_LOG_CRITICAL, "Failed to create producer: %s", errstr.c_str());
    }
    else
    {
        m_producer.reset(producer);
    }
}

const std::string& HMPublisherKafka::getTopic() const
{
    return m_topic;
}

const HMKafkaConfig& HMPublisherKafka::getConfig() const
{
    return m_config;
}

RdKafka::ErrorCode
HMPublisherKafka::produce(char* data, const size_t datalen) const
{
    if(!m_producer)
    {
        HMLog(HM_LOG_CRITICAL, "Invalid producer");
        return RdKafka::ErrorCode::ERR_INVALID_REQUEST;
    }
    return m_producer->produce(m_topic, RdKafka::Topic::PARTITION_UA,
            RdKafka::Producer::RK_MSG_COPY, data, datalen,
            NULL, 0, 0, NULL);
}

void HMPublisherKafka::publish(char* data, const size_t datalen) const
{
    if (!m_producer)
    {
        HMLog(HM_LOG_CRITICAL, "Invalid producer");
        return;
    }
    RdKafka::ErrorCode err = produce(data, datalen);
    if (err != RdKafka::ERR_NO_ERROR)
    {
        if (err == RdKafka::ERR__QUEUE_FULL)
        {
            m_producer->poll(1000);
            err = produce(data, datalen);
        }
    }

    if(err == RdKafka::ERR_NO_ERROR)
    {
        HMLog(HM_LOG_DEBUG, "Enqueued message for data");
    }
    else
    {
        HMLog(HM_LOG_ERROR, "Failed to produce to topic %s : %s", m_topic.c_str(), RdKafka::err2str(err).c_str());
    }
    m_producer->flush(10000);
}
