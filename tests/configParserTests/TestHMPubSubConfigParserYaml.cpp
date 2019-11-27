// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <string>
#include <fstream>
#include <sys/stat.h>

#include "common.h"
#include "HMConfigParserYaml.h"
#include "HMDataHostGroup.h"
#include "HMDataCheckParams.h"
#include "HMPublisherKafka.h"
#include "TestHMPubSubConfigParserYaml.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void
TESTNAME::setUp()
{
    setupCommon();

    ofstream fsinglekafka(singleKafka);
    fsinglekafka << "-   name: kafka1\n\
    type: kafka\n\
    parameters:\n\
        brokerlist:\n\
            - 192.168.1.1:9092\n\
        topic: test\n\
    \n";
    fsinglekafka.close();

    ofstream fmultiplekafka(multipleKafka);
    fmultiplekafka << "-   name: kafka1\n\
    type: kafka\n\
    parameters:\n\
        brokerlist:\n\
            - 192.168.1.1:9092\n\
        topic: test\n\
-   name: kafka2\n\
    type: kafka\n\
    hostgroups:\n\
        - hg1\n\
        - hg2\n\
    onchange: true\n\
    parameters:\n\
        topic: test1\n\
        brokerlist:\n\
            - 192.168.1.1:9092\n\
            - 192.168.1.1:9093\n\
        \n";
    fmultiplekafka.close();

    ofstream fnotype(noType);
    fnotype << "-   name: kafka1\n\
    parameters:\n\
        brokerlist:\n\
            - 192.168.1.1:9092\n\
        topic: test\n\
\n";
    fnotype.close();

    ofstream finvalidType(invalidType);
    finvalidType << "-   name: kafka1\n\
    type: invalid\n\
    parameters:\n\
        brokerlist:\n\
            - 192.168.1.1:9092\n\
        topic: test\n\
    \n";
    finvalidType.close();

    ofstream fnoBroker(noBrokerKakfa);
    fnoBroker << "-   name: kafka1\n\
    type: kafka\n\
    parameters:\n\
        topic: test\n\
    \n";
    fnoBroker.close();

    ofstream fnoTopic(noTopicKafka);
    fnoTopic << "-   name: kafka1\n\
    type: kafka\n\
    parameters:\n\
        brokerlist:\n\
            - 192.168.1.1:9092\n\
    \n";
    fnoTopic.close();

}
void
TESTNAME::tearDown()
{
    teardownCommon();
    remove(singleKafka.c_str());
    remove(multipleKafka.c_str());
    remove(noType.c_str());
    remove(invalidType.c_str());
    remove(noBrokerKakfa.c_str());
    remove(noTopicKafka.c_str());
}

void
TESTNAME::test_config1_tests()
{
    HMPubSubConfigParser parser;
    CPPUNIT_ASSERT(!parser.parseConfig(singleKafka, currentState));
    CPPUNIT_ASSERT_EQUAL(1, (int)currentState.m_resultPublisher.publishersCount());
    HMPublisherKafka* publisher = (HMPublisherKafka*)currentState.m_resultPublisher.getpublisher("kafka");
    CPPUNIT_ASSERT(!publisher);
    publisher = (HMPublisherKafka*)currentState.m_resultPublisher.getpublisher("kafka1");
    CPPUNIT_ASSERT(publisher);
    CPPUNIT_ASSERT(publisher->isPublishAll());
    CPPUNIT_ASSERT(!publisher->isPublishOnChange());
    CPPUNIT_ASSERT(publisher->getTopic() =="test");
    CPPUNIT_ASSERT(publisher->getConfig().brokers =="192.168.1.1:9092");
}

void
TESTNAME::test_config2_tests()
{
    HMPubSubConfigParser parser;
    CPPUNIT_ASSERT(!parser.parseConfig(multipleKafka, currentState));
    CPPUNIT_ASSERT_EQUAL(2, (int)currentState.m_resultPublisher.publishersCount());
    HMPublisherKafka* publisher = (HMPublisherKafka*)currentState.m_resultPublisher.getpublisher("kafka");
    CPPUNIT_ASSERT(!publisher);
    publisher = (HMPublisherKafka*)currentState.m_resultPublisher.getpublisher("kafka1");
    CPPUNIT_ASSERT(publisher);
    CPPUNIT_ASSERT(publisher->isPublishAll());
    CPPUNIT_ASSERT(!publisher->isPublishOnChange());
    CPPUNIT_ASSERT(publisher->getTopic() =="test");
    CPPUNIT_ASSERT(publisher->getConfig().brokers =="192.168.1.1:9092");
    publisher = (HMPublisherKafka*) currentState.m_resultPublisher.getpublisher(
            "kafka2");
    CPPUNIT_ASSERT(publisher);
    CPPUNIT_ASSERT(!publisher->isPublishAll());
    CPPUNIT_ASSERT(publisher->isPublishOnChange());
    CPPUNIT_ASSERT(publisher->getTopic() == "test1");
    CPPUNIT_ASSERT(publisher->getConfig().brokers == "192.168.1.1:9092,192.168.1.1:9093");
    CPPUNIT_ASSERT(
            publisher->getHostGroups().find("hg1")
                    != publisher->getHostGroups().end());
    CPPUNIT_ASSERT(
                publisher->getHostGroups().find("hg2")
                        != publisher->getHostGroups().end());

}

void
TESTNAME::test_config3_tests()
{
    HMPubSubConfigParser parser;
    CPPUNIT_ASSERT(parser.parseConfig(noType, currentState));
    CPPUNIT_ASSERT(parser.parseConfig(invalidType, currentState));
    CPPUNIT_ASSERT(parser.parseConfig(noTopicKafka, currentState));
    CPPUNIT_ASSERT(parser.parseConfig(noBrokerKakfa, currentState));
}
