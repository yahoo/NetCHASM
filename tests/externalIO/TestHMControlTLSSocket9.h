// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_HMCONTROLTLSSOCKET9_H_
#define TESTS_HMCONTROLTLSSOCKET9_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMStateManager.h"


#define TESTNAME Test_HMControlTLSSocket9

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_cmdlstnr1);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_cmdlstnr1();
    HMStateManager *sm;
    std::thread sm_thr;
    HMAPIIPAddress server;
    uint16_t port;
    std::string certfile;
    std::string keyfile;
    std::string caFile;
};
#endif /* TESTS_HMCONTROLTLSSOCKET6_H_ */
