// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTTIMESTAMP_H_
#define TESTTIMESTAMP_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMTimeStamp.h"

#define TESTNAME Test_TimeStamp

class TESTNAME : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_timestamp1);
    CPPUNIT_TEST(test_timestamp2);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_timestamp1();
    void test_timestamp2();
};

#endif /* TESTTIMESTAMP_H_ */
