// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_HMCONTROLTCPSOCKET_H_
#define TESTS_HMCONTROLTCPSOCKET_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMStateManager.h"


#define TESTNAME Test_HMControlTCPSocket

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_cmdlstnr1);
    CPPUNIT_TEST(test_cmdlstnr2);
    CPPUNIT_TEST(test_cmdlstnr3);
    CPPUNIT_TEST(test_cmdlstnr4);
    CPPUNIT_TEST(test_cmdlstnr5);
    CPPUNIT_TEST(test_cmdlstnr6);
    CPPUNIT_TEST(test_cmdlstnr7);
    CPPUNIT_TEST(test_cmdlstnr8);
    CPPUNIT_TEST(test_cmdlstnr9);
    CPPUNIT_TEST(test_cmdlstnr10);
    CPPUNIT_TEST(test_cmdlstnr11);
    CPPUNIT_TEST(test_cmdlstnr12);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_cmdlstnr1();
    void test_cmdlstnr2();
    void test_cmdlstnr3();
    void test_cmdlstnr4();
    void test_cmdlstnr5();
    void test_cmdlstnr6();
    void test_cmdlstnr7();
    void test_cmdlstnr8();
    void test_cmdlstnr9();
    void test_cmdlstnr10();
    void test_cmdlstnr11();
    void test_cmdlstnr12();
    HMStateManager *sm;
    std::thread sm_thr;
    HMAPIIPAddress server;
    uint16_t port;
};
#endif /* TESTS_HMCONTROLTCPSOCKET_H_ */
