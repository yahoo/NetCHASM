// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <thread>

#include "HMEventLoopQueue.h"
#include "TestHMWorkRemoteHostCheck.h"
#include "common.h"
#include "TestStorage.h"

using namespace std;

HM_WORK_STATUS
TestHMWorkRemoteHostCheck::remoteLookup()
{
    if(responseTime < 0)
    {
        return HM_WORK_COMPLETE;
    }
    m_response = (responseTime?HM_RESPONSE_CONNECTED:HM_RESPONSE_FAILED);
    m_reason = (responseTime?HM_REASON_SUCCESS:HM_REASON_CONNECT_FAILURE);
    m_start = HMTimeStamp::now();
    m_end = m_start + responseTime;
    return HM_WORK_COMPLETE_REMOTE;
}

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void
TESTNAME::setUp()
{
    setupCommon();
    HMDataHostGroupMap hostGroupMap;
    m_currentState = make_shared<HMState>();
    HMDNSCache dnsCache;
    m_currentState->m_datastore = make_unique<TestStorage>(&hostGroupMap, &dnsCache);
    m_state.setState(m_currentState);
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
TESTNAME::test_HMWorkRemoteCheck_basic()
{
    TestStorage* storage = dynamic_cast<TestStorage*>(m_currentState->m_datastore.get());

    multimap<HMDataCheckParams, HMDataCheckResult> results_h1;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h2_1;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h2_3;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h3;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h4;

    HMIPAddress address1_1;
    address1_1.set("1.2.3.4");

    HMIPAddress address1_2;
    address1_2.set("11.22.33.44");


    HMIPAddress address2;
    address2.set("4.5.6.7");

    HMIPAddress address3;
    address3.set("4.3.2.1");

    HMIPAddress address4;
    address4.set("7.6.5.4");

    string h1 = "host1";
    string h2 = "host2";
    string h3 = "host3";
    string h4 = "host4";

    HMDataCheckResult checkResult1_1_1;
    checkResult1_1_1.m_checkTime = HMTimeStamp::now();
    checkResult1_1_1.m_response = HM_RESPONSE_CONNECTED;
    checkResult1_1_1.m_reason = HM_REASON_SUCCESS;
    checkResult1_1_1.m_responseTime = 33;
    checkResult1_1_1.m_totalResponseTime = 66;
    checkResult1_1_1.m_smoothedResponseTime = 44;
    checkResult1_1_1.m_address = address1_1;


    HMDataCheckResult checkResult1_1_2;
    checkResult1_1_2.m_checkTime = HMTimeStamp::now();
    checkResult1_1_2.m_response = HM_RESPONSE_CONNECTED;
    checkResult1_1_2.m_reason = HM_REASON_SUCCESS;
    checkResult1_1_2.m_responseTime = 333;
    checkResult1_1_2.m_totalResponseTime = 666;
    checkResult1_1_2.m_smoothedResponseTime = 444;
    checkResult1_1_2.m_address = address1_2;


    HMDataCheckResult checkResult1_2_1;
    checkResult1_2_1.m_checkTime = HMTimeStamp::now();
    checkResult1_2_1.m_response = HM_RESPONSE_CONNECTED;
    checkResult1_2_1.m_reason = HM_REASON_SUCCESS;
    checkResult1_2_1.m_responseTime = 44;
    checkResult1_2_1.m_totalResponseTime = 77;
    checkResult1_2_1.m_smoothedResponseTime = 55;
    checkResult1_2_1.m_address = address1_1;


    HMDataCheckResult checkResult1_2_2;
    checkResult1_2_2.m_checkTime = HMTimeStamp::now();
    checkResult1_2_2.m_response = HM_RESPONSE_CONNECTED;
    checkResult1_2_2.m_reason = HM_REASON_SUCCESS;
    checkResult1_2_2.m_responseTime = 444;
    checkResult1_2_2.m_totalResponseTime = 777;
    checkResult1_2_2.m_smoothedResponseTime = 555;
    checkResult1_2_2.m_address = address1_2;

    HMDataCheckResult checkResult1_2;
    checkResult1_2.m_checkTime = HMTimeStamp::now();
    checkResult1_2.m_response = HM_RESPONSE_FAILED;
    checkResult1_2.m_reason = HM_REASON_RESPONSE_TIMEOUT;
    checkResult1_2.m_address = address2;

    HMDataCheckResult checkResult3_2;
    checkResult3_2.m_checkTime = HMTimeStamp::now();
    checkResult3_2.m_response = HM_RESPONSE_FAILED;
    checkResult3_2.m_reason = HM_REASON_RESPONSE_404;
    checkResult3_2.m_address = address2;

    HMDataCheckResult checkResult2_3;
    checkResult2_3.m_checkTime = HMTimeStamp::now();
    checkResult2_3.m_response = HM_RESPONSE_FAILED;
    checkResult2_3.m_reason = HM_REASON_RESPONSE_403;
    checkResult2_3.m_address = address3;

    HMDataCheckResult checkResult3_4;
    checkResult3_4.m_checkTime = HMTimeStamp::now();
    checkResult3_4.m_response = HM_RESPONSE_FAILED;
    checkResult3_4.m_reason = HM_REASON_RESPONSE_5XX;
    checkResult3_4.m_address = address4;

    string hg1 = "HostGroup1";
    string hg2 = "HostGroup2";
    string hg3 = "HostGroup3";

    HMDataHostCheck check1;
    HMDataHostCheck check2;
    HMDataHostCheck check3;
    HMDataCheckParams params1;
    HMDataCheckParams params2;
    HMDataCheckParams params3;
    string checkParams = "/check.html";
    HMDataHostGroup hostGroup1(hg1);
	hostGroup1.setCheckType(HM_CHECK_HTTP);
	hostGroup1.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup1.setPort(80);
	hostGroup1.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup1.setCheckInfo(checkParams);
	hostGroup1.setRemoteCheck("test");
	hostGroup1.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
	hostGroup1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    hostGroup1.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
	hostGroup1.setCheckTTL(300);
	hostGroup1.addHost(h1);
	hostGroup1.addHost(h2);
	hostGroup1.getHostCheck(check1);
	hostGroup1.getCheckParameters(params1);

    HMDataHostGroup hostGroup2(hg2);
    hostGroup2.setCheckType(HM_CHECK_HTTP);
    hostGroup2.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup2.setPort(80);
    hostGroup2.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup2.setCheckInfo(checkParams);
    hostGroup2.setRemoteCheck("test");
    hostGroup2.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
    hostGroup2.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    hostGroup2.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    hostGroup2.setCheckTTL(3000);
    hostGroup2.addHost(h1);
    hostGroup2.addHost(h3);
    hostGroup2.getHostCheck(check2);
    hostGroup2.getCheckParameters(params2);

    HMDataHostGroup hostGroup3(hg3);
    hostGroup3.setCheckType(HM_CHECK_HTTP);
    hostGroup3.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup3.setPort(8000);
    hostGroup3.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup3.setCheckInfo(checkParams);
    hostGroup3.setRemoteCheck("test");
    hostGroup3.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
    hostGroup3.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    hostGroup3.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    hostGroup3.setCheckTTL(30000);
    hostGroup3.addHost(h2);
    hostGroup3.addHost(h4);
    hostGroup3.getHostCheck(check3);
    hostGroup3.getCheckParameters(params3);

    results_h1.insert(make_pair(params1, checkResult1_1_1));
    results_h1.insert(make_pair(params1, checkResult1_1_2));
    results_h1.insert(make_pair(params2, checkResult1_2_1));
    results_h1.insert(make_pair(params2, checkResult1_2_2));

    results_h2_1.insert(make_pair(params1, checkResult1_2));
    results_h2_3.insert(make_pair(params3, checkResult3_2));

    results_h3.insert(make_pair(params2, checkResult2_3));
    results_h4.insert(make_pair(params3, checkResult3_4));



	m_currentState->m_hostGroups.insert(make_pair(hg1, hostGroup1));
	m_currentState->m_hostGroups.insert(make_pair(hg2, hostGroup2));
	m_currentState->m_hostGroups.insert(make_pair(hg3, hostGroup3));

	set<HMIPAddress> ips;
	ips.insert(address1_1);
    m_currentState->m_checkList.insertCheck(hg1, h1, check1, params1, ips);
    ips.clear();
    ips.insert(address1_2);
    m_currentState->m_checkList.insertCheck(hg2, h1, check2, params2, ips);
    ips.clear();
    ips.insert(address3);
    m_currentState->m_checkList.insertCheck(hg2, h3, check2, params2, ips);
    ips.clear();
    ips.insert(address2);
    m_currentState->m_checkList.insertCheck(hg1, h2, check1, params1, ips);
    ips.clear();
    ips.insert(address2);
    m_currentState->m_checkList.insertCheck(hg3, h2, check3, params3, ips);
    ips.clear();
    ips.insert(address4);
    m_currentState->m_checkList.insertCheck(hg3, h4, check3, params3, ips);


	HMDNSLookup lookup1(hostGroup1.getDNSType(), false, hostGroup1.getRemoteCheck());
    m_currentState->m_dnsCache.insertDNSEntry(h1, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h3, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h4, lookup1, 500, 60000);

    m_currentState->m_remoteHostCache.insertRemoteEntry(h1, check1, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h1, check2, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h2, check1, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h2, check3, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h3, check2, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h4, check3, 500, 60000);


    m_currentState->m_remoteHostCache.finishCheck(h1, check1, true);
    m_currentState->m_remoteHostCache.finishCheck(h1, check2, true);
    m_currentState->m_remoteHostCache.finishCheck(h2, check1, true);
    m_currentState->m_remoteHostCache.finishCheck(h2, check3, true);
    m_currentState->m_remoteHostCache.finishCheck(h3, check2, true);
    m_currentState->m_remoteHostCache.finishCheck(h3, check3, true);


    HMIPAddress addr(AF_INET);
    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h1, addr, check1, results_h1);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h2, addr, check1, results_h2_1);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    std::this_thread::sleep_for(2ms);
    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h2, addr, check3, results_h2_3);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h3, addr, check2, results_h3);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h4, addr, check3, results_h4);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    // query the results directly

    set<HMIPAddress> addresses;
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h1, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h2, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h3, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h4, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));

    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check1;
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address1_1, getResults));
        CPPUNIT_ASSERT(getResults.size() == 2);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address1_1);
        CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 33);
        CPPUNIT_ASSERT(getResults[0].second.m_totalResponseTime == 66);
        CPPUNIT_ASSERT(getResults[0].second.m_smoothedResponseTime == 44);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

        CPPUNIT_ASSERT(getResults[1].first == params2);
        CPPUNIT_ASSERT(getResults[1].second.m_address == address1_1);
        CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 44);
        CPPUNIT_ASSERT(getResults[1].second.m_totalResponseTime == 77);
        CPPUNIT_ASSERT(getResults[1].second.m_smoothedResponseTime == 55);
        CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_SUCCESS);

        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address1_2, getResults));
        CPPUNIT_ASSERT(getResults.size() == 2);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address1_2);
        CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 333);
        CPPUNIT_ASSERT(getResults[0].second.m_totalResponseTime == 666);
        CPPUNIT_ASSERT(getResults[0].second.m_smoothedResponseTime == 444);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

        CPPUNIT_ASSERT(getResults[1].first == params2);
        CPPUNIT_ASSERT(getResults[1].second.m_address == address1_2);
        CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 444);
        CPPUNIT_ASSERT(getResults[1].second.m_totalResponseTime == 777);
        CPPUNIT_ASSERT(getResults[1].second.m_smoothedResponseTime == 555);
        CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_SUCCESS);


        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h2, c, address2, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address2);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_TIMEOUT);
    }

    // query the results directly
    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check2;

        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address1_1, getResults));
        CPPUNIT_ASSERT(getResults.size() == 2);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address1_1);
        CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 33);
        CPPUNIT_ASSERT(getResults[0].second.m_totalResponseTime == 66);
        CPPUNIT_ASSERT(getResults[0].second.m_smoothedResponseTime == 44);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

        CPPUNIT_ASSERT(getResults[1].first == params2);
        CPPUNIT_ASSERT(getResults[1].second.m_address == address1_1);
        CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 44);
        CPPUNIT_ASSERT(getResults[1].second.m_totalResponseTime == 77);
        CPPUNIT_ASSERT(getResults[1].second.m_smoothedResponseTime == 55);
        CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_SUCCESS);

        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address1_2, getResults));
        CPPUNIT_ASSERT(getResults.size() == 2);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address1_2);
        CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 333);
        CPPUNIT_ASSERT(getResults[0].second.m_totalResponseTime == 666);
        CPPUNIT_ASSERT(getResults[0].second.m_smoothedResponseTime == 444);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

        CPPUNIT_ASSERT(getResults[1].first == params2);
        CPPUNIT_ASSERT(getResults[1].second.m_address == address1_2);
        CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 444);
        CPPUNIT_ASSERT(getResults[1].second.m_totalResponseTime == 777);
        CPPUNIT_ASSERT(getResults[1].second.m_smoothedResponseTime == 555);
        CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_SUCCESS);

        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h3, c, address3, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params2);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address3);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_403);
    }

    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check3;
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h2, c, address2, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params3);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address2);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_404);
        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h4, c, address4, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params3);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address4);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_5XX);
    }


    std::this_thread::sleep_for(505ms);

    CPPUNIT_ASSERT_EQUAL(5, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_hostCheck == check1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_hostCheck == check1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    CPPUNIT_ASSERT(work->m_hostCheck == check3);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h3);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    CPPUNIT_ASSERT(work->m_hostCheck == check2);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h4);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    CPPUNIT_ASSERT(work->m_hostCheck == check3);
}

