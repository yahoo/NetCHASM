// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_CORETESTS_TESTHMAUXINFO_H_
#define TESTS_CORETESTS_TESTHMAUXINFO_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#define TESTNAME Test_HMAuxInfo

class TESTNAME : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMAuxInfo_MalformedXML);
    CPPUNIT_TEST(test_HMAuxInfo_NewLFB);
    CPPUNIT_TEST(test_HMAuxInfo_OOB);
    CPPUNIT_TEST(test_parse_HMAuxInfo_NewLFB);
    CPPUNIT_TEST(test_parse_HMAuxInfo_OOB);
    CPPUNIT_TEST(test_neg_HMAuxInfo_OOB);
    CPPUNIT_TEST(test_neg_HMAuxInfo_NewLFB);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_HMAuxInfo_core();
    void test_HMAuxInfo_MalformedXML();
    void test_HMAuxInfo_NewLFB();
    void test_HMAuxInfo_OOB();
    void test_parse_HMAuxInfo_NewLFB();
    void test_parse_HMAuxInfo_OOB();
    void test_neg_HMAuxInfo_OOB();
    void test_neg_HMAuxInfo_NewLFB();
};

#endif /* TESTS_CORETESTS_TESTHMAUXINFO_H_ */
