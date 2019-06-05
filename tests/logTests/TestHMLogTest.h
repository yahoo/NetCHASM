// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMLOGTEST_H_
#define TEST_HMLOGTEST_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMLogText.h"
#include "HMLogBase.h"

#define TESTNAME Test_HMLogTest

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_logbase);
    CPPUNIT_TEST(test_syslog_level);
    CPPUNIT_TEST(test_syslog_called);
    CPPUNIT_TEST(test_syslog_notcalled);
    CPPUNIT_TEST_SUITE_END();


public:

    void setUp();
    void tearDown();
    void test_logbase();
    void test_syslog_level();
    void test_syslog_called();
    void test_syslog_notcalled();
protected:

};

void vsyslog(int priority, const char *format, va_list ap);

#endif /* TEST_HMLOGTEST_H_ */
 
