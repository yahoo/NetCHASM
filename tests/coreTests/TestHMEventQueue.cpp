// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMEventQueue.h"

#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "common.h"
#include <unistd.h>

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    setupCommon();

    m_currentState = make_shared<HMState>();
    m_state.setState(m_currentState);
    m_eventQueue = new HMEventLoopQueue(&m_state);
    m_eventThread = new std::thread(&HMEventLoopQueue::runThread, m_eventQueue);
}

void TESTNAME::tearDown()
{
    m_eventQueue->shutDown();
    m_eventThread->join();
    m_currentState.reset();
    m_state.setState(m_currentState);

    delete m_eventQueue;
    delete m_eventThread;

    teardownCommon();
}

void TESTNAME::test_basic_DNS_eventqueue()
{
    string dummy = "dummy.hm.com";
    HMDNSLookup dnsHostCheckF(HM_DNS_PLUGIN_STATIC, false);
    m_currentState->m_dnsCache.insertDNSEntry(dummy,dnsHostCheckF,300,3000);
    m_eventQueue->addDNSTimeout(dummy, dnsHostCheckF, HMTimeStamp::now());
    std::this_thread::sleep_for(2s);
    CPPUNIT_ASSERT_EQUAL(1, (int )m_state.m_workQueue.queueSize());
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == dummy);
}

void TESTNAME::test_basic_HC_eventqueue()
{
    string dummy = "dummy.hm.com";
    HMDataHostCheck check1;
    string checkParams = "dummy";
    HMDataCheckParams params;
    HMIPAddress ip;
    ip.set("192.168.1.1");
    set<HMIPAddress> ips;
    ips.insert(ip);
    params.setCheckParameters(0, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
            HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS,
            300, 3000, HM_DEFAULT_FLAP_THRESHOLD, 0);
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo(checkParams);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check1.setCheckParams(hostGroup);

    m_currentState->m_checkList.insertCheck("HostGroup1", dummy, check1,
            params,ips);
    m_eventQueue->addHealthCheckTimeout(dummy, ip, check1,
            HMTimeStamp::now());
    std::this_thread::sleep_for(2s);
    CPPUNIT_ASSERT_EQUAL(1, (int )m_state.m_workQueue.queueSize());
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == dummy);

}

void TESTNAME::test_ordering_HC_eventqueue()
{
    string dummy1 = "dummy1.hm.com";
    string dummy2 = "dummy2.hm.com";
    string dummy3 = "dummy3.hm.com";
    HMDataHostCheck check1;
    HMIPAddress ip0;
    ip0.set("192.168.1.0");
    HMIPAddress ip1;
    ip1.set("192.168.1.1");
    HMIPAddress ip2;
    ip2.set("192.168.1.2");

    set<HMIPAddress> ips0;
    ips0.insert(ip0);
    set<HMIPAddress> ips1;
    ips1.insert(ip1);
    set<HMIPAddress> ips2;
    ips2.insert(ip2);

    string checkParams = "dummy";
    HMDataCheckParams params, params1, params2;
    params.setCheckParameters(0, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
    HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS, 300,
            3000, HM_DEFAULT_FLAP_THRESHOLD, 0);
    params1.setCheckParameters(0, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
    HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS, 400,
            4000, HM_DEFAULT_FLAP_THRESHOLD, 0);
    params2.setCheckParameters(0, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
    HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS, 500,
            5000, HM_DEFAULT_FLAP_THRESHOLD, 0);
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo(checkParams);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check1.setCheckParams(hostGroup);

    HMTimeStamp t1 = HMTimeStamp::now() + 1000;
    HMTimeStamp t2 = t1 - 500;
    HMTimeStamp t3 = t2 - 1000;

    m_currentState->m_checkList.insertCheck("HostGroup1", dummy1, check1,
            params,ips0);
    m_currentState->m_checkList.insertCheck("HostGroup1", dummy2, check1,
            params,ips1);
    m_currentState->m_checkList.insertCheck("HostGroup1", dummy3, check1,
            params,ips2);
    m_eventQueue->addHealthCheckTimeout(dummy1, ip0, check1, t1);
    m_eventQueue->addHealthCheckTimeout(dummy2, ip1, check1, t2);
    m_eventQueue->addHealthCheckTimeout(dummy3, ip2, check1, t3);
    std::this_thread::sleep_for(2s);
    CPPUNIT_ASSERT_EQUAL(3, (int )m_state.m_workQueue.queueSize());
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == dummy3);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == dummy2);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == dummy1);

}

void TESTNAME::test_delay_HC_eventqueue()
{
    string dummy1 = "dummy1.hm.com";
    string dummy2 = "dummy2.hm.com";
    HMDataHostCheck check1;
    HMIPAddress ip;
    ip.set("192.168.1.1");
    HMIPAddress ip2;
    ip2.set("192.168.1.2");
    set<HMIPAddress> ips;
    ips.insert(ip);
    set<HMIPAddress> ips2;
    ips2.insert(ip2);
    string checkParams = "dummy";
    HMDataCheckParams params, params1;
    params.setCheckParameters(0, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
    HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS, 300,
            3000, HM_DEFAULT_FLAP_THRESHOLD, 0);
    params1.setCheckParameters(0, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
    HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS, 400,
            4000, HM_DEFAULT_FLAP_THRESHOLD, 0);
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo(checkParams);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check1.setCheckParams(hostGroup);
    HMTimeStamp t1 = HMTimeStamp::now() + 1000;
    HMTimeStamp t2 = t1 + 4000;

    m_currentState->m_checkList.insertCheck("HostGroup1", dummy1, check1,
            params,ips);
    m_currentState->m_checkList.insertCheck("HostGroup1", dummy2, check1,
            params1,ips2);
    m_eventQueue->addHealthCheckTimeout(dummy1, ip, check1, t1);
    m_eventQueue->addHealthCheckTimeout(dummy2, ip2, check1, t2);
    CPPUNIT_ASSERT_EQUAL(0, (int )m_state.m_workQueue.queueSize());
    std::this_thread::sleep_for(2s);
    CPPUNIT_ASSERT_EQUAL(1, (int )m_state.m_workQueue.queueSize());
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    std::this_thread::sleep_for(5s);
    CPPUNIT_ASSERT_EQUAL(2, (int )m_state.m_workQueue.queueSize());
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == dummy1);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == dummy2);
}
