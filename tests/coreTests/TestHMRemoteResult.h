// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMRemoteResult_H_
#define TEST_HMRemoteResult_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMRemoteResult.h"


#define TESTNAME Test_HMRemoteResult

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_next_query);
    CPPUNIT_TEST(test_next_query1);
    CPPUNIT_TEST(test_next_query2);
    CPPUNIT_TEST(test_queue_query);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();
    void test_next_query();
    void test_next_query1();
    void test_next_query2();
    void test_queue_query();
protected:
};

#endif /* TEST_HMRemoteResult_H_ */
 
