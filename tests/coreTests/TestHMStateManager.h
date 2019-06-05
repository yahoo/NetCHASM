// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMStateManager_H_
#define TEST_HMStateManager_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkQueue.h"

#define TESTNAME Test_HMStateManager

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_statemanager);
    CPPUNIT_TEST(test_master_conf_parse);
    CPPUNIT_TEST(test_master_conf_parse1);
    CPPUNIT_TEST(test_set_state);
    CPPUNIT_TEST(test_config_parse);
    CPPUNIT_TEST(test_config_parse1);
    CPPUNIT_TEST(test_master_yaml_conf_parse);
    CPPUNIT_TEST(test_master_yaml_conf_parse1);
    CPPUNIT_TEST(test_load_configs_backend);
    CPPUNIT_TEST_SUITE_END();


public:

    void setUp();
    void tearDown();
    void test_statemanager();
    void test_master_conf_parse();
    void test_master_conf_parse1();
    void test_set_state();
    void test_config_parse();
    void test_config_parse1();
    void test_base_reload();
    void test_master_yaml_conf_parse();
    void test_master_yaml_conf_parse1();
    void test_load_configs_backend();
protected:

};

#endif /* TEST_HMStateManager_H_ */
 
