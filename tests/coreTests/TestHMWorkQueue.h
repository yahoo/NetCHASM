// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMWorkQueue_H_
#define TEST_HMWorkQueue_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkQueue.h"

#define TESTNAME Test_HMWorkQueue

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_basic_workqueue);
    CPPUNIT_TEST(test_workqueue);
    CPPUNIT_TEST(test_notify_workqueue);
    CPPUNIT_TEST(test_shutdown_workqueue);
    CPPUNIT_TEST(test_multi_insert);
    CPPUNIT_TEST_SUITE_END();


public:

    void setUp();
    void tearDown();
    void test_basic_workqueue();
    void test_workqueue();
    void test_notify_workqueue();
    void test_shutdown_workqueue();
    void test_multi_insert();
protected:

};

#endif /* TEST_HMWorkQueue_H_ */
 
