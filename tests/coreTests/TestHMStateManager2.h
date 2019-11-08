// Copyright (c) 2019 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.

#ifndef TEST_HMStateManager2_H_
#define TEST_HMStateManager2_H_
#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkQueue.h"

#define TESTNAME Test_HMStateManager

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_load_configs_backend);
    CPPUNIT_TEST(test_initShutdown);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void test_initShutdown();
    void test_load_configs_backend();
};

#endif /* TEST_HMStateManager2_H_ */

