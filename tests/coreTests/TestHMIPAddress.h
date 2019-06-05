// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTHMIPADDRESS_H_
#define TESTHMIPADDRESS_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#define TESTNAME Test_HMIPAddress

class TESTNAME : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_v4_sets);
    CPPUNIT_TEST(test_v6_sets);
    CPPUNIT_TEST(test_comparisonsv4);
    CPPUNIT_TEST(test_comparisonsv6);
    CPPUNIT_TEST(test_printing);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_v4_sets();
    void test_v6_sets();
    void test_comparisonsv4();
    void test_comparisonsv6();
    void test_printing();
};

#endif /* TESTHMIPADDRESS_H_ */
