// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMTimeStamp.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include "HMTimeStamp.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void TESTNAME::setUp() {

}

void TESTNAME::tearDown() {

}

void TESTNAME::test_timestamp1() {
    CPPUNIT_ASSERT_EQUAL(1,1);

    HMTimeStamp ts1,ts2,ts3;
    ts1 = HMTimeStamp::now();
    ts2 = ts1;
    uint64_t offset =4;
    ts3 = ts1 + offset;
    CPPUNIT_ASSERT_EQUAL(ts1 <= ts2, true);
    CPPUNIT_ASSERT_EQUAL(ts1 < ts3, true);

}

void TESTNAME::test_timestamp2() {
    HMTimeStamp ts1,ts2,ts3,ts4;
    ts1 = HMTimeStamp::now();
    uint64_t offset =4;
    ts3 = offset + ts1;
    ts2 = ts1 - offset;
    CPPUNIT_ASSERT_EQUAL(ts3 > ts2, true);
    CPPUNIT_ASSERT_EQUAL(ts1 <= ts3, true);
    CPPUNIT_ASSERT_EQUAL(8,int(ts3 - ts2));
}

void TESTNAME::test_timestamp3() {
    HMTimeStamp ts1,ts2;

    uint64_t time = 1566465545000;
    ts1.setTime(time);

    string outputTime = ts1.print("%Y-%m-%dT%H:%M:%SZ");
    string expectedTime = "2019-08-22T09:19:05Z";
    CPPUNIT_ASSERT_EQUAL(expectedTime, outputTime);
    CPPUNIT_ASSERT_EQUAL(time,ts1.getTimeSinceEpoch());

    string time2 = "2019-8-22T09:19:5Z";
    ts2.setTime(time2,"%Y-%m-%dT%H:%M:%SZ");
    CPPUNIT_ASSERT_EQUAL(time,ts2.getTimeSinceEpoch());
}

