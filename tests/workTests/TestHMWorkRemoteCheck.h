// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TESTS_WORKTESTS_TESTHMWORKREMOTECHECK_H_
#define TESTS_WORKTESTS_TESTHMWORKREMOTECHECK_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <HMStorage.h>

#include "HMWorkRemoteCheck.h"
#include "HMWorkDNSLookup.h"
#include "HMWorkHealthCheck.h"
#include "HMStateManager.h"

#define TESTNAME Test_HMWorkRemoteCheck

class TestHMWorkRemoteCheck : public HMWorkRemoteCheck
{
public:
    TestHMWorkRemoteCheck(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& datahostCheck, const HMDataHostGroup& datahostGroup, const std::vector<HMGroupCheckResult>& results) :
            HMWorkRemoteCheck(hostname, ip, datahostCheck, datahostGroup) {responseTime = 0; m_results = results;};

    HM_WORK_STATUS remoteLookup();
    void init(HMWorkState& state) {};

    int responseTime;
};

class TestHMWorkDNSLookup : public HMWorkDNSLookup
{
public:
    TestHMWorkDNSLookup(const std::string& hostname, const HMIPAddress& ip, const HMDataHostCheck& hostcheck, const HMDNSLookup& dnsHostCheck) :
            HMWorkDNSLookup(hostname, ip, hostcheck, dnsHostCheck) {};

    void init(HMWorkState& state) {};
    HM_WORK_STATUS dnsLookup();
    void addTarget(HMIPAddress target);

    std::set<HMIPAddress> m_v6Targets;
    std::set<HMIPAddress> m_v4Targets;
};

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
    CPPUNIT_TEST(test_HMWorkRemoteCheck_basic);
    CPPUNIT_TEST(test_HMWorkRemoteCheck_mixed_dns);
    CPPUNIT_TEST(test_HMWorkRemoteCheck_mixed_healthcheck);
    CPPUNIT_TEST(test_HMWorkRemoteCheck_mixed_remote);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    TestHMWorkRemoteCheck* remoteCheck;
    TestHMWorkHealthCheck* healthCheck;
    TestHMWorkDNSLookup* dnsLookup;

    std::shared_ptr<HMState> m_currentState;
    HMStateManager m_state;
    HMEventLoopQueue* m_eventQueue;
    std::thread* m_eventThread;


protected:
    void test_HMWorkRemoteCheck_basic();
    void test_HMWorkRemoteCheck_mixed_dns();
    void test_HMWorkRemoteCheck_mixed_healthcheck();
    void test_HMWorkRemoteCheck_mixed_remote();
};
#endif /* TESTS_WORKTESTS_TESTHMWORKREMOTECHECK_H_ */
