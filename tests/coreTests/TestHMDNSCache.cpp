// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMDNSCache.h"

#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMEventLoopQueue.h"
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
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    hmdns.insertDNSEntry(hostname, false, 1000, 1000);
    HMTimeStamp starttime = hmdns.startDNSQuery(hostname, false);
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, false));
    sleep(2);
    CPPUNIT_ASSERT(
            ((starttime + 1000) <= hmdns.nextQueryTime(hostname, false)));
}

/*
 * This test create entry into DNS Cache. We update the results with IPV4 address and
 * check the get addresses function of the DNS Cache.
 */
void TESTNAME::test_update_results()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip_ret;
    ip.set("192.168.1.1");
    set<HMIPAddress> vip, vip_ret;
    vip.insert(ip);
    hmdns.updateDNSEntry(hostname, false, vip);
    hmdns.finishQuery(hostname, false, true);
    CPPUNIT_ASSERT_EQUAL(false,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY, vip_ret));
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, false));
}

/*
 * This test create entry into DNS Cache. We update the results with multiple IPV4 address and
 * check the get addresses function of the DNS Cache.
 */
void TESTNAME::test_update_results_ipv4s()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1;
    ip.set("192.168.1.1");
    ip1.set("192.168.1.2");
    set<HMIPAddress> vip, vip_ret,vip_diff;
    vip.insert(ip);
    vip.insert(ip1);
    hmdns.updateDNSEntry(hostname, false, vip);
    hmdns.finishQuery(hostname, false, true);

    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY, vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_ret.size());
    set_intersection(vip.begin(), vip.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_diff.size());
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, false));
}

void TESTNAME::test_get_expired_ipv4s()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1, ip2, ip3;
    ip.set("192.168.1.1");
    ip1.set("192.168.1.2");
    ip2.set("192.168.1.3");
    ip3.set("192.168.1.4");

    set<HMIPAddress> vip, vip1, vip_ret,vip_diff;
    vip.insert(ip);
    vip.insert(ip1);

    vip1.insert(ip2);
    vip1.insert(ip3);
    hmdns.updateDNSEntry(hostname, false, vip);
    hmdns.finishQuery(hostname, false, true);
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY, vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_ret.size());
    set_intersection(vip.begin(), vip.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_diff.size());
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, false));

    hmdns.updateDNSEntry(hostname, false, vip1);
    hmdns.finishQuery(hostname, false, true);
    hmdns.getExpiredAddresses(hostname, HM_DUALSTACK_IPV4_ONLY, vip_ret);
    set_intersection(vip.begin(), vip.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_diff.size());
}

/*
 * This test create entry into DNS Cache. We update the results with multiple IPV6 address and
 * check the get addresses function of the DNS Cache.
 */
void TESTNAME::test_update_results_ipv6s()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1;
    ip.set("2001:0db8::0370:7334");
    ip1.set("2001:0db8::0370:7335");
    set<HMIPAddress> vip, vip_ret, vip_diff;
    vip.insert(ip);
    vip.insert(ip1);
    hmdns.updateDNSEntry(hostname, true, vip);
    hmdns.finishQuery(hostname, true, true);
    CPPUNIT_ASSERT_EQUAL(false,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY, vip_ret));
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY, vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_ret.size());
    set_intersection(vip.begin(), vip.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_diff.size());

    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, true));

}

void TESTNAME::test_get_expired_ipv6s()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1, ip2, ip3;
    ip.set("2001:0db8::0370:7334");
    ip1.set("2001:0db8::0370:7335");
    ip2.set("2001:0db8::0370:7336");
    ip3.set("2001:0db8::0370:7337");

    set<HMIPAddress> vip, vip1, vip_ret,vip_diff;
    vip.insert(ip);
    vip.insert(ip1);

    vip1.insert(ip2);
    vip1.insert(ip3);
    hmdns.updateDNSEntry(hostname, true, vip);
    hmdns.finishQuery(hostname, true, true);

    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY, vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_ret.size());
    set_intersection(vip.begin(), vip.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_diff.size());
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, false));

    hmdns.updateDNSEntry(hostname, true, vip1);
    hmdns.finishQuery(hostname, true, true);
    hmdns.getExpiredAddresses(hostname, HM_DUALSTACK_IPV6_ONLY, vip_ret);
    set_intersection(vip.begin(), vip.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip.size(), vip_diff.size());
}


