// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMDNSCache_H_
#define TEST_HMDNSCache_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkDNSLookup.h"
#include "HMStateManager.h"
#include "HMDNSCache.h"
#include "HMEventLoopQueue.h"

#define TESTNAME Test_HMDNSCache

class TESTNAME: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_basic_cache);
    CPPUNIT_TEST(test_update_results);
    CPPUNIT_TEST(test_update_results_ipv4s);
    CPPUNIT_TEST(test_get_expired_ipv4s);
    CPPUNIT_TEST(test_update_results_ipv6s);
    CPPUNIT_TEST(test_get_expired_ipv6s);
    CPPUNIT_TEST(test_get_expired_ipv4_prefered);
    CPPUNIT_TEST(test_get_expired_ipv6_prefered);
    CPPUNIT_TEST(test_update_results_dualstack);
    CPPUNIT_TEST(test_update_results_failed_ipv4);
    CPPUNIT_TEST(test_update_results_failed_ipv6);
    CPPUNIT_TEST(test_update_results_failed_dualstack);
    CPPUNIT_TEST(test_update_results_dualstack_ipv4);
    CPPUNIT_TEST(test_update_results_dualstack_ipv6);
    CPPUNIT_TEST(test_update_results_failure);
    CPPUNIT_TEST(test_update_results_dualstack_v4);
    CPPUNIT_TEST(test_update_results_dualstack_v6);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void test_basic_cache();
    void test_update_results();
    void test_update_results_ipv4s();
    void test_get_expired_ipv4s();
    void test_update_results_ipv6s();
    void test_get_expired_ipv6s();
    void test_get_expired_ipv4_prefered();
    void test_get_expired_ipv6_prefered();
    void test_update_results_dualstack();
    void test_update_results_failed_ipv4();
    void test_update_results_failed_ipv6();
    void test_update_results_failed_dualstack();
    void test_update_results_dualstack_ipv4();
    void test_update_results_dualstack_ipv6();
    void test_update_results_failure();
    void test_update_results_dualstack_v4();
    void test_update_results_dualstack_v6();
protected:

};

#endif /* TEST_HMDNSCache_H_ */

