// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMDNSResult.h"

#include "HMDNSCache.h"
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

void TESTNAME::test_get_address()
{

    HMDNSResult results;
    HMIPAddress ip, ip_recv;
    in_addr addr;
    addr.s_addr = inet_addr("192,168.1.1");
    ip.set((char*) &addr, AF_INET);
    set<HMIPAddress> ips;
    ips.insert(ip);
    results.updateQuery(ips);
    CPPUNIT_ASSERT_EQUAL(true, results.getAddress(ip_recv));
    CPPUNIT_ASSERT_EQUAL(ip.addr4(), ip_recv.addr4());
}

void TESTNAME::test_get_addresses()
{

    HMDNSResult results;
    HMIPAddress ip, ip1, ip_recv;
    in_addr addr;
    set<HMIPAddress> ips;
    addr.s_addr = inet_addr("192.168.1.1");
    ip.set((char*) &addr, AF_INET);
    ips.insert(ip);
    addr.s_addr = inet_addr("192.168.1.2");
    ip1.set((char*) &addr, AF_INET);
    ips.insert(ip1);
    results.updateQuery(ips);
    set<HMIPAddress> addresses;
    CPPUNIT_ASSERT_EQUAL(true, results.getAddresses(addresses));
    CPPUNIT_ASSERT_EQUAL(2, (int )addresses.size());
    CPPUNIT_ASSERT(addresses.find(ip) != addresses.end());
    CPPUNIT_ASSERT(addresses.find(ip1) != addresses.end());
}

void TESTNAME::test_next_query()
{

    HMDNSResult results;
    uint64_t query_timeout = 500;
    results.updateTimeouts(100, query_timeout);
    HMTimeStamp start_time = results.startQuery();
    HMTimeStamp next_time = results.nextQueryTime();
    CPPUNIT_ASSERT_EQUAL(start_time.getTimeSinceEpoch() + query_timeout,
            next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_IN_PROGRESS, results.getQueryState());
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    next_time = results.nextQueryTime();
    CPPUNIT_ASSERT(
            (start_time.getTimeSinceEpoch() + query_timeout)
                    < next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_IN_PROGRESS, results.getQueryState());
}

void TESTNAME::test_next_query1()
{

    HMDNSResult results;
    uint64_t dns_timeout = 500;
    results.updateTimeouts(dns_timeout, 100);
    HMTimeStamp result_time = HMTimeStamp::now();
    results.finishQuery(true);
    HMTimeStamp next_time = results.nextQueryTime();
    CPPUNIT_ASSERT(
            (result_time.getTimeSinceEpoch() + dns_timeout)
                    >= next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, results.getQueryState());
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    next_time = results.nextQueryTime();
    CPPUNIT_ASSERT(
            (result_time.getTimeSinceEpoch() + dns_timeout)
                    < next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, results.getQueryState());
}

void TESTNAME::test_next_query2()
{

    HMDNSResult results;
    uint64_t dns_timeout = 500;
    results.updateTimeouts(dns_timeout, 100);
    HMTimeStamp result_time = HMTimeStamp::now();
    results.finishQuery(false);
    HMTimeStamp next_time = results.nextQueryTime();
    CPPUNIT_ASSERT(
            (result_time.getTimeSinceEpoch() + dns_timeout)
                    >= next_time.getTimeSinceEpoch());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_FAILED, results.getQueryState());
}

void TESTNAME::test_queue_query()
{

    HMDNSResult results;
    results.queueQuery();
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_QUEUED, results.getQueryState());
}


void TESTNAME::test_get_exp_addresses()
{

    HMDNSResult results;
    set<HMIPAddress> ips;
    HMIPAddress ip, ip1, ip_recv;
    in_addr addr;
    addr.s_addr = inet_addr("192.168.1.1");
    ip.set((char*) &addr, AF_INET);

    addr.s_addr = inet_addr("192.168.1.2");
    ip1.set((char*) &addr, AF_INET);
    ips.insert(ip);
    ips.insert(ip1);
    results.updateQuery(ips);

    set<HMIPAddress> addresses;
    CPPUNIT_ASSERT_EQUAL(true, results.getAddresses(addresses));
    CPPUNIT_ASSERT_EQUAL(2, (int )addresses.size());
    CPPUNIT_ASSERT(addresses.find(ip) != addresses.end());
    CPPUNIT_ASSERT(addresses.find(ip1) != addresses.end());

    HMIPAddress ip2, ip3;
    ips.clear();
    addr.s_addr = inet_addr("192.168.1.3");
    ip2.set((char*) &addr, AF_INET);
    addr.s_addr = inet_addr("192.168.1.4");
    ip3.set((char*) &addr, AF_INET);
    ips.insert(ip2);
    ips.insert(ip3);
    results.updateQuery(ips);
    addresses.clear();
    CPPUNIT_ASSERT_EQUAL(true, results.getAddresses(addresses));
    CPPUNIT_ASSERT_EQUAL(2, (int )addresses.size());
    CPPUNIT_ASSERT(addresses.find(ip2) != addresses.end());
    CPPUNIT_ASSERT(addresses.find(ip3) != addresses.end());
    addresses.clear();
    CPPUNIT_ASSERT_EQUAL(true, results.getExpiredAddresses(addresses));
    CPPUNIT_ASSERT_EQUAL(2, (int )addresses.size());
    CPPUNIT_ASSERT(addresses.find(ip) != addresses.end());
    CPPUNIT_ASSERT(addresses.find(ip1) != addresses.end());

    results.updateQuery(ips);
    addresses.clear();
    CPPUNIT_ASSERT_EQUAL(true, results.getAddresses(addresses));
    CPPUNIT_ASSERT_EQUAL(2, (int )addresses.size());
    CPPUNIT_ASSERT(addresses.find(ip2) != addresses.end());
    CPPUNIT_ASSERT(addresses.find(ip3) != addresses.end());
    addresses.clear();
    CPPUNIT_ASSERT_EQUAL(false, results.getExpiredAddresses(addresses));
    CPPUNIT_ASSERT_EQUAL(0, (int )addresses.size());

    HMIPAddress ip4;
    ips.clear();
    addr.s_addr = inet_addr("192.168.1.4");
    ip3.set((char*) &addr, AF_INET);
    addr.s_addr = inet_addr("192.168.1.5");
    ip4.set((char*) &addr, AF_INET);
    ips.insert(ip3);
    ips.insert(ip4);
    results.updateQuery(ips);
    addresses.clear();
    CPPUNIT_ASSERT_EQUAL(true, results.getAddresses(addresses));
    CPPUNIT_ASSERT_EQUAL(2, (int )addresses.size());
    CPPUNIT_ASSERT(addresses.find(ip3) != addresses.end());
    CPPUNIT_ASSERT(addresses.find(ip4) != addresses.end());
    addresses.clear();
    CPPUNIT_ASSERT_EQUAL(true, results.getExpiredAddresses(addresses));
    CPPUNIT_ASSERT_EQUAL(1, (int )addresses.size());
    CPPUNIT_ASSERT(addresses.find(ip2) != addresses.end());


}
