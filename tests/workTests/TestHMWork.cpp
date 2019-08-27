// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMWork.h"
#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMEventLoopQueue.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void
TESTNAME::setUp()
{}

void
TESTNAME::tearDown()
{}

void
TESTNAME::test_HMWork_Construction()
{
    string hostname = "test.host.hm.com";
    HMIPAddress addr;
    addr.set("192.168.0.1");
    HMDataHostCheck hostCheck;

    TestHMWork* test = new TestHMWork();
    CPPUNIT_ASSERT(test->m_hostname == "");
    CPPUNIT_ASSERT_EQUAL((int)test->m_response, (int)HM_RESPONSE_NONE);
    CPPUNIT_ASSERT_EQUAL((int)test->m_reason, (int)HM_REASON_NONE);
    CPPUNIT_ASSERT_EQUAL((int)test->m_ID, 0);
    CPPUNIT_ASSERT(test->getStateManager() == nullptr);
    delete test;

    test = new TestHMWork(hostname, addr, hostCheck);
    CPPUNIT_ASSERT(test->m_hostname == hostname);
    CPPUNIT_ASSERT(test->m_ipAddress == addr);
    CPPUNIT_ASSERT(!(test->m_hostCheck != hostCheck));
    CPPUNIT_ASSERT_EQUAL((int)test->m_response, (int)HM_RESPONSE_NONE);
    CPPUNIT_ASSERT_EQUAL((int)test->m_reason, (int)HM_REASON_NONE);
    CPPUNIT_ASSERT_EQUAL((int)test->m_ID, 0);
    CPPUNIT_ASSERT(test->getStateManager() == nullptr);
    delete test;
}

void
TESTNAME::test_HMWork_updateState()
{
    shared_ptr<HMState> current;
    HMStateManager state;
    state.updateState(current);
    HMEventLoopQueue eventQueue(&state);

    TestHMWork test;
    test.updateState(&state, &eventQueue);

    CPPUNIT_ASSERT(test.getStateManager() == &state);

}

void
TESTNAME::test_HMWorkState_Construction()
{
    ares_channel ares;
    ares_init(&ares);

    // Test the default constructor
    HMWorkState* hmWorkState = new HMWorkState();
    CPPUNIT_ASSERT(!hmWorkState->m_aresLoaded);
    CPPUNIT_ASSERT(hmWorkState->m_channel == 0);

    delete hmWorkState;

    // Test the 2nd constructor
    hmWorkState = new HMWorkState(ares, true);
    CPPUNIT_ASSERT(hmWorkState->m_channel == ares);
    CPPUNIT_ASSERT(hmWorkState->m_aresLoaded);
    delete hmWorkState;
    ares_destroy(ares);
}

void
TESTNAME::test_HMWorkReloadState()
{
    HMWorkState* hmWorkState = new HMWorkState();

    shared_ptr<HMState> currentState = make_shared<HMState>();
    HMStateManager state;
    uint64_t threadID = 0xFF;
    state.setState(currentState);
    hmWorkState->reloadState(&state, threadID);
    CPPUNIT_ASSERT(hmWorkState->m_aresLoaded);
    ares_destroy(hmWorkState->m_channel);

    currentState->m_dnsCache.init();
    hmWorkState->reloadState(&state, threadID);
    CPPUNIT_ASSERT(hmWorkState->m_aresLoaded);
    ares_destroy(hmWorkState->m_channel);

    HMIPAddress server;
    server.set("192.168.0.1");
    shared_ptr<HMState> checkState;
    state.updateState(checkState);
    checkState->setDNSServer(server);
    checkState.reset();

    hmWorkState->reloadState(&state, threadID);
    CPPUNIT_ASSERT(hmWorkState->m_aresLoaded);
    ares_destroy(hmWorkState->m_channel);

    server.set("fd01::1");
    state.updateState(checkState);
    checkState->setDNSServer(server);
    checkState.reset();
    hmWorkState->reloadState(&state, threadID);
    CPPUNIT_ASSERT(hmWorkState->m_aresLoaded);
    ares_destroy(hmWorkState->m_channel);

    HMIPAddress server2;
    state.updateState(checkState);
    checkState->setDNSServer(server2);
    checkState.reset();
    hmWorkState->reloadState(&state, threadID);
    CPPUNIT_ASSERT(hmWorkState->m_aresLoaded);
    ares_destroy(hmWorkState->m_channel);

    delete hmWorkState;
}



