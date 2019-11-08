// Copyright (c) 2019 Arista Networks, Inc.  All rights reserved.
// Arista Networks, Inc. Confidential and Proprietary.

#ifndef TESTS_STORETESTS_TESTHMSTORAGENOTIFIER_H_
#define TESTS_STORETESTS_TESTHMSTORAGENOTIFIER_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMStorageObserver.h"
#include "HMConstants.h"
#include "HMStorageHostNotifier.h"
#define TESTNAME Test_HMStorageNotifier

class ObserverText : public HMStorageObserver
{
public:
   ObserverText() {};
   virtual ~ObserverText() {};
   virtual bool storeHostCheckResult(HMCheckData& checkData) {
       std::string up = checkData.m_result.m_status == HM_HOST_STATUS_UP ? "up" : "down";
       m_text = checkData.m_checkParams.printHostGroups() + "\t"
          + checkData.m_hostname + "\t" + up;
       return true;
   }

   std::string m_text;
};

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMStorageNotifier_storeCheckResult);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
protected:
    void test_HMStorageNotifier_storeCheckResult();
};

#endif /* TESTS_STORETESTS_TESTHMSTORAGENOTIFIER_H_ */
