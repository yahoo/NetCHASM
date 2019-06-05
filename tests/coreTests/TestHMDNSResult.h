// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMDNSResult_H_
#define TEST_HMDNSResult_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkDNSLookup.h"
#include "HMStateManager.h"
#include "HMDNSCache.h"
#include "HMEventLoopQueue.h"


#define TESTNAME Test_HMDNSResult

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_get_address);
    CPPUNIT_TEST(test_get_addresses);
    CPPUNIT_TEST(test_next_query);
    CPPUNIT_TEST(test_next_query1);
    CPPUNIT_TEST(test_next_query2);
    CPPUNIT_TEST(test_queue_query);
    CPPUNIT_TEST(test_get_exp_addresses);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();
    void test_get_address();
    void test_get_addresses();
    void test_next_query();
    void test_next_query1();
    void test_next_query2();
    void test_queue_query();
    void test_get_exp_addresses();
protected:
};

#endif /* TEST_HMEventQueue_H_ */
 
