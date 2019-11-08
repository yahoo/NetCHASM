// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TestEventQueue_H_
#define TestEventQueue_H_

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkDNSLookup.h"
#include "HMStateManager.h"
#include "HMDNSCache.h"
#include "HMEventLoopQueue.h"


#define TESTNAME TestEventQueue

class TESTNAME : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
#ifdef USE_ARES
    CPPUNIT_TEST(test_basic_DNS_eventqueue);
#endif
    CPPUNIT_TEST(test_basic_HC_eventqueue);
    CPPUNIT_TEST(test_ordering_HC_eventqueue);
    CPPUNIT_TEST(test_delay_HC_eventqueue);
    CPPUNIT_TEST_SUITE_END();


public:
	std::shared_ptr<HMState> m_currentState;
	HMStateManager m_state;
	HMEventLoopQueue* m_eventQueue;
	std::thread* m_eventThread;
    void setUp();
    void tearDown();

protected:
    void test_basic_DNS_eventqueue();
    void test_basic_HC_eventqueue();
    void test_ordering_HC_eventqueue();
    void test_delay_HC_eventqueue();
};

#endif /* TestEventQueue_H_ */