void TESTNAME::test_get_expired_ipv4_prefered()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1, ip2, ip3;
    ip.set("192.168.1.1");
    ip1.set("2001:0db8::0370:7335");
    ip2.set( "192.168.1.2");
    ip3.set("2001:0db8::0370:7337");

    set<HMIPAddress> vip, vip1, vip2, vip3, vip_ret;
    vip.insert(ip);
    vip1.insert(ip1);

    hmdns.updateDNSEntry(hostname, false, vip);
    hmdns.finishQuery(hostname, false, true);
    hmdns.updateDNSEntry(hostname, true, vip1);
    hmdns.finishQuery(hostname, true, true);

    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT_EQUAL(2, (int)vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());
    CPPUNIT_ASSERT(vip_ret.find(ip1) != vip_ret.end());

    vip2.insert(ip2);
    vip3.insert(ip3);


    hmdns.updateDNSEntry(hostname, false, vip2);
    hmdns.finishQuery(hostname, false, true);
    hmdns.updateDNSEntry(hostname, true, vip3);
    hmdns.finishQuery(hostname, true, true);
    CPPUNIT_ASSERT_EQUAL(true,
                hmdns.getExpiredAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT_EQUAL(2, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());
    CPPUNIT_ASSERT(vip_ret.find(ip1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT_EQUAL(2, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip2) != vip_ret.end());
    CPPUNIT_ASSERT(vip_ret.find(ip3) != vip_ret.end());

}


void TESTNAME::test_get_expired_ipv6_prefered()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1, ip2, ip3;
    ip.set( "192.168.1.1");
    ip1.set("2001:0db8::0370:7335");
    ip2.set( "192.168.1.2");
    ip3.set("2001:0db8::0370:7337");

    set<HMIPAddress> vip, vip1, vip2, vip3, vip_ret;
    vip.insert(ip);
    vip1.insert(ip1);

    hmdns.updateDNSEntry(hostname, false, vip);
    hmdns.finishQuery(hostname, false, true);
    hmdns.updateDNSEntry(hostname, true, vip1);
    hmdns.finishQuery(hostname, true, true);

    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT_EQUAL(2, (int)vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());
    CPPUNIT_ASSERT(vip_ret.find(ip1) != vip_ret.end());

    vip2.insert(ip2);
    vip3.insert(ip3);


    hmdns.updateDNSEntry(hostname, false, vip2);
    hmdns.finishQuery(hostname, false, true);
    hmdns.updateDNSEntry(hostname, true, vip3);
    hmdns.finishQuery(hostname, true, true);
    CPPUNIT_ASSERT_EQUAL(true,
                hmdns.getExpiredAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT_EQUAL(2, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());
    CPPUNIT_ASSERT(vip_ret.find(ip1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT_EQUAL(2, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip2) != vip_ret.end());
    CPPUNIT_ASSERT(vip_ret.find(ip3) != vip_ret.end());



}




/*
 * This test create entry into DNS Cache. We update the results with both IPV4 and IPV6 address and
 * check the get addresses function of the DNS Cache to get IPv4 and IPv6 respectively.
 */
void TESTNAME::test_update_results_dualstack()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1;
    ip.set("2001:0db8::0370:7334");
    ip1.set("192.168.1.1");
    set<HMIPAddress> vip_ipv4, vip_ipv6, vip_ret, vip_diff;
    vip_ipv6.insert(ip);
    hmdns.updateDNSEntry(hostname, true, vip_ipv6);
    hmdns.finishQuery(hostname, true, true);
    vip_ipv4.insert(ip1);
    hmdns.updateDNSEntry(hostname, false, vip_ipv4);
    hmdns.finishQuery(hostname, false, true);
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip_ipv4.size(), vip_ret.size());
    set_intersection(vip_ipv4.begin(), vip_ipv4.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip_ipv4.size(), vip_diff.size());
    vip_diff.clear();
    vip_ret.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip_ipv6.size(), vip_ret.size());
    set_intersection(vip_ipv6.begin(), vip_ipv6.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip_ipv6.size(), vip_diff.size());

    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, true));
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, false));
}


