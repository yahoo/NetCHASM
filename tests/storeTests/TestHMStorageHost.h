// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_STORETESTS_TESTHMSTORAGEHOST_H_
#define TESTS_STORETESTS_TESTHMSTORAGEHOST_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMStorageHost.h"
#include "TestStorageHost.h"
#define TESTNAME Test_HMStorageHost

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMStorageHost_ReadOnly);
    CPPUNIT_TEST(test_HMStorageHost_ReadWrite);
    //CPPUNIT_TEST(test_HMStorageHost_Restore);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_HMStorageHost_ReadOnly();
    void test_HMStorageHost_ReadWrite();
    void test_HMStorageHost_Restore();
};

#endif /* TESTS_STORETESTS_TESTHMSTORAGEHOST_H_ */
