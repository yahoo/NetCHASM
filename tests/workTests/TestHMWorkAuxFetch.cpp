// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMWorkAuxFetch.h"
#include "common.h"
#include "TestStorage.h"
using namespace std;

HM_WORK_STATUS TestHMWorkAuxFetch::fetchAux()
{
    shared_ptr<HMState> currentState;

    if (m_hostCheck.getCheckType() == HM_CHECK_AUX_HTTP
                || m_hostCheck.getCheckType() == HM_CHECK_AUX_HTTPS
                || m_hostCheck.getCheckType() == HM_CHECK_AUX_HTTPS_NO_PEER_CHECK)
    {

        m_stateManager->updateState(currentState);

        m_start = HMTimeStamp::now();
        m_response = HM_RESPONSE_CONNECTED;
        m_end = HMTimeStamp::now()+100;
        return HM_WORK_COMPLETE;
    }

    return HM_WORK_COMPLETE;
}


CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void
TESTNAME::setUp()
{
    setupCommon();
    HMDataHostGroupMap hostGroupMap;
    m_currentState = make_shared<HMState>();
    m_currentState->m_datastore = make_unique<TestStorage>(&hostGroupMap);

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
TESTNAME::test_HMWorkAuxFetch_Process_Success()
{

    HMDataHostCheck check1;
    HMDataCheckParams params;
    string checkParams = "/check.html";
    check1.setCheckParams(HM_CHECK_AUX_HTTP,
            HM_CHECK_PLUGIN_AUX_CURL,
            80,
            HM_DUALSTACK_UNDEFINED,
            checkParams);

    params.setCheckParameters(3, 700, 0, 0, 0, 0, 0, 10000, 800, 0, 0);


    string success = "ipv4.success.hm.com";
    string successv6 = "ipv6.success.hm.com";

    HMIPAddress addr,addrv6;
    addr.set("192.168.0.1");
    addrv6.set("1::9");
    set<HMIPAddress> addrs;
    set<HMIPAddress> addrsv6;
    addrs.insert(addr);
    addrsv6.insert(addrv6);

    m_currentState->m_checkList.insertCheck("HostGroup1", success, check1, params, addrs);
    m_currentState->m_checkList.insertCheck("HostGroup2", successv6, check1, params, addrsv6);

    HMDataHostCheck hostCheck;
    vector<HMCheckHeader> allChecks;



    //IPV4 success
    fetchAux = new TestHMWorkAuxFetch(success, addr, check1);
    fetchAux->updateState(&m_state, m_eventQueue);


    CPPUNIT_ASSERT(fetchAux->processWork() == HM_WORK_COMPLETE);

    m_currentState->m_checkList.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL(success,allChecks[0].m_hostname);

    delete fetchAux;

    //IPV6 success
    fetchAux = new TestHMWorkAuxFetch(successv6, addrv6, check1);
    fetchAux->updateState(&m_state, m_eventQueue);


    CPPUNIT_ASSERT(fetchAux->processWork() == HM_WORK_COMPLETE);

    m_currentState->m_checkList.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL(successv6,allChecks[1].m_hostname);

    delete fetchAux;

    // sleep and make sure all required work is present
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL(2, (int)m_state.m_workQueue.queueSize());


    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == success);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == successv6);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "1::9");

}

