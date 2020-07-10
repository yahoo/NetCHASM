// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMRemoteResult.h"

#include "HMRemoteResult.h"
#include "HMStateManager.h"
#include "HMEventLoopQueue.h"
#include "common.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <set>

using namespace std;
CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp()
{
    setupCommon();
}

void TESTNAME::tearDown()
{
    teardownCommon();
}

void TESTNAME::test_next_query()
{

    HMRemoteResult results;
    uint64_t query_timeout = 500;
    results.updateTimeouts(100, query_timeout);
    HMTimeStamp start_time = results.startCheck();
    HMTimeStamp next_time = results.nextCheckTime();
    CPPUNIT_ASSERT_EQUAL(start_time.getTimeSinceEpoch() + query_timeout,
            next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_IN_PROGRESS, results.getCheckState());
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    next_time = results.nextCheckTime();
    CPPUNIT_ASSERT(
            (start_time.getTimeSinceEpoch() + query_timeout)
                    < next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_IN_PROGRESS, results.getCheckState());
}

void TESTNAME::test_next_query1()
{

    HMRemoteResult results;
    uint64_t remote_timeout = 500;
    results.updateTimeouts(remote_timeout, 100);
    HMTimeStamp result_time = HMTimeStamp::now();
    results.finishCheck(true);
    HMTimeStamp next_time = results.nextCheckTime();
    CPPUNIT_ASSERT(
            (result_time.getTimeSinceEpoch() + remote_timeout)
                    >= next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, results.getCheckState());
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    next_time = results.nextCheckTime();
    CPPUNIT_ASSERT(
            (result_time.getTimeSinceEpoch() + remote_timeout)
                    < next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, results.getCheckState());
}

void TESTNAME::test_next_query2()
{

    HMRemoteResult results;
    uint64_t remote_timeout = 500;
    results.updateTimeouts(remote_timeout, 100);
    HMTimeStamp result_time = HMTimeStamp::now();
    results.finishCheck(false);
    HMTimeStamp next_time = results.nextCheckTime();
    CPPUNIT_ASSERT(
            (result_time.getTimeSinceEpoch() + remote_timeout)
                    >= next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_FAILED, results.getCheckState());
}

void TESTNAME::test_queue_query()
{

    HMRemoteResult results;
    results.queueCheck();
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_QUEUED, results.getCheckState());
}