void TESTNAME::test_update_results_failed_ipv4()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1;
    ip.set("1234::5679");
    ip1.set("0.0.0.0");
    set<HMIPAddress> vip_ipv4, vip_ipv6, vip_ret, vip_diff;
    vip_ipv6.insert(ip);
    hmdns.updateDNSEntry(hostname, true, vip_ipv6);
    hmdns.finishQuery(hostname, true, true);
    vip_ipv4.insert(ip1);
    hmdns.updateDNSEntry(hostname, false, vip_ipv4);
    hmdns.finishQuery(hostname, false, true);
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(1, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(HMIPAddress(AF_INET)) != vip_ret.end());

    vip_ret.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(1, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());

    vip_ret.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH , vip_ret));
    CPPUNIT_ASSERT_EQUAL(2, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());
    CPPUNIT_ASSERT(vip_ret.find(ip1) != vip_ret.end());
}


void TESTNAME::test_update_results_failed_ipv6()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1;
    ip.set("::");
    ip1.set("192.168.1.1");
    set<HMIPAddress> vip_ipv4, vip_ipv6, vip_ret, vip_diff;
    vip_ipv6.insert(ip);
    hmdns.updateDNSEntry(hostname, true, vip_ipv6);
    hmdns.finishQuery(hostname, true, true);
    vip_ipv4.insert(ip1);
    hmdns.updateDNSEntry(hostname, false, vip_ipv4);
    hmdns.finishQuery(hostname, false, true);
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(1, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip1) != vip_ret.end());

    vip_diff.clear();
    vip_ret.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(1, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(HMIPAddress(AF_INET6)) != vip_ret.end());

    vip_diff.clear();
    vip_ret.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH , vip_ret));
    CPPUNIT_ASSERT_EQUAL(2, (int )vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(ip1) != vip_ret.end());
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());
}


void TESTNAME::test_update_results_failed_dualstack()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip1;
    ip.set("0.0.0.0");
    ip1.set("0.0.0.0");
    set<HMIPAddress> vip_ipv4, vip_ipv6, vip_ret, vip_diff;
    vip_ipv6.insert(ip);
    hmdns.updateDNSEntry(hostname, true, vip_ipv6);
    hmdns.finishQuery(hostname, true, true);
    vip_ipv4.insert(ip1);
    hmdns.updateDNSEntry(hostname, false, vip_ipv4);
    hmdns.finishQuery(hostname, false, true);
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(1, (int)vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(HMIPAddress(AF_INET)) != vip_ret.end());

    vip_diff.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(1, (int)vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(HMIPAddress(AF_INET)) != vip_ret.end());

    vip_diff.clear();
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH , vip_ret));
    CPPUNIT_ASSERT_EQUAL(1, (int)vip_ret.size());
    CPPUNIT_ASSERT(vip_ret.find(HMIPAddress(AF_INET)) != vip_ret.end());
}

/*
 * This test create entry into DNS Cache. We update the results with IPV4 address and
 * check the get addresses function of the DNS Cache to get IPv6 Prefer option to test dual stack.
 */
