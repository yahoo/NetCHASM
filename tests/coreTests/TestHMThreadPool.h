// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef TEST_HMThreadPool_H_
#define TEST_HMThreadPool_H_

#include <memory>

#include <cppunit/Test.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "HMWorkQueue.h"

#define TESTNAME Test_HMThreadPool

class TestHMThreadPoolWork: public HMWork
{
    HM_WORK_STATUS processWork()
    {
        std::unique_lock<std::mutex> testLock(m_testMutex, std::defer_lock);
        testLock.lock();
        m_testCond.wait(testLock);
        testLock.unlock();
        return HM_WORK_COMPLETE;
    }
    void init(HMWorkState& state)
    {

    }
public:
    HM_WORK_TYPE getWorkType() { return HM_WORK_HEALTHCHECK; }
    std::mutex m_testMutex;
    std::condition_variable m_testCond;
    TestHMThreadPoolWork() {}
    TestHMThreadPoolWork(TestHMThreadPoolWork& test) {}
};

class TESTNAME: public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TESTNAME);
    CPPUNIT_TEST(test_basic_threadpool);
    CPPUNIT_TEST(test_resize_threadpool);
    CPPUNIT_TEST(test_idle_threadpool);CPPUNIT_TEST_SUITE_END();

public:

    void setUp();
    void tearDown();
    void test_basic_threadpool();
    void test_resize_threadpool();
    void test_idle_threadpool();
protected:
    std::unique_ptr<HMStateManager> sm;
    std::unique_ptr<HMThreadPool> tp;
};

#endif /* TEST_HMThreadPool_H_ */

