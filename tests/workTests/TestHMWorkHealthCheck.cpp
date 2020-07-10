// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <thread>

#include "HMEventLoopQueue.h"
#include "TestHMWorkHealthCheck.h"
#include "common.h"
#include "TestStorage.h"

using namespace std;

HM_WORK_STATUS
TestHMWorkHealthCheck::healthCheck()
{
    if(responseTime < 0)
    {
        return HM_WORK_COMPLETE;
    }
    m_response = (responseTime?HM_RESPONSE_CONNECTED:HM_RESPONSE_FAILED);
    m_reason = (responseTime?HM_REASON_SUCCESS:HM_REASON_CONNECT_FAILURE);
    m_start = HMTimeStamp::now();
    m_end = m_start + responseTime;
    return HM_WORK_COMPLETE;
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
TESTNAME::test_HMWorkHealthCheck_IPV4()
{
    TestStorage* storage = dynamic_cast<TestStorage*>(m_currentState->m_datastore.get());

    string checkParams = "/check.html";
    HMDataHostCheck hostCheck;
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
	hostCheck.setCheckParams(hostGroup);
    HMDNSLookup dnsHostCheckF(HM_DNS_TYPE_LOOKUP, false);
    HMDataCheckParams paramsNormal;
    HMDataCheckParams paramsTimedout;

    paramsNormal.setCheckParameters(3,
            100,
            0,
            1000,
            5,
            10,
            3,
            10000,
            100, HM_DEFAULT_FLAP_THRESHOLD, 0);

    paramsTimedout.setCheckParameters(3,
            0,
            0,
            500,
            2,
            10,
            3,
            10000,
            0, HM_DEFAULT_FLAP_THRESHOLD, 0);

    string basic = "ipv4.basic.hm.com";
    string success = "ipv4.success.hm.com";
    string failure = "ipv4.failure.hm.com";
    string error = "ipv4.error.hm.com";

    HMIPAddress addr1;
    HMIPAddress addr2;
    addr1.set("192.168.0.1");
    addr2.set("192.168.0.2");
    set<HMIPAddress> ips;
    set<HMIPAddress> ips1;
    ips1.insert(addr1);
    ips.insert(addr1);
    ips.insert(addr2);

    m_currentState->m_checkList.insertCheck("HostGroup1", basic, hostCheck, paramsNormal, ips1);

    m_currentState->m_checkList.insertCheck("HostGroup1", success, hostCheck, paramsNormal, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", success, hostCheck, paramsTimedout, ips);

    m_currentState->m_checkList.insertCheck("HostGroup1", failure, hostCheck, paramsNormal, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", failure, hostCheck, paramsTimedout, ips);

    m_currentState->m_checkList.insertCheck("HostGroup1", error, hostCheck, paramsNormal, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", error, hostCheck, paramsTimedout, ips);
    m_currentState->m_dnsCache.insertDNSEntry(basic, dnsHostCheckF, 10000, 10000);
    m_currentState->m_dnsCache.insertDNSEntry(success, dnsHostCheckF, 10000, 10000);
    m_currentState->m_dnsCache.insertDNSEntry(failure, dnsHostCheckF, 10000, 10000);
    m_currentState->m_dnsCache.insertDNSEntry(error, dnsHostCheckF, 10000, 10000);
    m_currentState->m_dnsCache.updateDNSEntry(basic, dnsHostCheckF, ips1);
    m_currentState->m_dnsCache.updateDNSEntry(success, dnsHostCheckF, ips);
    m_currentState->m_dnsCache.updateDNSEntry(failure, dnsHostCheckF, ips);
    m_currentState->m_dnsCache.updateDNSEntry(error, dnsHostCheckF, ips);
    m_currentState->m_dnsCache.finishQuery(basic, dnsHostCheckF, true);
    m_currentState->m_dnsCache.finishQuery(success, dnsHostCheckF, true);
    m_currentState->m_dnsCache.finishQuery(failure, dnsHostCheckF, true);
    m_currentState->m_dnsCache.finishQuery(error, dnsHostCheckF, true);
    std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(basic, addr1, hostCheck);
    healthCheck->responseTime = 10;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr1;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(basic, hostCheck, addr1, getResults);
    CPPUNIT_ASSERT(getResults.size() == 1);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 1);
    CPPUNIT_ASSERT(storage->m_hostname == basic);
    CPPUNIT_ASSERT(storage->m_address == addr1);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsNormal);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 10);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_SUCCESS);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(success, addr1, hostCheck);
    healthCheck->responseTime = 10;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr1;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(success, hostCheck, addr1, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_SUCCESS);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == success);
    CPPUNIT_ASSERT(storage->m_address == addr1);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 10);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_SUCCESS);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(success, addr2, hostCheck);
    healthCheck->responseTime = 10;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr2;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(success, hostCheck, addr2, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_SUCCESS);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == success);
    CPPUNIT_ASSERT(storage->m_address == addr2);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 10);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_SUCCESS);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(failure, addr1, hostCheck);
    healthCheck->responseTime = 0;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr1;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(failure, hostCheck, addr1, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_CONNECT_FAILURE);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_CONNECT_FAILURE);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == failure);
    CPPUNIT_ASSERT(storage->m_address == addr1);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10000);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 0);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_CONNECT_FAILURE);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(failure, addr2, hostCheck);
    healthCheck->responseTime = 0;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr2;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(failure, hostCheck, addr2, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_CONNECT_FAILURE);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_CONNECT_FAILURE);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == failure);
    CPPUNIT_ASSERT(storage->m_address == addr2);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10000);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 0);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_CONNECT_FAILURE);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(error, addr1, hostCheck);
    healthCheck->responseTime = -1;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr1;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(error, hostCheck, addr1, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_NONE);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_NONE);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == error);
    CPPUNIT_ASSERT(storage->m_address == addr1);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10000);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 0);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_NONE);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(error, addr2, hostCheck);
    healthCheck->responseTime = -1;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr2;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(error, hostCheck, addr2, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_NONE);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_NONE);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == error);
    CPPUNIT_ASSERT(storage->m_address == addr2);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10000);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 0);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_NONE);
    delete healthCheck;

    std::this_thread::sleep_for(500ms);

    CPPUNIT_ASSERT_EQUAL(7, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress == addr1);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress == addr2);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT(work->m_hostname == failure);
    CPPUNIT_ASSERT(work->m_ipAddress == addr1);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT(work->m_hostname == failure);
    CPPUNIT_ASSERT(work->m_ipAddress == addr2);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT(work->m_hostname == error);
    CPPUNIT_ASSERT(work->m_ipAddress == addr1);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT(work->m_hostname == error);
    CPPUNIT_ASSERT(work->m_ipAddress == addr2);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT(work->m_hostname == basic);
    CPPUNIT_ASSERT(work->m_ipAddress == addr1);
}

