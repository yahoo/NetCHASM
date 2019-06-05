// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_STORETESTS_TESTHMSTORAGE_H_
#define TESTS_STORETESTS_TESTHMSTORAGE_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMStorage.h"

#define TESTNAME Test_HMStorage

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMStorage_Construction);
    CPPUNIT_TEST(test_HMStorage_OpenClose);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_HMStorage_Construction();
    void test_HMStorage_OpenClose();

};
#endif /* TESTS_STORETESTS_TESTHMSTORAGE_H_ */
