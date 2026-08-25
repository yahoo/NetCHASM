// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMDATAPACKING_H_
#define TEST_HMDATAPACKING_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#define TESTNAME Test_HMDataPacking

/*!
    CPPUnit coverage for HMDataPacking hash validation paths.
 */
class TESTNAME : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(TestOversizedHashRejected);
    CPPUNIT_TEST(TestMismatchedHashRejected);
    CPPUNIT_TEST(TestNegativeHashSizeRejected);
    CPPUNIT_TEST_SUITE_END();

public:
    //! Common test setup.
    void setUp() override;
    //! Common test teardown.
    void tearDown() override;

protected:
    //! Reject payloads with hash sizes larger than the fixed buffer.
    void TestOversizedHashRejected();
    //! Reject payloads with declared sizes larger than the byte payload.
    void TestMismatchedHashRejected();
    //! Reject payloads with negative hash sizes.
    void TestNegativeHashSizeRejected();
};

#endif /* TEST_HMDATAPACKING_H_ */