void
TESTNAME::test_HMWorkRemoteCheck_mixed_dns()
{

    TestStorage* storage = dynamic_cast<TestStorage*>(m_currentState->m_datastore.get());

    multimap<HMDataCheckParams, HMDataCheckResult> results_h1;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h2;

    HMIPAddress address1;
    address1.set("1.2.3.4");

    HMIPAddress address2;
    address2.set("4.5.6.7");

    HMIPAddress address3;
    address3.set("4.3.2.1");

    HMIPAddress address4;
    address4.set("7.6.5.4");

    string h1 = "host1";
    string h2 = "host2";

    HMDataCheckResult checkResult1;
    checkResult1.m_checkTime = HMTimeStamp::now();
    checkResult1.m_response = HM_RESPONSE_CONNECTED;
    checkResult1.m_reason = HM_REASON_SUCCESS;
    checkResult1.m_responseTime = 33;
    checkResult1.m_totalResponseTime = 66;
    checkResult1.m_smoothedResponseTime = 44;
    checkResult1.m_address = address1;

    HMDataCheckResult checkResult2;
    checkResult2.m_checkTime = HMTimeStamp::now();
    checkResult2.m_response = HM_RESPONSE_FAILED;
    checkResult2.m_reason = HM_REASON_RESPONSE_TIMEOUT;
    checkResult2.m_address = address2;
    string hg1 = "HostGroup1";
    string hg2 = "HostGroup2";

    HMDataHostCheck defaultcheck;
    HMDataHostCheck check1;
    HMDataHostCheck check2;
    HMDataCheckParams params1;
    HMDataCheckParams params2;
    string checkParams = "/check.html";
    HMDataHostGroup hostGroup1(hg1);
    hostGroup1.setCheckType(HM_CHECK_HTTP);
    hostGroup1.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup1.setPort(80);
    hostGroup1.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup1.setCheckInfo(checkParams);
    hostGroup1.setRemoteCheck("test");
    hostGroup1.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
    hostGroup1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);

    hostGroup1.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    hostGroup1.addHost(h1);
    hostGroup1.addHost(h2);
    hostGroup1.getHostCheck(check1);
    hostGroup1.getCheckParameters(params1);

    HMDataHostGroup hostGroup2(hg2);
    hostGroup2.setCheckType(HM_CHECK_HTTP);
    hostGroup2.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup2.setPort(80);
    hostGroup2.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup2.setCheckInfo(checkParams);
    hostGroup2.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
    hostGroup2.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    hostGroup1.addHost(h1);
    hostGroup1.addHost(h2);
    check2.setCheckParams(hostGroup2);
    hostGroup2.getCheckParameters(params2);


    results_h1.insert(make_pair(params1, checkResult1));
    results_h2.insert(make_pair(params1, checkResult2));


    m_currentState->m_hostGroups.insert(make_pair(hg1, hostGroup1));
    m_currentState->m_hostGroups.insert(make_pair(hg2, hostGroup2));


    m_currentState->m_dnsWaitList.insert(make_pair(HMDNSTypeMap(h1, HM_DNS_TYPE_STATIC, ""),check2));
    m_currentState->m_dnsWaitList.insert(make_pair(HMDNSTypeMap(h2, HM_DNS_TYPE_STATIC, ""),check2));

    set<HMIPAddress> ips;
    ips.insert(address1);
    m_currentState->m_checkList.insertCheck(hg1, h1, check1, params1, ips);
    ips.clear();
    ips.insert(address2);
    m_currentState->m_checkList.insertCheck(hg1, h2, check1, params1, ips);
    ips.clear();
    ips.insert(address3);
    m_currentState->m_checkList.insertCheck(hg2, h1, check2, params2, ips);
    ips.clear();
    ips.insert(address4);
    m_currentState->m_checkList.insertCheck(hg2, h2, check2, params2, ips);

    HMDNSLookup lookup1(hostGroup1.getDNSType(), false, hostGroup1.getRemoteCheck());
    lookup1.setPlugin(HM_DNS_PLUGIN_ARES);
    m_currentState->m_dnsCache.insertDNSEntry(h1, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, lookup1, 500, 60000);

    HMDNSLookup dnsHostCheckF(HM_DNS_TYPE_STATIC, false, hostGroup2.getRemoteCheck());
    dnsHostCheckF.setPlugin(HM_DNS_PLUGIN_STATIC);
    m_currentState->m_dnsCache.insertDNSEntry(h1, dnsHostCheckF, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, dnsHostCheckF, 500, 60000);

    m_currentState->m_remoteHostCache.insertRemoteEntry(h1, check1, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h2, check1, 500, 60000);
    m_currentState->m_remoteHostCache.finishCheck(h1, check1, true);
    m_currentState->m_remoteHostCache.finishCheck(h2, check1, true);

    HMIPAddress addr(AF_INET);
    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h1, addr, check1, results_h1);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    remoteCheck = new TestHMWorkRemoteHostCheck(h2, addr, check1, results_h2);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;
    std::this_thread::sleep_for(2ms);
    dnsLookup = new TestHMWorkDNSLookup(h1, addr, check2, dnsHostCheckF);
    dnsLookup->updateState(&m_state, m_eventQueue);
    dnsLookup->addTarget(address3);
    set<HMIPAddress> vip_ret;
    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    CPPUNIT_ASSERT(m_currentState->m_dnsCache.getAddresses(h1, HM_DUALSTACK_IPV4_ONLY, dnsHostCheckF, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address3) != vip_ret.end());
    delete dnsLookup;

    // Test to make sure we do not return if the query is timed out
    dnsLookup = new TestHMWorkDNSLookup(h2, addr, check2, dnsHostCheckF);
    dnsLookup->updateState(&m_state, m_eventQueue);
    dnsLookup->addTarget(address4);
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsLookup->processWork() == HM_WORK_COMPLETE);
    CPPUNIT_ASSERT(m_currentState->m_dnsCache.getAddresses(h2, HM_DUALSTACK_IPV4_ONLY, dnsHostCheckF, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address4) != vip_ret.end());
    delete dnsLookup;

    // query the results directly
    set<HMIPAddress> addresses;
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h1, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h2, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));

    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check1;
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address1, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address1);
        CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 33);
        CPPUNIT_ASSERT(getResults[0].second.m_totalResponseTime == 66);
        CPPUNIT_ASSERT(getResults[0].second.m_smoothedResponseTime == 44);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h2, c, address2, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address2);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_TIMEOUT);
    }

    std::this_thread::sleep_for(505ms);

    CPPUNIT_ASSERT_EQUAL(6, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_hostCheck == check2);
    CPPUNIT_ASSERT(work->m_ipAddress == address3);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_hostCheck == check2);
    CPPUNIT_ASSERT(work->m_ipAddress == address4);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_hostCheck == check1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_hostCheck == check1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_hostCheck == defaultcheck);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_hostCheck == defaultcheck);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
}

