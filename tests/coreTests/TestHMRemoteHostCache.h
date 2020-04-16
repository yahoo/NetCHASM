// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMRemoteHostCache_H_
#define TEST_HMRemoteHostCache_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#define TESTNAME Test_HMRemoteHostCache

class TESTNAME: public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_basic_cache);
    CPPUNIT_TEST(test_basic_cache1);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void test_basic_cache();
    void test_basic_cache1();
protected:

};

#endif /* TEST_HMRemoteHostCache_H_ */

