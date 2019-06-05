// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMWorkDNSLookup.h"
#include "common.h"
#include "TestStorage.h"
using namespace std;
HM_WORK_STATUS TestHMWorkDNSLookup::dnsLookup()
{
    shared_ptr<HMState> currentState;
    m_stateManager->updateState(currentState);
    if(m_v4Targets.size() == 0 && m_v6Targets.size() == 0)
    {
        currentState->m_dnsCache.updateDNSEntry(m_hostname, false, m_v4Targets);
        currentState->m_dnsCache.updateDNSEntry(m_hostname, true, m_v6Targets);
        currentState->m_dnsCache.finishQuery(m_hostname, false, false);
        currentState->m_dnsCache.finishQuery(m_hostname, true, false);

        return HM_WORK_IDLE;
    }

    currentState->m_dnsCache.updateDNSEntry(m_hostname, false, m_v4Targets);
    currentState->m_dnsCache.updateDNSEntry(m_hostname, true, m_v6Targets);
    currentState->m_dnsCache.finishQuery(m_hostname, false, (m_v4Targets.size() > 0));
    currentState->m_dnsCache.finishQuery(m_hostname, true, (m_v6Targets.size() > 0));

    std::this_thread::sleep_for(50ms);
    return HM_WORK_COMPLETE;
}

void TestHMWorkDNSLookup::addTarget(HMIPAddress target)
{
    if(target.getType() == AF_INET)
    {
        m_v4Targets.insert(target);
    }
    else if(target.getType() == AF_INET6)
    {
        m_v6Targets.insert(target);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void
TESTNAME::setUp()
{
    setupCommon();
    HMDataHostGroupMap hostGroup;
    m_currentState = make_shared<HMState>();
    m_state.setState(m_currentState);
    m_currentState->m_datastore = make_unique<TestStorage>(&hostGroup);
    m_eventQueue = new HMEventLoopQueue(&m_state);
    m_eventThread = new std::thread(&HMEventLoopQueue::runThread, m_eventQueue);
}

void
TESTNAME::tearDown()
{
    m_eventQueue->shutDown();
    m_eventThread->join();

    m_currentState.reset();
    m_state.setState(m_currentState);

    delete m_eventQueue;
    delete m_eventThread;

    teardownCommon();
}

void
TESTNAME::test_HMWorkDNSLookup_Process_IPV4()
{
    // Setup everything required to conduct the DNS lookups and verify the correct tasks are generated
    HMDataHostCheck check1;
    HMDataHostCheck check2;
    HMIPAddress addr;
    addr.set("192.168.0.1");
    set<HMIPAddress> ips;
    ips.insert(addr);
    HMDataCheckParams params;
    string checkParams = "/check.html";
    check1.setCheckParams(HM_CHECK_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            checkParams);

    check2.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            checkParams);

    string success = "ipv4.success.hm.com";
    string timedout = "ipv4.timedout.hm.com";
    string failure = "ipv4.failure.hm.com";

    m_currentState->m_dnsWaitList.insert(pair<string,HMDataHostCheck>(success,check1));
    m_currentState->m_dnsWaitList.insert(pair<string,HMDataHostCheck>(success,check2));
    m_currentState->m_dnsWaitList.insert(pair<string,HMDataHostCheck>(timedout,check1));
    m_currentState->m_dnsWaitList.insert(pair<string,HMDataHostCheck>(timedout,check2));
    m_currentState->m_dnsWaitList.insert(pair<string,HMDataHostCheck>(failure,check1));
    m_currentState->m_dnsWaitList.insert(pair<string,HMDataHostCheck>(failure,check2));

    m_currentState->m_checkList.insertCheck("HostGroup1", success, check1, params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", success, check2, params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup1", timedout, check1, params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", timedout, check2, params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup1", failure, check1, params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", failure, check2, params, ips);

    // Basic ipv4 test to make sure the address gets added to the cache and timeout is updated
    HMDataHostCheck hostCheck;

    m_currentState->m_dnsCache.insertDNSEntry(success, false, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(timedout, false, 10, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(failure, false, 500, 60000);

    dnsLookup = new TestHMWorkDNSLookup(success, addr, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    addr.set("192.168.0.1");
    dnsLookup->addTarget(addr);
    set<HMIPAddress> vip_ret;
    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    CPPUNIT_ASSERT(m_currentState->m_dnsCache.getAddresses(success, HM_DUALSTACK_IPV4_ONLY, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(addr) != vip_ret.end());
    delete dnsLookup;

    // Test to make sure we do not return if the query is timed out
    dnsLookup = new TestHMWorkDNSLookup(timedout, addr, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    addr.set("192.168.1.1");
    dnsLookup->addTarget(addr);
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    CPPUNIT_ASSERT(m_currentState->m_dnsCache.getAddresses(timedout, HM_DUALSTACK_IPV4_ONLY, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(addr) != vip_ret.end());
    delete dnsLookup;

    // Test to make sure we do not return if the DNS looup fails
    dnsLookup = new TestHMWorkDNSLookup(failure, addr, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    addr.set("192.168.2.1");
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(failure, HM_DUALSTACK_IPV4_ONLY, vip_ret));
    CPPUNIT_ASSERT_EQUAL(0, (int)vip_ret.size());
    delete dnsLookup;

    // sleep and make sure all required work is present
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL(7, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check2));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == timedout);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "0.0.0.0");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == timedout);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.1.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check2));
    CPPUNIT_ASSERT(work->m_hostname == timedout);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.1.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "0.0.0.0");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == failure);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "0.0.0.0");
}