void
TESTNAME::test_HMWorkRemoteCheck_mixed_healthcheck()
{

    TestStorage* storage = dynamic_cast<TestStorage*>(m_currentState->m_datastore.get());

    multimap<HMDataCheckParams, HMDataCheckResult> results_h1;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h2;

    HMIPAddress address1;
    address1.set("1.2.3.4");

    HMIPAddress address2;
    address2.set("4.5.6.7");

    HMIPAddress address3;
    address3.set("4.3.2.1");

    HMIPAddress address4;
    address4.set("7.6.5.4");

    string h1 = "host1";
    string h2 = "host2";

    HMDataCheckResult checkResult1;
    checkResult1.m_checkTime = HMTimeStamp::now();
    checkResult1.m_response = HM_RESPONSE_CONNECTED;
    checkResult1.m_reason = HM_REASON_SUCCESS;
    checkResult1.m_responseTime = 33;
    checkResult1.m_totalResponseTime = 66;
    checkResult1.m_smoothedResponseTime = 44;
    checkResult1.m_address = address1;

    HMDataCheckResult checkResult2;
    checkResult2.m_checkTime = HMTimeStamp::now();
    checkResult2.m_response = HM_RESPONSE_FAILED;
    checkResult2.m_reason = HM_REASON_RESPONSE_TIMEOUT;
    checkResult2.m_address = address2;

    string hg1 = "HostGroup1";
    string hg2 = "HostGroup2";

    HMDataHostCheck defaultcheck;
    HMDataHostCheck check1;
    HMDataHostCheck check2;
    HMDataCheckParams params1;
    HMDataCheckParams params2;
    string checkParams = "/check.html";
    HMDataHostGroup hostGroup1(hg1);
    hostGroup1.setCheckType(HM_CHECK_HTTP);
    hostGroup1.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup1.setPort(80);
    hostGroup1.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup1.setCheckInfo(checkParams);
    hostGroup1.setRemoteCheck("test");
    hostGroup1.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
    hostGroup1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);

    hostGroup1.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    hostGroup1.addHost(h1);
    hostGroup1.addHost(h2);
    hostGroup1.getHostCheck(check1);
    hostGroup1.getCheckParameters(params1);

    HMDataHostGroup hostGroup2(hg2);
    hostGroup2.setCheckType(HM_CHECK_HTTP);
    hostGroup2.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup2.setPort(80);
    hostGroup2.setCheckTTL(500);
    hostGroup2.setCheckTimeout(60000);
    hostGroup2.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup2.setCheckInfo(checkParams);
    hostGroup2.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
    hostGroup2.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    hostGroup2.addHost(h1);
    hostGroup2.addHost(h2);
    check2.setCheckParams(hostGroup2);
    hostGroup2.getCheckParameters(params2);

    results_h1.insert(make_pair(params1, checkResult1));
    results_h2.insert(make_pair(params1, checkResult2));

    m_currentState->m_hostGroups.insert(make_pair(hg1, hostGroup1));
    m_currentState->m_hostGroups.insert(make_pair(hg2, hostGroup2));


    set<HMIPAddress> ips;
    ips.insert(address1);
    m_currentState->m_checkList.insertCheck(hg1, h1, check1, params1, ips);
    ips.clear();
    ips.insert(address2);
    m_currentState->m_checkList.insertCheck(hg1, h2, check1, params1, ips);
    ips.clear();
    ips.insert(address3);
    m_currentState->m_checkList.insertCheck(hg2, h1, check2, params2, ips);
    ips.clear();
    ips.insert(address4);
    m_currentState->m_checkList.insertCheck(hg2, h2, check2, params2, ips);

    HMDNSLookup lookup1(hostGroup1.getDNSType(), false, hostGroup1.getRemoteCheck());
    m_currentState->m_dnsCache.insertDNSEntry(h1, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, lookup1, 500, 60000);

    HMDNSLookup dnsHostCheckF(hostGroup2.getDNSType(), false, hostGroup2.getRemoteCheck());
    m_currentState->m_dnsCache.insertDNSEntry(h1, dnsHostCheckF, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, dnsHostCheckF, 500, 60000);
    ips.clear();
    ips.insert(address3);
    m_currentState->m_dnsCache.updateDNSEntry(h1, dnsHostCheckF, ips);
    ips.clear();
    ips.insert(address4);
    m_currentState->m_dnsCache.updateDNSEntry(h2, dnsHostCheckF, ips);
    m_currentState->m_dnsCache.finishQuery(h1, dnsHostCheckF, true);
    m_currentState->m_dnsCache.finishQuery(h2, dnsHostCheckF, true);

    m_currentState->m_remoteHostCache.insertRemoteEntry(h1, check1, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h2, check1, 500, 60000);

    m_currentState->m_remoteHostCache.finishCheck(h1, check1, true);
    m_currentState->m_remoteHostCache.finishCheck(h2, check1, true);

    HMIPAddress addr(AF_INET);
    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h1, addr, check1, results_h1);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    remoteCheck = new TestHMWorkRemoteHostCheck(h2, addr, check1, results_h2);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    std::this_thread::sleep_for(2ms);

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(h1, address3, check2);
    healthCheck->responseTime = 10;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = address3;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);
    delete healthCheck;

    std::this_thread::sleep_for(2ms);
    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(h2, address4, check2);
    healthCheck->responseTime = 10;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = address4;
    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);
    delete healthCheck;

    std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
    // query the results directly
    m_currentState->m_checkList.getCheckResults(h1, check2, address3, getResults);
    CPPUNIT_ASSERT(getResults.size() == 1);

    CPPUNIT_ASSERT(getResults[0].first == params2);
    CPPUNIT_ASSERT(getResults[0].second.m_address == address3);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(h2, check2, address4, getResults);
    CPPUNIT_ASSERT(getResults.size() == 1);

    CPPUNIT_ASSERT(getResults[0].first == params2);
    CPPUNIT_ASSERT(getResults[0].second.m_address == address4);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);


    // query the results directly
    set<HMIPAddress> addresses;
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h1, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h2, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));

    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check1;
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address1, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address1);
        CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 33);
        CPPUNIT_ASSERT(getResults[0].second.m_totalResponseTime == 66);
        CPPUNIT_ASSERT(getResults[0].second.m_smoothedResponseTime == 44);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h2, c, address2, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address2);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_TIMEOUT);
    }

    std::this_thread::sleep_for(505ms);

    CPPUNIT_ASSERT_EQUAL(4, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_hostCheck == check1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_hostCheck == check1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_hostCheck == check2);
    CPPUNIT_ASSERT(work->m_ipAddress == address3);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_hostCheck == check2);
    CPPUNIT_ASSERT(work->m_ipAddress == address4);
}


