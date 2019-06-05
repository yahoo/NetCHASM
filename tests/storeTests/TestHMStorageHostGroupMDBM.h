// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_STORETESTS_TESTHMSTORAGEHOSTGROUPMDBM_H_
#define TESTS_STORETESTS_TESTHMSTORAGEHOSTGROUPMDBM_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMStorageHostGroupMDBM.h"

#define TESTNAME Test_HMStorageHostGroupYForMDBM

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_Construction);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_TestRT);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_BackendTest);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_ConfigStoreRetrieve);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_HMLoadFile_StoreRetrieve);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_HMAuxOOB_StoreRetrieve);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_HMLoadObject_StoreRetrieve);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_ZeroIp);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_ClearBackend);
    CPPUNIT_TEST(test_HMStorageHostGroupYForMDBM_VersionChange);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

protected:
    void test_HMStorageHostGroupYForMDBM_Construction();
    void test_HMStorageHostGroupYForMDBM_TestRT();
    void test_HMStorageHostGroupYForMDBM_ConfigStoreRetrieve();
    void test_HMStorageHostGroupYForMDBM_HMLoadFile_StoreRetrieve();
    void test_HMStorageHostGroupYForMDBM_HMAuxOOB_StoreRetrieve();
    void test_HMStorageHostGroupYForMDBM_HMLoadObject_StoreRetrieve();
    void test_HMStorageHostGroupYForMDBM_ZeroIp();
    void test_HMStorageHostGroupYForMDBM_ClearBackend();
    void test_HMStorageHostGroupYForMDBM_VersionChange();
    void test_HMStorageHostGroupYForMDBM_BackendTest();
};

#endif /* TESTS_STORETESTS_TESTHMSTORAGEHOSTGROUPMDBM_H_ */
