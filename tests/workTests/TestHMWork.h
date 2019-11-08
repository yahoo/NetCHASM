// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMWORK_H_
#define TEST_HMWORK_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMWork.h"

#define TESTNAME Test_HMWork

// Minimal required functionality to test base class
class TestHMWork : public HMWork
{
public:
    TestHMWork() :
        HMWork() {};

    TestHMWork(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWork(hostname, ip, hostcheck) {};
    HM_WORK_TYPE getWorkType() { return HM_WORK_HEALTHCHECK; }
    HM_WORK_STATUS processWork() {return HM_WORK_COMPLETE;}
    void init(HMWorkState& state) {}

    const HMStateManager* getStateManager() {return m_stateManager;}

};

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMWork_Construction);
    CPPUNIT_TEST(test_HMWork_updateState);
#ifdef USE_ARES
    CPPUNIT_TEST(test_HMWorkState_Construction);
    CPPUNIT_TEST(test_HMWorkReloadState);
#endif
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp();
    void tearDown();

protected:
    void test_HMWork_Construction();
    void test_HMWork_updateState();
    void test_HMWork_setThreadID();
    void test_HMWorkState_Construction();
    void test_HMWorkReloadState();
};

#endif // TEST_MDBMSTORE_H_

