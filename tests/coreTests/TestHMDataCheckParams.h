// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMDataCheckParams_H_
#define TEST_HMDataCheckParams_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkQueue.h"

#define TESTNAME Test_HMDataCheckParams

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_basic_datacheckparams);
    CPPUNIT_TEST(test_basic_query);
    CPPUNIT_TEST(test_basic_result);
    CPPUNIT_TEST(test_basic_result1);
    CPPUNIT_TEST(test_basic_result2);
    CPPUNIT_TEST(test_basic_result3);
    CPPUNIT_TEST(test_basic_result4);
    CPPUNIT_TEST(test_basic_result5);
    CPPUNIT_TEST(test_basic_result6);
    CPPUNIT_TEST(test_basic_retry);
    CPPUNIT_TEST(test_basic_flaps);
    CPPUNIT_TEST(test_nexttime_retry);
    CPPUNIT_TEST(test_nexttime_ttl);
    CPPUNIT_TEST(test_nexttime_immediate);
    CPPUNIT_TEST(test_dns_failed_result);
    CPPUNIT_TEST(test_response_200);
    CPPUNIT_TEST(test_retry_200);
    CPPUNIT_TEST_SUITE_END();


public:

    void setUp();
    void tearDown();
    void test_basic_datacheckparams();
    void test_basic_query();
    void test_basic_result();
    void test_basic_result1();
    void test_basic_result2();
    void test_basic_result3();
    void test_basic_result4();
    void test_basic_result5();
    void test_basic_result6();
    void test_basic_retry();
    void test_basic_flaps();
    void test_nexttime_retry();
    void test_nexttime_ttl();
    void test_nexttime_immediate();
    void test_dns_failed_result();
    void test_dns_no_valid_host();
    void test_response_200();
    void test_retry_200();
protected:

};

#endif /* TEST_HMDataCheckParams_H_ */