void
TESTNAME::test_HMWorkRemoteCheck_mixed_remote_host_remote_hostgroup()
{
    TestStorage* storage = dynamic_cast<TestStorage*>(m_currentState->m_datastore.get());

    multimap<HMDataCheckParams, HMDataCheckResult> results_h1_1;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h1_2;
    vector<HMGroupCheckResult> results_hg2;

    HMIPAddress address1;
    address1.set("1.2.3.4");

    HMIPAddress address2;
    address2.set("4.5.6.7");

    HMIPAddress address3;
    address3.set("4.3.2.1");

    HMIPAddress address4;
    address4.set("7.6.5.4");

    string h1 = "host1";
    string h2 = "host2";

    HMDataCheckResult checkResult1;
    checkResult1.m_checkTime = HMTimeStamp::now();
    checkResult1.m_response = HM_RESPONSE_CONNECTED;
    checkResult1.m_reason = HM_REASON_SUCCESS;
    checkResult1.m_responseTime = 33;
    checkResult1.m_totalResponseTime = 66;
    checkResult1.m_smoothedResponseTime = 44;
    checkResult1.m_address = address1;

    HMDataCheckResult checkResult2;
    checkResult2.m_checkTime = HMTimeStamp::now();
    checkResult2.m_response = HM_RESPONSE_FAILED;
    checkResult2.m_reason = HM_REASON_RESPONSE_TIMEOUT;
    checkResult2.m_address = address2;

    HMDataCheckResult checkResult3;
    checkResult3.m_checkTime = HMTimeStamp::now();
    checkResult3.m_response = HM_RESPONSE_FAILED;
    checkResult3.m_reason = HM_REASON_RESPONSE_403;
    checkResult3.m_address = address3;

    HMDataCheckResult checkResult4;
    checkResult4.m_checkTime = HMTimeStamp::now();
    checkResult4.m_response = HM_RESPONSE_FAILED;
    checkResult4.m_reason = HM_REASON_RESPONSE_5XX;
    checkResult4.m_address = address4;

    HMGroupCheckResult result3;
    result3.m_address = address3;
    result3.m_hostName = h1;
    result3.m_result = checkResult3;

    HMGroupCheckResult result4;
    result4.m_address = address4;
    result4.m_hostName = h2;
    result4.m_result = checkResult4;

    string hg1 = "HostGroup1";
    string hg2 = "HostGroup2";

    HMDataHostCheck check1;
    HMDataHostCheck check2;
    HMDataCheckParams params1;
    HMDataCheckParams params2;
    string checkParams = "/check.html";
    HMDataHostGroup hostGroup1(hg1);
    hostGroup1.setCheckType(HM_CHECK_HTTP);
    hostGroup1.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup1.setPort(80);
    hostGroup1.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup1.setCheckInfo(checkParams);
    hostGroup1.setRemoteCheck("test1");
    hostGroup1.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
    hostGroup1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);

    hostGroup1.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    hostGroup1.addHost(h1);
    hostGroup1.addHost(h2);
    hostGroup1.getHostCheck(check1);
    hostGroup1.getCheckParameters(params1);

    HMDataHostGroup hostGroup2(hg2);
    hostGroup2.setCheckType(HM_CHECK_HTTP);
    hostGroup2.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup2.setPort(80);
    hostGroup2.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup2.setCheckInfo(checkParams);
    hostGroup2.setRemoteCheck("test2");
    hostGroup2.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
    hostGroup2.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    hostGroup2.setFlowType(HM_FLOW_REMOTE_HOSTGROUP_TYPE);
    hostGroup2.addHost(h1);
    hostGroup2.addHost(h2);
    hostGroup2.getHostCheck(check2);
    hostGroup2.getCheckParameters(params2);

    results_hg2.push_back(result3);
    results_hg2.push_back(result4);

    results_h1_1.insert(make_pair(params1, checkResult1));
    results_h1_2.insert(make_pair(params1, checkResult2));

    m_currentState->m_hostGroups.insert(make_pair(hg1, hostGroup1));
    m_currentState->m_hostGroups.insert(make_pair(hg2, hostGroup2));

    set<HMIPAddress> ips;
    ips.insert(address1);
    m_currentState->m_checkList.insertCheck(hg1, h1, check1, params1, ips);
    ips.clear();
    ips.insert(address2);
    m_currentState->m_checkList.insertCheck(hg1, h2, check1, params1, ips);
    ips.clear();
    ips.insert(address3);
    m_currentState->m_checkList.insertCheck(hg2, h1, check2, params2, ips);
    ips.clear();
    ips.insert(address4);
    m_currentState->m_checkList.insertCheck(hg2, h2, check2, params2, ips);


    HMDNSLookup lookup1(hostGroup1.getDNSType(), false, hostGroup1.getRemoteCheck());
    HMDNSLookup lookup2(hostGroup2.getDNSType(), false, hostGroup2.getRemoteCheck());
    m_currentState->m_dnsCache.insertDNSEntry(h1, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h1, lookup2, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, lookup2, 500, 60000);

    m_currentState->m_remoteHostCache.insertRemoteEntry(h1, check1, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h2, check1, 500, 60000);
    m_currentState->m_remoteCache.insertRemoteEntry(hg2, 500, 60000);

    m_currentState->m_remoteHostCache.finishCheck(h1, check1, true);
    m_currentState->m_remoteHostCache.finishCheck(h2, check1, true);
    m_currentState->m_remoteCache.finishCheck(hg2, true);

    HMIPAddress addr(AF_INET);
    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h1, addr, check1, results_h1_1);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h2, addr, check1, results_h1_2);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    std::this_thread::sleep_for(2ms);

    storage->m_writeCount = 0;
    remoteGroupCheck = new TestHMWorkRemoteCheck(hg2, addr, check2, hostGroup2, results_hg2);
    remoteGroupCheck->responseTime = 10;
    remoteGroupCheck->updateState(&m_state, m_eventQueue);
    remoteGroupCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteGroupCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteGroupCheck;

    // query the results directly
    set<HMIPAddress> addresses;
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h1, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h2, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));

    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check1;
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address1, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address1);
        CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 33);
        CPPUNIT_ASSERT(getResults[0].second.m_totalResponseTime == 66);
        CPPUNIT_ASSERT(getResults[0].second.m_smoothedResponseTime == 44);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h2, c, address2, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address2);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_TIMEOUT);
    }




    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h1, HM_DUALSTACK_IPV4_ONLY, lookup2, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h2, HM_DUALSTACK_IPV4_ONLY, lookup2, addresses));

    // query the results directly
    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check2;
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address3, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params2);
        CPPUNIT_ASSERT(getResults[0].second == HMDataCheckResult(params2.getTimeout()));
        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h2, c, address4, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params2);
        CPPUNIT_ASSERT(getResults[0].second == HMDataCheckResult(params2.getTimeout()));

        vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(m_currentState->m_datastore->getGroupCheckResults(hg2, results));
        CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_address == address3);
        CPPUNIT_ASSERT(results[0].m_hostName == h1);
        CPPUNIT_ASSERT(results[0].m_result.m_reason == checkResult3.m_reason);
        CPPUNIT_ASSERT(results[1].m_address == address4);
        CPPUNIT_ASSERT(results[1].m_hostName == h2);
        CPPUNIT_ASSERT(results[1].m_result.m_reason == checkResult4.m_reason);
    }


    std::this_thread::sleep_for(505ms);

    CPPUNIT_ASSERT_EQUAL(3, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check2));
    CPPUNIT_ASSERT(work->m_hostname == hg2);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
}