void
TESTNAME::test_HMWorkDNSLookup_Process_IPV6()
{
    // Setup everything required to conduct the DNS lookups and verify the correct tasks are generated
    HMDataHostCheck check1;
    HMDataHostCheck check2;
    HMDataCheckParams params;
    HMIPAddress addr(AF_INET6);
    addr.set("fd01::1");
    set<HMIPAddress> ips;
    ips.insert(addr);
    string checkParams = "/check.html";
    check1.setCheckParams(HM_CHECK_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 80,
    HM_DUALSTACK_IPV6_ONLY, checkParams);

    check2.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 443,
    HM_DUALSTACK_IPV6_ONLY, checkParams);

    string success = "ipv6.success.hm.com";
    string timedout = "ipv6.timedout.hm.com";
    string failure = "ipv6.failure.hm.com";

    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(success, check1));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(success, check2));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(timedout, check1));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(timedout, check2));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(failure, check1));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(failure, check2));

    m_currentState->m_checkList.insertCheck("HostGroup1", success, check1,
            params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", success, check2,
            params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup1", timedout, check1,
            params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", timedout, check2,
            params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup1", failure, check1,
            params, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", failure, check2,
            params, ips);

    // Basic ipv4 test to make sure the address gets added to the cache and timeout is updated
    HMDataHostCheck hostCheck;

    m_currentState->m_dnsCache.insertDNSEntry(success, true, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(timedout, true, 10, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(failure, true, 500, 60000);

    dnsLookup = new TestHMWorkDNSLookup(success, addr, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    addr.set("fd01::1");
    dnsLookup->addTarget(addr);

    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    set<HMIPAddress> vip_ret;
    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(success, HM_DUALSTACK_IPV6_ONLY, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(addr) != vip_ret.end());
    delete dnsLookup;

    // Test to make sure we do not return if the query is timed out
    dnsLookup = new TestHMWorkDNSLookup(timedout, addr, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    addr.set("fd01::2");
    dnsLookup->addTarget(addr);
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(timedout, HM_DUALSTACK_IPV6_ONLY, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(addr) != vip_ret.end());
    delete dnsLookup;

    // Test to make sure we do not return if the DNS looup fails
    dnsLookup = new TestHMWorkDNSLookup(failure, addr, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    addr.set("fd01::3");
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    CPPUNIT_ASSERT(
            !m_currentState->m_dnsCache.getAddresses(failure, HM_DUALSTACK_IPV6_ONLY, vip_ret));
    CPPUNIT_ASSERT_EQUAL(0, (int)vip_ret.size());
    delete dnsLookup;

    // sleep and make sure all required work is present
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL(7, (int )m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "fd01::1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check2));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "fd01::1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == timedout);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "::");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == timedout);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "fd01::2");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check2));
    CPPUNIT_ASSERT(work->m_hostname == timedout);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "fd01::2");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "::");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == failure);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "::");

}

