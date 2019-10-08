// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef CONFIGPARSEYAML_H_
#define CONFIGPARSEYAML_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMStateManager.h"
#include "HMConfigParserYaml.h"

#define TESTNAME Test_ConfigParseYaml

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_config1_tests);
    CPPUNIT_TEST(test_config2_tests);
    CPPUNIT_TEST(test_config3_tests);
    CPPUNIT_TEST(test_config4_tests);
    CPPUNIT_TEST(test_config5_tests);
    CPPUNIT_TEST(test_config6_tests);
    CPPUNIT_TEST(test_config7_tests);
    CPPUNIT_TEST(test_config8_tests);
    CPPUNIT_TEST(test_config9_tests);
    CPPUNIT_TEST(test_config10_tests);
    CPPUNIT_TEST(test_config11_tests);
    CPPUNIT_TEST(test_config12_tests);
    CPPUNIT_TEST(test_config13_tests);
    CPPUNIT_TEST(test_config14_tests);
    CPPUNIT_TEST(test_config15_tests);
    CPPUNIT_TEST(test_config_http_tests);
    CPPUNIT_TEST(test_config_https_tests);
    CPPUNIT_TEST(test_config_neg_tests);
    CPPUNIT_TEST(test_config_garbage_tests);
    CPPUNIT_TEST(test_indirect_hosts);
    CPPUNIT_TEST(test_indirect_hosts1);
    CPPUNIT_TEST(test_config18_tests);
    CPPUNIT_TEST(test_config19_tests);
    CPPUNIT_TEST(test_neg_configs);
    CPPUNIT_TEST(test_write_configs);
    CPPUNIT_TEST(test_write_configs1);
    CPPUNIT_TEST(test_config20_tests);
    CPPUNIT_TEST(test_write_configs2);    
    CPPUNIT_TEST(test_config_mark_http_tests);
    CPPUNIT_TEST(test_config_mark_https_tests);
    CPPUNIT_TEST(test_config_mark_https_no_peer_tests);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_config1_tests();
    void test_config2_tests();
    void test_config3_tests();
    void test_config4_tests();
    void test_config5_tests();
    void test_config6_tests();
    void test_config7_tests();
    void test_config8_tests();
    void test_config9_tests();
    void test_config10_tests();
    void test_config11_tests();
    void test_config12_tests();
    void test_config13_tests();
    void test_config14_tests();
    void test_config15_tests();
    void test_config_http_tests();
    void test_config_https_tests();
    void test_config_neg_tests();
    void test_config_garbage_tests();
    void test_indirect_hosts();
    void test_indirect_hosts1();
    void test_config18_tests();
    void test_config19_tests();
    void test_neg_configs();
    void test_write_configs();
    void test_write_configs1();
    void test_config20_tests();
    void test_write_configs2();
    void test_config_mark_http_tests();
    void test_config_mark_https_tests();
    void test_config_mark_https_no_peer_tests();
    const std::string folderLocation = "./yamlconfig";
    const std::string folderLocation1 = "./yamlconfig1";
    const std::string fileLocation = "./yamlconfig/testconf.yaml";
    const std::string fileLocation1 = "./yamlconfig/testconf1.yaml";
    const std::string fileLocation2 = "./yamlconfig/testconf2.yaml";
    const std::string garbageLocation = "./badyamlconfig";
    const std::string garbageConfig = "./badyamlconfig/testconf.yaml";
    const std::string wrongConfig = "./badyamlconfig/testconf1.yaml";
    const std::string fileLocation3 = "./yamlconfig1/testconf1.yaml";
    const std::string fileLocation4 = "./yamlconfig1/testconf2.yaml";

    HMState currentState;
};

#endif /* CONFIGPARSEYAML_H_ */
