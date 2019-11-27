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

