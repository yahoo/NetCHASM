// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_STORETESTS_TESTHMSTORAGETEXT_H_
#define TESTS_STORETESTS_TESTHMSTORAGETEXT_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMStorageHostText.h"
#include "HMAuxCache.h"

#define TESTNAME Test_HMStorageText

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    //CPPUNIT_TEST(test_HMStorageText_StoreRetrieve);
    CPPUNIT_TEST(test_HMStorageText_ConfigStoreRetrieve);
    CPPUNIT_TEST(test_HMStorageText_AuxStoreRetrieve);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

protected:
    void test_HMStorageText_StoreRetrieve();
    void test_HMStorageText_ConfigStoreRetrieve();
    void test_HMStorageText_AuxStoreRetrieve();
};

#endif /* TESTS_STORETESTS_TESTHMSTORAGETEXT_H_ */
