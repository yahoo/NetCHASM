// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMRemoteHostCache.h"

#include "HMRemoteHostCache.h"
#include "HMDataHostCheck.h"

#include "common.h"
#include <unistd.h>
#include <arpa/inet.h>

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

/*
 * This test create entry into DNS Cache with specified ttl. after starting query we
 * verify the query needed status and the next querytime.
 */
void TESTNAME::test_basic_cache()
{
    HMRemoteHostCache hmremote;
    string hostname = "Dummy.hm.com";
    HMDataHostCheck dataHostCheck;
    hmremote.insertRemoteEntry(hostname, dataHostCheck, 1000, 1000);
    hmremote.startRemoteCheck(hostname, dataHostCheck);
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmremote.checkNeeded(hostname, dataHostCheck));
    sleep(2);
}

/*
 * This test create entry into DNS Cache. We update the results with IPV4 address and
 * check the get addresses function of the DNS Cache.
 */
void TESTNAME::test_basic_cache1()
{
    HMRemoteHostCache hmremote;
    string hostname = "Dummy.hm.com";
    HMDataHostCheck dataHostCheck;
    hmremote.insertRemoteEntry(hostname, dataHostCheck, 10000,10000);
    hmremote.finishCheck(hostname, dataHostCheck, true);
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmremote.checkNeeded(hostname, dataHostCheck));
}