void
TESTNAME::test_HMWorkDNSLookup_Process_IPV4_6()
{
    // Setup everything required to conduct the DNS lookups and verify the correct tasks are generated
    HMDataHostCheck checkV4Only;
    HMDataHostCheck checkV6Only;
    HMDataHostCheck checkBoth;

    // Set a small TTL to test the event scheduling
    HMDataCheckParams params;
    params.setCheckParameters(0, 0, 0,
    HM_DEFAULT_SMOOTHING_WINDOW,
    HM_DEFAULT_GROUP_THRESHOLD,
    HM_DEFAULT_SLOW_THRESHOLD,
    HM_DEFAULT_MAX_FLAPS, 100, 100, HM_DEFAULT_FLAP_THRESHOLD, 0);

    string checkParams = "/check.html";

    checkV4Only.setCheckParams(HM_CHECK_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 80,
    HM_DUALSTACK_IPV4_ONLY, checkParams);

    checkV6Only.setCheckParams(HM_CHECK_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 443,
    HM_DUALSTACK_IPV6_ONLY, checkParams);

    checkBoth.setCheckParams(HM_CHECK_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 85,
    HM_DUALSTACK_BOTH, checkParams);

    string v4Success = "ipv4.success.hm.com";
    string v6Success = "ipv6.success.hm.com";
    string bothSuccess = "both.success.hm.com";

    // First test to see what it returned if both types of address are present
    HMIPAddress addrv4_1(AF_INET);
    addrv4_1.set("192.168.0.1");
    HMIPAddress addrv4_2(AF_INET);
    addrv4_2.set("192.168.0.2");
    HMIPAddress addrv6_1(AF_INET6);
    addrv6_1.set("fd01::1");
    HMIPAddress addrv6_2(AF_INET6);
    addrv6_2.set("fd01::2");

    set<HMIPAddress> ipsv4;
    ipsv4.insert(addrv4_1);
    ipsv4.insert(addrv4_2);

    set<HMIPAddress> ipsv6;
    ipsv6.insert(addrv6_1);
    ipsv6.insert(addrv6_2);

    set<HMIPAddress> ipsboth;
    ipsboth.insert(addrv4_1);
    ipsboth.insert(addrv4_2);
    ipsboth.insert(addrv6_1);
    ipsboth.insert(addrv6_2);

    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(bothSuccess, checkV4Only));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(bothSuccess, checkV6Only));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(bothSuccess, checkBoth));

    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(v4Success, checkV4Only));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(v4Success, checkV6Only));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(v4Success, checkBoth));

    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(v6Success, checkV4Only));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(v6Success, checkV6Only));
    m_currentState->m_dnsWaitList.insert(
            pair<string, HMDataHostCheck>(v6Success, checkBoth));

    m_currentState->m_checkList.insertCheck("HostGroup1", bothSuccess,
            checkV4Only, params, ipsv4);
    m_currentState->m_checkList.insertCheck("HostGroup2", bothSuccess,
            checkV6Only, params, ipsv4);
    m_currentState->m_checkList.insertCheck("HostGroup3", bothSuccess,
            checkBoth, params, ipsboth);
    m_currentState->m_checkList.insertCheck("HostGroup1", v4Success,
            checkV4Only, params, ipsv4);
    m_currentState->m_checkList.insertCheck("HostGroup2", v4Success,
            checkV6Only, params, ipsv6);
    m_currentState->m_checkList.insertCheck("HostGroup3", v4Success,
            checkBoth, params, ipsboth);
    m_currentState->m_checkList.insertCheck("HostGroup1", v6Success,
            checkV4Only, params, ipsv4);
    m_currentState->m_checkList.insertCheck("HostGroup2", v6Success,
            checkV6Only, params, ipsv6);
    m_currentState->m_checkList.insertCheck("HostGroup3", v6Success,
            checkBoth, params, ipsboth);

    m_currentState->m_dnsCache.insertDNSEntry(bothSuccess, true, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(bothSuccess, false, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(v4Success, true, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(v4Success, false, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(v6Success, true, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(v6Success, false, 500, 60000);

    // Test the event scheduling for a health check not needing immediate checking
    TestHMWorkDNSLookup testEventQueue(v6Success, addrv6_2, checkBoth);
    m_currentState->m_checkList.updateCheck(&testEventQueue,
            testEventQueue.m_hostCheck);

    HMDataHostCheck hostCheck;

    dnsLookup = new TestHMWorkDNSLookup(bothSuccess, addrv4_1, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    dnsLookup->addTarget(addrv4_1);
    dnsLookup->addTarget(addrv4_2);
    dnsLookup->addTarget(addrv6_1);
    dnsLookup->addTarget(addrv6_2);

    set<HMIPAddress> answer;

    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(bothSuccess, HM_DUALSTACK_IPV4_ONLY, answer));
    CPPUNIT_ASSERT(answer.size() == 2);
    CPPUNIT_ASSERT(answer.find(addrv4_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv4_2) != answer.end());
    answer.clear();

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(bothSuccess, HM_DUALSTACK_BOTH, answer));
    CPPUNIT_ASSERT(answer.size() == 4);
    CPPUNIT_ASSERT(answer.find(addrv4_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv4_2) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv6_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv6_2) != answer.end());

    answer.clear();

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(bothSuccess, HM_DUALSTACK_IPV6_ONLY, answer));
    CPPUNIT_ASSERT(answer.size() == 2);
    CPPUNIT_ASSERT(answer.find(addrv6_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv6_2) != answer.end());
    answer.clear();

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(bothSuccess, HM_DUALSTACK_BOTH, answer));
    CPPUNIT_ASSERT(answer.size() == 4);
    CPPUNIT_ASSERT(answer.find(addrv6_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv6_2) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv4_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv4_2) != answer.end());

    answer.clear();
    delete dnsLookup;

    // Test to make sure we do not return if the query is timed out
    dnsLookup = new TestHMWorkDNSLookup(v4Success, addrv4_1, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    dnsLookup->addTarget(addrv4_1);
    dnsLookup->addTarget(addrv4_2);

    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(v4Success, HM_DUALSTACK_IPV4_ONLY, answer));
    CPPUNIT_ASSERT(answer.size() == 2);
    CPPUNIT_ASSERT(answer.find(addrv4_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv4_2) != answer.end());
    answer.clear();

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(v4Success, HM_DUALSTACK_BOTH, answer));
    CPPUNIT_ASSERT(answer.size() == 2);
    CPPUNIT_ASSERT(answer.find(addrv4_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv4_2) != answer.end());
    answer.clear();

    CPPUNIT_ASSERT(
            !(m_currentState->m_dnsCache.getAddresses(v4Success, HM_DUALSTACK_IPV6_ONLY, answer)));
    CPPUNIT_ASSERT(answer.size() == 0);

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(v4Success, HM_DUALSTACK_BOTH, answer));
    CPPUNIT_ASSERT(answer.size() == 2);
    CPPUNIT_ASSERT(answer.find(addrv4_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv4_2) != answer.end());
    answer.clear();
    delete dnsLookup;

    // Test to make sure we do not return if the DNS looup fails
    dnsLookup = new TestHMWorkDNSLookup(v6Success, addrv6_1, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);

    dnsLookup->addTarget(addrv6_1);
    dnsLookup->addTarget(addrv6_2);

    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    answer.clear();
    CPPUNIT_ASSERT(
            !(m_currentState->m_dnsCache.getAddresses(v6Success, HM_DUALSTACK_IPV4_ONLY, answer)));
    CPPUNIT_ASSERT(answer.size() == 0);
    answer.clear();
    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(v6Success, HM_DUALSTACK_BOTH, answer));
    CPPUNIT_ASSERT(answer.size() == 2);
    CPPUNIT_ASSERT(answer.find(addrv6_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv6_2) != answer.end());
    answer.clear();

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(v6Success, HM_DUALSTACK_IPV6_ONLY, answer));
    CPPUNIT_ASSERT(answer.size() == 2);
    CPPUNIT_ASSERT(answer.find(addrv6_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv6_2) != answer.end());
    answer.clear();

    CPPUNIT_ASSERT(
            m_currentState->m_dnsCache.getAddresses(v6Success, HM_DUALSTACK_BOTH, answer));
    CPPUNIT_ASSERT(answer.size() == 2);
    CPPUNIT_ASSERT(answer.find(addrv6_1) != answer.end());
    CPPUNIT_ASSERT(answer.find(addrv6_2) != answer.end());
    answer.clear();
    delete dnsLookup;

    // sleep and make sure all required work is present
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL(15, (int )m_state.m_workQueue.queueSize());

    // Now check the work list
    // Health Checks for bothSuccess
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkV4Only));
    CPPUNIT_ASSERT(work->m_hostname == bothSuccess);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkV4Only));
    CPPUNIT_ASSERT(work->m_hostname == bothSuccess);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.2");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkBoth));
    CPPUNIT_ASSERT(work->m_hostname == bothSuccess);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkBoth));
    CPPUNIT_ASSERT(work->m_hostname == bothSuccess);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.2");

        // HealthChecks for v4Success
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkV4Only));
    CPPUNIT_ASSERT(work->m_hostname == v4Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkV4Only));
    CPPUNIT_ASSERT(work->m_hostname == v4Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.2");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkBoth));
    CPPUNIT_ASSERT(work->m_hostname == v4Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkBoth));
    CPPUNIT_ASSERT(work->m_hostname == v4Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.2");

    // HealthCheck for v6Success
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkV6Only));
    CPPUNIT_ASSERT(work->m_hostname == v6Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "fd01::1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkV6Only));
    CPPUNIT_ASSERT(work->m_hostname == v6Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "fd01::2");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkBoth));
    CPPUNIT_ASSERT(work->m_hostname == v6Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "fd01::1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != checkBoth));
    CPPUNIT_ASSERT(work->m_hostname == v6Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "fd01::2");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == bothSuccess);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "0.0.0.0");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == v4Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "0.0.0.0");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != HMDataHostCheck()));
    CPPUNIT_ASSERT(work->m_hostname == v6Success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "::");
}


