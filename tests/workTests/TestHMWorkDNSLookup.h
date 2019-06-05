// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_WORKTESTS_TESTHMWORKDNSLOOKUP_H_
#define TESTS_WORKTESTS_TESTHMWORKDNSLOOKUP_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "HMWorkDNSLookup.h"
#include "HMStateManager.h"
#include "HMDNSCache.h"
#include "HMEventLoopQueue.h"

#define TESTNAME Test_HMWorkDNSLookup

class TestHMWorkDNSLookup : public HMWorkDNSLookup
{
public:
    TestHMWorkDNSLookup(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck) :
            HMWorkDNSLookup(hostname, ip, hostcheck) {};

    void init(HMWorkState& state) {};
    HM_WORK_STATUS dnsLookup();
    void addTarget(HMIPAddress target);

    std::set<HMIPAddress> m_v6Targets;
    std::set<HMIPAddress> m_v4Targets;
};

class TESTNAME : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_HMWorkDNSLookup_Process_IPV4);
    CPPUNIT_TEST(test_HMWorkDNSLookup_Process_IPV6);
    CPPUNIT_TEST(test_HMWorkDNSLookup_Process_IPV4_6);
    CPPUNIT_TEST(test_HMWorkDNSLookup_DnsFailed);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    TestHMWorkDNSLookup* dnsLookup;

    std::shared_ptr<HMState> m_currentState;
    HMStateManager m_state;
    HMEventLoopQueue* m_eventQueue;
    std::thread* m_eventThread;


protected:
    void test_HMWorkDNSLookup_Process_IPV4();
    void test_HMWorkDNSLookup_Process_IPV6();
    void test_HMWorkDNSLookup_Process_IPV4_6();
    void test_HMWorkDNSLookup_DnsFailed();
};

#endif /* TESTS_WORKTESTS_TESTHMWORKDNSLOOKUP_H_ */
