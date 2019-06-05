// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_STORETESTS_TESTHMSTORAGEHOSTGROUP_H_
#define TESTS_STORETESTS_TESTHMSTORAGEHOSTGROUP_H_

#include <set>
#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMStorageHostGroup.h"

#define TESTNAME Test_HMStorageHostGroup

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMStorageHostGroup_ReadOnly);
    CPPUNIT_TEST(test_HMStorageHostGroup_ReadWrite);
    CPPUNIT_TEST(test_HMStorageHostGroup_Restore);
    CPPUNIT_TEST(test_HMStorageHostGroup_InternalFunctions);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_HMStorageHostGroup_Construction();
    void test_HMStorageHostGroup_ReadOnly();
    void test_HMStorageHostGroup_ReadWrite();
    void test_HMStorageHostGroup_Restore();
    void test_HMStorageHostGroup_InternalFunctions();
    void test_HMStorageHostGroup_ZeroIp();
};

#endif /* TESTS_STORETESTS_TESTHMSTORAGEHOSTGROUP_H_ */
