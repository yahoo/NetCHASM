// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMPUBSUBCONFIGPARSEYAML_H_
#define TEST_HMPUBSUBCONFIGPARSEYAML_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMStateManager.h"
#include "HMPubSubConfigParser.h"

#define TESTNAME Test_PubSubConfigParseYaml

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_config1_tests);
    CPPUNIT_TEST(test_config2_tests);
    CPPUNIT_TEST(test_config3_tests);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_config1_tests();
    void test_config2_tests();
    void test_config3_tests();
    const std::string singleKafka = "./singlekafka.yaml";
    const std::string multipleKafka = "./multiplekafka.yaml";
    const std::string noType = "./notype.yaml";
    const std::string invalidType = "./invalidtype.yaml";
    const std::string noBrokerKakfa = "./noBrokerKafka.yaml";
    const std::string noTopicKafka = "./noTopic.yaml";

    HMState currentState;
};

#endif /* TEST_HMPUBSUBCONFIGPARSEYAML_H_ */
