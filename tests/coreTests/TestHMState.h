// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMState_H_
#define TEST_HMState_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkQueue.h"

#define TESTNAME Test_HMState

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_allchecks);
    CPPUNIT_TEST(test_allchecks1);
    CPPUNIT_TEST(test_allcheckResults);
    CPPUNIT_TEST(test_allcheckResults1);
    CPPUNIT_TEST(test_allcheckResults2);
    CPPUNIT_TEST(test_allcheckResults3);
    CPPUNIT_TEST(test_allcheckResults4);

    CPPUNIT_TEST_SUITE_END();


public:

    void setUp();
    void tearDown();
    void test_allchecks();
    void test_allchecks1();
    void test_allcheckResults();
    void test_allcheckResults1();
    void test_allcheckResults2();
    void test_allcheckResults3();
    void test_allcheckResults4();

protected:

};

#endif /* TEST_HMStateManager_H_ */
 
