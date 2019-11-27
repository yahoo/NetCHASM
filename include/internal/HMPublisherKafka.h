// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMPUBLISHERKAFKA_H_
#define HMPUBLISHERKAFKA_H_
#include <iostream>
#include <set>
#include <vector>
#include <memory>
#include "HMPublisherProto.h"
#include "librdkafka/rdkafkacpp.h"

class DeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
  void dr_cb (RdKafka::Message &message);
};

class HMKafkaConfig
{
public:
    std::string brokers;
};
class HMPublisherKafka : public HMPublisherProto
{
public:
    ~HMPublisherKafka() {}
    HMPublisherKafka(HMKafkaConfig& config, std::string topic);
    //! This function is called to publish the check information to the publisher.
    /*
         This function is called to publish the check information to the publisher.
         \param pointer to data;
         \param size of data.
     */
    void publish(char* data, const size_t datalen) const;
    //! Return the registered topic
    const std::string& getTopic() const;
    //! Return the kafka config object
    const HMKafkaConfig& getConfig() const;

    std::unique_ptr<RdKafka::Producer> m_producer = nullptr;

private:
    //! This function is called to publish the check information to the publisher.
    /*
         This function is called to publish the check information to the publisher.
         \param pointer to data;
         \param size of data.
         \return Kafka error code.
     */
    RdKafka::ErrorCode produce(char* data, const size_t datalen) const;
    DeliveryReportCb m_dr_cb;
    std::string m_topic;
    HMKafkaConfig m_config;
};

#endif /* HMPUBLISHERBASE_H_ */
