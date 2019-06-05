// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_WORKTESTS_TESTHMWORKAUXFETCH_H_
#define TESTS_WORKTESTS_TESTHMWORKAUXFETCH_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMWorkAuxFetch.h"
#include "HMStateManager.h"
#include "HMEventLoopQueue.h"
#include <HMStorage.h>

#define TESTNAME Test_HMWorkAuxFetch

class TestHMWorkAuxFetch : public HMWorkAuxFetch
{
public:
    TestHMWorkAuxFetch(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
        HMWorkAuxFetch(hostname, ip, hostcheck) {};

    void init(HMWorkState& state) {};
    HM_WORK_STATUS fetchAux();

};

class TESTNAME : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMWorkAuxFetch_Process_Success);
    CPPUNIT_TEST(test_HMWorkAuxFetch_Process_Timeout);
    CPPUNIT_TEST(test_HMWorkAuxFetch_Process_Failure);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    TestHMWorkAuxFetch* fetchAux;

    std::shared_ptr<HMState> m_currentState;
    HMStateManager m_state;
    HMEventLoopQueue* m_eventQueue;
    std::thread* m_eventThread;

protected:
    void test_HMWorkAuxFetch_Process_Success();
    void test_HMWorkAuxFetch_Process_Timeout();
    void test_HMWorkAuxFetch_Process_Failure();
};

#endif /* TESTS_WORKTESTS_TESTHMWORKAUXFETCH_H_ */