void
TESTNAME::test_HMWorkDNSLookup_DnsFailed()
{
    std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> results;
    // Setup everything required to conduct the DNS lookups and verify the correct tasks are generated
    HMDataHostCheck check1;
    HMDataCheckParams params;
    string checkParams = "/check.html";
    check1.setCheckParams(HM_CHECK_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            checkParams);

    string failure = "ipv4.failure.hm.com";
    HMIPAddress addr;
    addr.set("0.0.0.0");
    set<HMIPAddress> ips;
    ips.insert(addr);
    m_currentState->m_dnsWaitList.insert(pair<string,HMDataHostCheck>(failure,check1));

    m_currentState->m_checkList.insertCheck("HostGroup1", failure, check1, params, ips);

    // Basic ipv4 test to make sure the address gets added to the cache and timeout is updated
    HMDataHostCheck hostCheck;
    HMTimeStamp time = HMTimeStamp::now();

    dnsLookup = new TestHMWorkDNSLookup(failure, addr, hostCheck);
    dnsLookup->updateState(&m_state, m_eventQueue);
    dnsLookup->m_response = HM_RESPONSE_DNS_FAILED;
    dnsLookup->m_reason = HM_REASON_DNS_NOTFOUND;
    dnsLookup->m_start = time;
    dnsLookup->m_end = time;
    dnsLookup->addTarget(addr);
    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    // sleep and make sure all required work is present
    CPPUNIT_ASSERT_EQUAL(true, m_currentState->m_checkList.getCheckResults(failure,check1,addr,results));
    CPPUNIT_ASSERT_EQUAL(1, (int)results.size());
    for( auto it = results.begin(); it != results.end(); ++it )
    {
        CPPUNIT_ASSERT_EQUAL((int)HM_RESPONSE_DNS_FAILED, (int)it->second.m_response);
        CPPUNIT_ASSERT_EQUAL((int)HM_REASON_DNS_NOTFOUND, (int)it->second.m_reason);
        CPPUNIT_ASSERT_EQUAL(time.getTimeSinceEpoch(), it->second.m_checkTime.getTimeSinceEpoch());
        CPPUNIT_ASSERT_EQUAL(time.getTimeSinceEpoch(), it->second.m_start.getTimeSinceEpoch());
        CPPUNIT_ASSERT_EQUAL(time.getTimeSinceEpoch(), it->second.m_end.getTimeSinceEpoch());
    }
    delete dnsLookup;
}
