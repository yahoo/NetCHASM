// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include <unistd.h>
#include <memory>

#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMEventLoopQueue.h"
#include "HMThreadPool.h"
#include "common.h"
#include "TestHMThreadPool.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    sm = std::make_unique<HMStateManager>();
    setupCommon();
    shared_ptr<HMState> m_currentState = make_shared<HMState>();
    sm->setState(m_currentState);
    HMEventLoopQueue eq(sm.get());
    tp = std::make_unique<HMThreadPool>(sm.get(),&eq);
}

void TESTNAME::tearDown()
{
    tp->shutdown();
    tp.reset();
    teardownCommon();
    sm.reset();
}
/*
 * This test create three threads and since no work is assigned to them
 * they are all idle. We then shutdown all the threads.
 */
void TESTNAME::test_basic_threadpool()
{
    tp->resize(3);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    CPPUNIT_ASSERT_EQUAL(3,(int)tp->countIdle());
}

/*
 * This test create three threads and since no work is assigned to them
 * they are all idle. We then increase the thread pool to 6 and we test
 * is all the threads are idle. We then reduce the thread pool size to 2
 * and check if only two all idle. The other threads should be shut down.
 */
void TESTNAME::test_resize_threadpool()
{
    tp->resize(3);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    CPPUNIT_ASSERT_EQUAL(3,(int)tp->countIdle());
    tp->resize(6);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    CPPUNIT_ASSERT_EQUAL(6,(int)tp->countIdle());
    tp->resize(2);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    CPPUNIT_ASSERT_EQUAL(2,(int)tp->countIdle());
}

/*
 * This test create three threads and we assign a work which waits on conditional
 * variable. So we test if only 2 threads are idle. After we notify the locked thread
 * it should come back to the idle status. We then test if all three threads in the
 * pool are idle.
 */
void TESTNAME::test_idle_threadpool()
{
    tp->resize(3);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    TestHMThreadPoolWork test_hm;
    std::unique_ptr<HMWork> ptr_test = std::make_unique<TestHMThreadPoolWork>(test_hm);
    HMWork* ptr = ptr_test.get();
    sm->m_workQueue.insertWork(ptr_test);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    CPPUNIT_ASSERT_EQUAL(2,(int)tp->countIdle());
    std::lock_guard<std::mutex> lg(test_hm.m_testMutex);
    static_cast<TestHMThreadPoolWork*>(ptr)->m_testCond.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    CPPUNIT_ASSERT_EQUAL(3,(int)tp->countIdle());
}
