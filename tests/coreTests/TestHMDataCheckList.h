// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMDataCheckList_H_
#define TEST_HMDataCheckList_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkQueue.h"

#define TESTNAME Test_HMDataCheckList

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_basic_datachecklist);
    CPPUNIT_TEST(test_basic_healthPlugins_tcp);
    CPPUNIT_TEST(test_basic_healthPlugins_remoteDisabled);
    CPPUNIT_TEST(test_basic_healthPlugins_remoteEnabled);
    CPPUNIT_TEST(test_basic_healthPlugins_ftp);
    CPPUNIT_TEST(test_basic_healthPlugins_dns);
    CPPUNIT_TEST(test_basic_healthPlugins_none);
    CPPUNIT_TEST(test_add_hostgroup);
    CPPUNIT_TEST(test_ip_dns_failed);
    CPPUNIT_TEST(test_hostgroup_distributed_fallback);
    CPPUNIT_TEST_SUITE_END();


public:

    void setUp();
    void tearDown();
    void test_basic_datachecklist();
    void test_basic_healthPlugins_tcp();
    void test_basic_healthPlugins_remoteDisabled();
    void test_basic_healthPlugins_remoteEnabled();
    void test_basic_healthPlugins_ftp();
    void test_basic_healthPlugins_dns();
    void test_basic_healthPlugins_none();
    void test_add_hostgroup();
    void test_hostgroup_distributed_fallback();
    void test_ip_dns_failed();
protected:

};

#endif /* TEST_HMDataCheckList_H_ */
 
