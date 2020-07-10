// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.#ifndef TESTS_LIBRARYTESTS_TESTHMLIBRARY_H_
#ifndef TESTS_HMTCPCONNECTIONHANDLERSOCKET1_H_
#define TESTS_HMTCPCONNECTIONHANDLERSOCKET1_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMStateManager.h"


#define TESTNAME Test_HMTCPConnectionHandler1

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
    HMStateManager *sm;
    std::thread sm_thr;
    HMIPAddress server;
    HMDataHostCheck hostCheck, hostCheck1, hostCheck2;
    HMDataCheckParams checkParams, checkParams1, checkParams2;
    uint16_t port;
    HMHash hash;
    HMHash hash1;
    HMHash hash2;
    HMDataCheckResult checkResultg1h11;
    HMDataCheckResult checkResultg1h12;
    HMDataCheckResult checkResulth2;
    HMDataCheckResult checkResultg3h11;
    HMDataCheckResult checkResultg3h12;
    HMAuxInfo auxInfo;
    HMAuxInfo auxInfo1;
    HMAuxInfo auxInfo2;
    HMAuxInfo emptyAuxInfo;
};
#endif /* TESTS_HMTCPCONNECTIONHANDLERSOCKET1_H_ */