void TESTNAME::test_update_results_dualstack_ipv4()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip;
    ip.set("192.168.1.1");
    set<HMIPAddress> vip_ipv4, vip_ret, vip_diff;
    vip_ipv4.insert(ip);
    hmdns.updateDNSEntry(hostname, false, vip_ipv4);
    hmdns.finishQuery(hostname, false, true);
    CPPUNIT_ASSERT_EQUAL(false,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH , vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip_ipv4.size(), vip_ret.size());
    set_intersection(vip_ipv4.begin(), vip_ipv4.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip_ipv4.size(), vip_diff.size());

}

/*
 * This test create entry into DNS Cache. We update the results with IPV6 address and
 * check the get addresses function of the DNS Cache to get IPv4 Prefer option to test dual stack.
 */
void TESTNAME::test_update_results_dualstack_ipv6()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip;
    ip.set("2001:0db8::0370:7334");
    set<HMIPAddress> vip_ipv6, vip_ret, vip_diff;
    vip_ipv6.insert(ip);
    hmdns.updateDNSEntry(hostname, true, vip_ipv6);
    hmdns.finishQuery(hostname, true, true);
    CPPUNIT_ASSERT_EQUAL(false,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip_ipv6.size(), vip_ret.size());
    set_intersection(vip_ipv6.begin(), vip_ipv6.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip_ipv6.size(), vip_diff.size());

}

/*
 * This test create entry into DNS Cache. We update the results with failure
 * and test the query needed function and ip address
 */

void TESTNAME::test_update_results_failure()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip;
    sockaddr_in6 addr;
    inet_pton(AF_INET6, "2001:0db8::0370:7334", &addr.sin6_addr);
    ip.set((char*) &addr, AF_INET6);
    set<HMIPAddress> vip_ipv6, vip_ret, vip_diff;
    vip_ipv6.insert(ip);
    hmdns.updateDNSEntry(hostname, true, vip_ipv6);
    hmdns.finishQuery(hostname, true, false);
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
                hmdns.queryNeeded(hostname, true));
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT_EQUAL(vip_ipv6.size(), vip_ret.size());
    set_intersection(vip_ipv6.begin(), vip_ipv6.end(), vip_ret.begin(), vip_ret.end(),
            std::inserter(vip_diff, vip_diff.begin()));
    CPPUNIT_ASSERT_EQUAL(vip_ipv6.size(), vip_diff.size());

    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            hmdns.queryNeeded(hostname, true));
}

/*
 * This test create entry into DNS Cache. We update the results with IPV4 address and
 * check the get address function of the DNS Cache to get IPv6 Prefer option to test dual stack.
 */
void TESTNAME::test_update_results_dualstack_v4()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip_ret;
    sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr("192.168.1.1");
    ip.set((char*) &addr, AF_INET);
    set<HMIPAddress> vip_ipv4, vip_ret;
    vip_ipv4.insert(ip);
    hmdns.updateDNSEntry(hostname, false, vip_ipv4);
    hmdns.finishQuery(hostname, false, true);
    CPPUNIT_ASSERT_EQUAL(false,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH , vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(ip) != vip_ret.end());
}

/*
 * This test create entry into DNS Cache. We update the results with IPV6 address and
 * check the get address function of the DNS Cache to get IPv4 Prefer option to test dual stack.
 */

void TESTNAME::test_update_results_dualstack_v6()
{
    HMDNSCache hmdns;
    string hostname = "Dummy.hm.com";
    HMIPAddress ip, ip_ret;
    sockaddr_in6 addr;
    inet_pton(AF_INET6, "2001:0db8::0370:7334", &addr.sin6_addr);
    ip.set((char*) &addr, AF_INET6);
    set<HMIPAddress> vip_ipv6, vip_ret;
    vip_ipv6.insert(ip);
    hmdns.updateDNSEntry(hostname, true, vip_ipv6);
    hmdns.finishQuery(hostname, true, true);
    CPPUNIT_ASSERT_EQUAL(false,
            hmdns.getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY , vip_ret));
    CPPUNIT_ASSERT_EQUAL(true,
            hmdns.getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT(vip_ipv6.find(ip) != vip_ret.end());
}