void
TESTNAME::test_HMWorkHealthCheck_IPV6()
{
    TestStorage* storage = dynamic_cast<TestStorage*>(m_currentState->m_datastore.get());
    HMDNSLookup dnsHostCheckT(HM_DNS_TYPE_LOOKUP, true);
    string checkParams = "/check.html";
    HMDataHostCheck hostCheck;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV6_ONLY);
	hostGroup.setCheckInfo(checkParams);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	hostCheck.setCheckParams(hostGroup);

    HMDataCheckParams paramsNormal;
    HMDataCheckParams paramsTimedout;

    paramsNormal.setCheckParameters(3,
            100,
            0,
            1000,
            5,
            10,
            3,
            10000,
            100, HM_DEFAULT_FLAP_THRESHOLD, 0);

    paramsTimedout.setCheckParameters(3,
            0,
            0,
            500,
            2,
            10,
            3,
            10000,
            0, HM_DEFAULT_FLAP_THRESHOLD, 0);

    string basic = "ipv6.basic.hm.com";
    string success = "ipv6.success.hm.com";
    string failure = "ipv6.failure.hm.com";
    string error = "ipv6.error.hm.com";

    HMIPAddress addr1;
    HMIPAddress addr2;
    addr1.set("fd01::1");
    addr2.set("fd01::2");
    set<HMIPAddress> ips;
    ips.insert(addr1);
    ips.insert(addr2);

    set<HMIPAddress> ips1;
    ips1.insert(addr1);

    m_currentState->m_checkList.insertCheck("HostGroup1", basic, hostCheck, paramsNormal, ips1);

    m_currentState->m_checkList.insertCheck("HostGroup1", success, hostCheck, paramsNormal, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", success, hostCheck, paramsTimedout, ips);

    m_currentState->m_checkList.insertCheck("HostGroup1", failure, hostCheck, paramsNormal, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", failure, hostCheck, paramsTimedout, ips);

    m_currentState->m_checkList.insertCheck("HostGroup1", error, hostCheck, paramsNormal, ips);
    m_currentState->m_checkList.insertCheck("HostGroup2", error, hostCheck, paramsTimedout, ips);
    m_currentState->m_dnsCache.insertDNSEntry(basic, dnsHostCheckT, 10000, 10000);
    m_currentState->m_dnsCache.insertDNSEntry(success, dnsHostCheckT, 10000, 10000);
    m_currentState->m_dnsCache.insertDNSEntry(failure, dnsHostCheckT, 10000, 10000);
    m_currentState->m_dnsCache.insertDNSEntry(error, dnsHostCheckT, 10000, 10000);
    m_currentState->m_dnsCache.updateDNSEntry(basic, dnsHostCheckT, ips1);
    m_currentState->m_dnsCache.updateDNSEntry(success, dnsHostCheckT, ips);
    m_currentState->m_dnsCache.updateDNSEntry(failure, dnsHostCheckT, ips);
    m_currentState->m_dnsCache.updateDNSEntry(error, dnsHostCheckT, ips);
    m_currentState->m_dnsCache.finishQuery(basic, dnsHostCheckT, true);
    m_currentState->m_dnsCache.finishQuery(success, dnsHostCheckT, true);
    m_currentState->m_dnsCache.finishQuery(failure, dnsHostCheckT, true);
    m_currentState->m_dnsCache.finishQuery(error, dnsHostCheckT, true);

    std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> getResults;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(basic, addr1, hostCheck);
    healthCheck->responseTime = 10;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr1;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(basic, hostCheck, addr1, getResults);
    CPPUNIT_ASSERT(getResults.size() == 1);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 1);
    CPPUNIT_ASSERT(storage->m_hostname == basic);
    CPPUNIT_ASSERT(storage->m_address == addr1);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsNormal);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 10);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_SUCCESS);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(success, addr1, hostCheck);
    healthCheck->responseTime = 10;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr1;
    storage->m_writeCount = 0;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(success, hostCheck, addr1, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_SUCCESS);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == success);
    CPPUNIT_ASSERT(storage->m_address == addr1);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 10);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_SUCCESS);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(success, addr2, hostCheck);
    healthCheck->responseTime = 10;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr2;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(success, hostCheck, addr2, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_SUCCESS);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_SUCCESS);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == success);
    CPPUNIT_ASSERT(storage->m_address == addr2);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 10);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_CONNECTED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_SUCCESS);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(failure, addr1, hostCheck);
    healthCheck->responseTime = 0;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr1;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(failure, hostCheck, addr1, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_CONNECT_FAILURE);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_CONNECT_FAILURE);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == failure);
    CPPUNIT_ASSERT(storage->m_address == addr1);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10000);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 0);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_CONNECT_FAILURE);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(failure, addr2, hostCheck);
    healthCheck->responseTime = 0;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr2;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(failure, hostCheck, addr2, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_CONNECT_FAILURE);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_CONNECT_FAILURE);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == failure);
    CPPUNIT_ASSERT(storage->m_address == addr2);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10000);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 0);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_FAILED);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_CONNECT_FAILURE);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(error, addr1, hostCheck);
    healthCheck->responseTime = -1;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr1;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(error, hostCheck, addr1, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_NONE);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr1);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_NONE);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == error);
    CPPUNIT_ASSERT(storage->m_address == addr1);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10000);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 0);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_NONE);
    delete healthCheck;

    storage->m_writeCount = 0;
    healthCheck = new TestHMWorkHealthCheck(error, addr2, hostCheck);
    healthCheck->responseTime = -1;
    healthCheck->updateState(&m_state, m_eventQueue);
    healthCheck->m_ipAddress = addr2;

    CPPUNIT_ASSERT(healthCheck->processWork() == HM_WORK_COMPLETE);

    // query the results directly
    m_currentState->m_checkList.getCheckResults(error, hostCheck, addr2, getResults);
    CPPUNIT_ASSERT(getResults.size() == 2);

    CPPUNIT_ASSERT(getResults[0].first == paramsNormal);
    CPPUNIT_ASSERT(getResults[0].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[0].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[0].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[0].second.m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(getResults[0].second.m_reason == HM_REASON_NONE);

    CPPUNIT_ASSERT(getResults[1].first == paramsTimedout);
    CPPUNIT_ASSERT(getResults[1].second.m_address == addr2);
    CPPUNIT_ASSERT(getResults[1].second.m_numChecks == 1);
    CPPUNIT_ASSERT(getResults[1].second.m_responseTime == 10000);
    CPPUNIT_ASSERT(getResults[1].second.m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(getResults[1].second.m_reason == HM_REASON_NONE);

    // Check the store class
    CPPUNIT_ASSERT(storage->m_writeCount == 2);
    CPPUNIT_ASSERT(storage->m_hostname == error);
    CPPUNIT_ASSERT(storage->m_address == addr2);
    CPPUNIT_ASSERT(storage->m_checkParams == paramsTimedout);
    CPPUNIT_ASSERT(storage->m_checkResult.m_numChecks == 1);
    CPPUNIT_ASSERT(storage->m_checkResult.m_responseTime == 10000);
    CPPUNIT_ASSERT(storage->m_end - storage->m_start == 0);
    CPPUNIT_ASSERT(storage->m_response == HM_RESPONSE_NONE);
    CPPUNIT_ASSERT(storage->m_reason == HM_REASON_NONE);
    delete healthCheck;

    std::this_thread::sleep_for(500ms);

    CPPUNIT_ASSERT_EQUAL(7, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT_EQUAL(work->m_hostname, success);
    CPPUNIT_ASSERT(work->m_ipAddress == addr1);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT_EQUAL(work->m_hostname, success);
    CPPUNIT_ASSERT(work->m_ipAddress == addr2);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT_EQUAL(work->m_hostname, failure);
    CPPUNIT_ASSERT(work->m_ipAddress == addr1);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT_EQUAL(work->m_hostname, failure);
    CPPUNIT_ASSERT(work->m_ipAddress == addr2);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT_EQUAL(work->m_hostname, error);
    CPPUNIT_ASSERT(work->m_ipAddress == addr1);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT_EQUAL(work->m_hostname, error);
    CPPUNIT_ASSERT(work->m_ipAddress == addr2);

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT_EQUAL(work->m_hostname, basic);
    CPPUNIT_ASSERT(work->m_ipAddress == addr1);
}
