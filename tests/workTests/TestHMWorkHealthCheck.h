// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_WORKTESTS_TESTHMWORKHEALTHCHECK_H_
#define TESTS_WORKTESTS_TESTHMWORKHEALTHCHECK_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <HMStorage.h>

#include "HMWorkHealthCheck.h"
#include "HMStateManager.h"

#define TESTNAME Test_HMWorkHealthCheck

class TestHMWorkHealthCheck : public HMWorkHealthCheck
{
public:
    TestHMWorkHealthCheck(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
            HMWorkHealthCheck(hostname, ip, hostcheck) {responseTime = 0;};

    HM_WORK_STATUS healthCheck();
    void init(HMWorkState& state) {};

    int responseTime;
};

class TESTNAME : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMWorkHealthCheck_IPV4);
    CPPUNIT_TEST(test_HMWorkHealthCheck_IPV6);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    TestHMWorkHealthCheck* healthCheck;

    std::shared_ptr<HMState> m_currentState;
    HMStateManager m_state;
    HMEventLoopQueue* m_eventQueue;
    std::thread* m_eventThread;


protected:
    void test_HMWorkHealthCheck_IPV4();
    void test_HMWorkHealthCheck_IPV6();
};
#endif /* TESTS_WORKTESTS_TESTHMWORKHEALTHCHECK_H_ */