void
TESTNAME::test_HMWorkAuxFetch_Process_Timeout()
{



    HMDataHostCheck check1;
    HMDataCheckParams params;
    string checkParams = "/check.html";
    check1.setCheckParams(HM_CHECK_AUX_HTTP,
            HM_CHECK_PLUGIN_AUX_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            checkParams);

    params.setCheckParameters(3, 10000, 0, 0, 0, 0, 0, 1000, 10000, 0, 0);


    check1.setCheckParams(HM_CHECK_AUX_HTTP,
            HM_CHECK_PLUGIN_AUX_CURL,
            443,
            HM_DUALSTACK_UNDEFINED,
            checkParams);

    string timedout = "ipv4.timedout.hm.com";
    string timedoutv6 = "ipv6.timedout.hm.com";

    HMIPAddress addr,addrv6;
    addr.set("192.168.0.1");
    addrv6.set("1::9");
    set<HMIPAddress> addrs;
    set<HMIPAddress> addrsv6;
    addrs.insert(addr);
    addrsv6.insert(addrv6);
    m_currentState->m_checkList.insertCheck("HostGroup1", timedout, check1, params, addrs);
    m_currentState->m_checkList.insertCheck("HostGroup2", timedoutv6, check1, params, addrsv6);


    HMDataHostCheck hostCheck;
    vector<HMCheckHeader> allChecks;



    //IPv4 timeout
    fetchAux = new TestHMWorkAuxFetch(timedout, addr, check1);
    fetchAux->updateState(&m_state, m_eventQueue);


    CPPUNIT_ASSERT(fetchAux->processWork() == HM_WORK_COMPLETE);

    m_currentState->m_checkList.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL(timedout,allChecks[0].m_hostname);

    delete fetchAux;

    //IPv6 timeout
    fetchAux = new TestHMWorkAuxFetch(timedoutv6, addrv6, check1);
    fetchAux->updateState(&m_state, m_eventQueue);


    CPPUNIT_ASSERT(fetchAux->processWork() == HM_WORK_COMPLETE);

    m_currentState->m_checkList.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL(timedoutv6,allChecks[1].m_hostname);

    delete fetchAux;

    // sleep and make sure all required work is present
    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL(0, (int)m_state.m_workQueue.queueSize());

    // Now check the work list
    unique_ptr<HMWork> work;
    bool threadStatus = false;
    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == timedout);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "192.168.0.1");

    m_state.m_workQueue.getWork(work, threadStatus);
    CPPUNIT_ASSERT(!(work->m_hostCheck != check1));
    CPPUNIT_ASSERT(work->m_hostname == timedoutv6);
    CPPUNIT_ASSERT(work->m_ipAddress.toString() == "1::9");

}

void
TESTNAME::test_HMWorkAuxFetch_Process_Failure()
{

    HMDataHostCheck check1;
    HMDataCheckParams params;
    string checkParams = "/check.html";
    check1.setCheckParams(HM_CHECK_HTTP,
            HM_CHECK_PLUGIN_AUX_CURL,
            80,
            HM_DUALSTACK_UNDEFINED,
            checkParams);

    params.setCheckParameters(3, 1000, 0, 0, 0, 0, 0, 1000, 1000, 0, 0);

    string failure = "ipv4.failure.hm.com";
    string failurev6 = "ipv6.failure.hm.com";

    HMIPAddress addr,addrv6;
    addr.set("192.168.0.1");
    addrv6.set("1::7");
    set<HMIPAddress> addrs;
    set<HMIPAddress> addrsv6;
    addrs.insert(addr);
    addrsv6.insert(addrv6);

    m_currentState->m_checkList.insertCheck("HostGroup1", failure, check1, params, addrs);
    m_currentState->m_checkList.insertCheck("HostGroup2", failurev6, check1, params, addrsv6);

    HMDataHostCheck hostCheck;
    vector<HMCheckHeader> allChecks;

    //ipv4 failure
    fetchAux = new TestHMWorkAuxFetch(failure, addr, check1);
    fetchAux->updateState(&m_state, m_eventQueue);


    CPPUNIT_ASSERT(fetchAux->processWork() == HM_WORK_COMPLETE);

    m_currentState->m_checkList.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL(failure,allChecks[0].m_hostname);

    delete fetchAux;

   //ipv6 failure
    fetchAux = new TestHMWorkAuxFetch(failurev6, addrv6, check1);
    fetchAux->updateState(&m_state, m_eventQueue);


    CPPUNIT_ASSERT(fetchAux->processWork() == HM_WORK_COMPLETE);

    m_currentState->m_checkList.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL(failurev6,allChecks[1].m_hostname);

    delete fetchAux;

    std::this_thread::sleep_for(1s);

    CPPUNIT_ASSERT_EQUAL(0, (int)m_state.m_workQueue.queueSize());

}
