// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMCONTROLTLSSOCKET4_H_
#define TEST_HMCONTROLTLsSOCKET4_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMStateManager.h"


#define TESTNAME Test_HMControlTLSSocket4

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_cmdlstnr1);
    CPPUNIT_TEST(test_cmdlstnr2);
    CPPUNIT_TEST(test_cmdlstnr3);
    CPPUNIT_TEST(test_cmdlstnr4);
    CPPUNIT_TEST(test_cmdlstnr5);
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
    HMStateManager *sm;
    std::thread sm_thr;
    HMTimeStamp now;
    std::string host1 = "test.hm.com";
    std::string host2 = "test2.hm.com";
    std::string host3 = "test3.hm.com";
    HMAPIIPAddress server;
    uint16_t port;
    std::string certfile;
    std::string keyfile;
    std::string caFile;
    HMHash hash;
    HMHash hash1;
};
#endif /* TEST_HMCONTROLTLSSOCKET4_H_ */