void
TESTNAME::test_HMWorkRemoteCheck_mixed_remote()
{
    TestStorage* storage = dynamic_cast<TestStorage*>(m_currentState->m_datastore.get());

    multimap<HMDataCheckParams, HMDataCheckResult> results_h1_1;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h2_1;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h1_2;
    multimap<HMDataCheckParams, HMDataCheckResult> results_h2_2;

    HMIPAddress address1;
    address1.set("1.2.3.4");

    HMIPAddress address2;
    address2.set("4.5.6.7");

    HMIPAddress address3;
    address3.set("4.3.2.1");

    HMIPAddress address4;
    address4.set("7.6.5.4");

    string h1 = "host1";
    string h2 = "host2";

    HMDataCheckResult checkResult1;
    checkResult1.m_checkTime = HMTimeStamp::now();
    checkResult1.m_response = HM_RESPONSE_CONNECTED;
    checkResult1.m_reason = HM_REASON_SUCCESS;
    checkResult1.m_responseTime = 33;
    checkResult1.m_totalResponseTime = 66;
    checkResult1.m_smoothedResponseTime = 44;
    checkResult1.m_address = address1;

    HMDataCheckResult checkResult2;
    checkResult2.m_checkTime = HMTimeStamp::now();
    checkResult2.m_response = HM_RESPONSE_FAILED;
    checkResult2.m_reason = HM_REASON_RESPONSE_TIMEOUT;
    checkResult2.m_address = address2;

    HMDataCheckResult checkResult3;
    checkResult3.m_checkTime = HMTimeStamp::now();
    checkResult3.m_response = HM_RESPONSE_FAILED;
    checkResult3.m_reason = HM_REASON_RESPONSE_403;
    checkResult3.m_address = address3;

    HMDataCheckResult checkResult4;
    checkResult4.m_checkTime = HMTimeStamp::now();
    checkResult4.m_response = HM_RESPONSE_FAILED;
    checkResult4.m_reason = HM_REASON_RESPONSE_5XX;
    checkResult4.m_address = address4;

    string hg1 = "HostGroup1";
    string hg2 = "HostGroup2";

    HMDataHostCheck check1;
    HMDataHostCheck check2;
    HMDataCheckParams params1;
    HMDataCheckParams params2;
    string checkParams = "/check.html";
    HMDataHostGroup hostGroup1(hg1);
    hostGroup1.setCheckType(HM_CHECK_HTTP);
    hostGroup1.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup1.setPort(80);
    hostGroup1.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup1.setCheckInfo(checkParams);
    hostGroup1.setRemoteCheck("test1");
    hostGroup1.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
    hostGroup1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);

    hostGroup1.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    hostGroup1.addHost(h1);
    hostGroup1.addHost(h2);
    hostGroup1.getHostCheck(check1);
    hostGroup1.getCheckParameters(params1);

    HMDataHostGroup hostGroup2(hg2);
    hostGroup2.setCheckType(HM_CHECK_HTTP);
    hostGroup2.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup2.setPort(80);
    hostGroup2.setDualStack(HM_DUALSTACK_IPV4_ONLY);
    hostGroup2.setCheckInfo(checkParams);
    hostGroup2.setRemoteCheck("test2");
    hostGroup2.setRemoteCheckType(HM_REMOTE_SHARED_CHECK_TCP);
    hostGroup2.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);

    hostGroup2.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    hostGroup2.addHost(h1);
    hostGroup2.addHost(h2);
    hostGroup2.getHostCheck(check2);
    hostGroup2.getCheckParameters(params2);

    results_h1_1.insert(make_pair(params1, checkResult1));
    results_h1_2.insert(make_pair(params1, checkResult2));
    results_h2_1.insert(make_pair(params2, checkResult3));
    results_h2_2.insert(make_pair(params2, checkResult4));

    m_currentState->m_hostGroups.insert(make_pair(hg1, hostGroup1));
    m_currentState->m_hostGroups.insert(make_pair(hg2, hostGroup2));

    set<HMIPAddress> ips;
    ips.insert(address1);
    m_currentState->m_checkList.insertCheck(hg1, h1, check1, params1, ips);
    ips.clear();
    ips.insert(address2);
    m_currentState->m_checkList.insertCheck(hg1, h2, check1, params1, ips);
    ips.clear();
    ips.insert(address3);
    m_currentState->m_checkList.insertCheck(hg2, h1, check2, params2, ips);
    ips.clear();
    ips.insert(address4);
    m_currentState->m_checkList.insertCheck(hg2, h2, check2, params2, ips);


    HMDNSLookup lookup1(hostGroup1.getDNSType(), false, hostGroup1.getRemoteCheck());
    HMDNSLookup lookup2(hostGroup2.getDNSType(), false, hostGroup2.getRemoteCheck());
    m_currentState->m_dnsCache.insertDNSEntry(h1, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, lookup1, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h1, lookup2, 500, 60000);
    m_currentState->m_dnsCache.insertDNSEntry(h2, lookup2, 500, 60000);

    m_currentState->m_remoteHostCache.insertRemoteEntry(h1, check1, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h2, check1, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h1, check2, 500, 60000);
    m_currentState->m_remoteHostCache.insertRemoteEntry(h2, check2, 500, 60000);


    m_currentState->m_remoteHostCache.finishCheck(h1, check1, true);
    m_currentState->m_remoteHostCache.finishCheck(h2, check1, true);
    m_currentState->m_remoteHostCache.finishCheck(h1, check2, true);
    m_currentState->m_remoteHostCache.finishCheck(h2, check2, true);

    HMIPAddress addr(AF_INET);
    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h1, addr, check1, results_h1_1);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h2, addr, check1, results_h1_2);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    std::this_thread::sleep_for(2ms);

    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h1, addr, check2, results_h2_1);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    std::this_thread::sleep_for(2ms);

    storage->m_writeCount = 0;
    remoteCheck = new TestHMWorkRemoteHostCheck(h2, addr, check2, results_h2_2);
    remoteCheck->responseTime = 10;
    remoteCheck->updateState(&m_state, m_eventQueue);
    remoteCheck->m_ipAddress = addr;

    CPPUNIT_ASSERT(remoteCheck->processWork() == HM_WORK_COMPLETE_REMOTE);
    delete remoteCheck;

    // query the results directly
    set<HMIPAddress> addresses;
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h1, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h2, HM_DUALSTACK_IPV4_ONLY, lookup1, addresses));

    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check1;
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address1, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address1);
        CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 33);
        CPPUNIT_ASSERT(getResults[0].second.m_totalResponseTime == 66);
        CPPUNIT_ASSERT(getResults[0].second.m_smoothedResponseTime == 44);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h2, c, address2, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params1);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address2);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_TIMEOUT);
    }




    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h1, HM_DUALSTACK_IPV4_ONLY, lookup2, addresses));
    CPPUNIT_ASSERT(!m_currentState->m_dnsCache.getAddresses(h2, HM_DUALSTACK_IPV4_ONLY, lookup2, addresses));

    // query the results directly
    {
        std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;
        HMDataHostCheck c = check2;
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h1, c, address3, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params2);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address3);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_403);
        getResults.clear();
        CPPUNIT_ASSERT(m_currentState->m_checkList.getCheckResults(h2, c, address4, getResults));
        CPPUNIT_ASSERT(getResults.size() == 1);
        CPPUNIT_ASSERT(getResults[0].first == params2);
        CPPUNIT_ASSERT(getResults[0].second.m_address == address4);
        CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
        CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_RESPONSE_5XX);
    }




    std::this_thread::sleep_for(505ms);

    CPPUNIT_ASSERT_EQUAL(4, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check2));
    CPPUNIT_ASSERT(work->m_hostname == h1);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check2));
    CPPUNIT_ASSERT(work->m_hostname == h2);
    CPPUNIT_ASSERT(work->m_ipAddress == addr);
}
