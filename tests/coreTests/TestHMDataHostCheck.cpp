// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMDataHostCheck.h"

#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMWorkDNSLookupAres.h"
#include "common.h"
#include <unistd.h>

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

void TESTNAME::test_basic_data_host_check()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getPort());
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getCheckType());
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getCheckPlugin());
    CPPUNIT_ASSERT_EQUAL((int )HM_DUALSTACK_IPV4_ONLY,
            (int )data_host.getDualStack());
    CPPUNIT_ASSERT(data_host.getCheckInfo().empty());
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT_EQUAL(53, (int )data_host.getPort());
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_HTTPS,
            (int )data_host.getCheckType());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_HTTP_CURL, (int )data_host.getCheckPlugin());
    CPPUNIT_ASSERT_EQUAL((int )HM_DUALSTACK_BOTH,
            (int )data_host.getDualStack());
    string test = data_host.getCheckInfo();
    CPPUNIT_ASSERT(!data_host.getCheckInfo().compare(check_info));
}

void TESTNAME::test_operations_equal()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(!(data_host != data_host1));
}

void TESTNAME::test_operations_nequal_checktype()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_operations_nequal_checkinfo()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    HMDataHostCheck data_host1;
    string check_info1 = "DummyCheckInfo1";
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info1);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_operations_nequal_checkport()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 54,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_operations_nequal_dualstack()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_IPV4_ONLY, check_info);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_operations_equal_checkplugin()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_DNS_ARES, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(!(data_host != data_host1));
}

void TESTNAME::test_print_entry()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: https:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
    data_host.setCheckParams(HM_CHECK_NONE, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', false).compare(
                    "none:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
    data_host.setCheckParams(HM_CHECK_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: http:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));

    data_host.setCheckParams(HM_CHECK_TCP, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: tcp:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));

    data_host.setCheckParams(HM_CHECK_FTP, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_IPV4_ONLY, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: ftp:Check Info: DummyCheckInfo:Port: 53:Dual Stack: ipv4-only"));

    data_host.setCheckParams(HM_CHECK_DNS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_IPV6_ONLY, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: dns:Check Info: DummyCheckInfo:Port: 53:Dual Stack: ipv6-only"));
    data_host.setCheckParams(HM_CHECK_DNSVC, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: dnsvc:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
    data_host.setCheckParams(HM_CHECK_HTTPS_NO_PEER_CHECK, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: https-no-peer-check:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
    data_host.setCheckParams(HM_CHECK_FTPS_EXPLICIT, HM_CHECK_PLUGIN_FTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: ftps-explicit:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
    data_host.setCheckParams(HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK, HM_CHECK_PLUGIN_FTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: ftps-explicit-no-peer-check:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
    data_host.setCheckParams(HM_CHECK_AUX_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: aux http:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
    data_host.setCheckParams(HM_CHECK_AUX_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: aux https:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
    data_host.setCheckParams(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: aux https-no-peer-check:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));

}


void TESTNAME::test_operations_less_checktype()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT((data_host < data_host1));
    data_host1.setCheckParams(HM_CHECK_HTTP, HM_CHECK_PLUGIN_HTTP_CURL, 53,
        HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}


void TESTNAME::test_operations_less_port()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 52,
    HM_DUALSTACK_BOTH, check_info);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT((data_host < data_host1));
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_DEFAULT, 52,
        HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}

void TESTNAME::test_operations_less_dualstack()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
            HM_DUALSTACK_IPV4_ONLY, check_info);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info);
    CPPUNIT_ASSERT((data_host < data_host1));
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_DEFAULT, 53,
            HM_DUALSTACK_IPV4_ONLY, check_info);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}

void TESTNAME::test_operations_less_checkinfo()
{
    HMDataHostCheck data_host;
    string check_info1 = "DummyCheckInfo1";
    string check_info2 = "DummyCheckInfo2";
    data_host.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info1);
    HMDataHostCheck data_host1;
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
    HM_DUALSTACK_BOTH, check_info2);
    CPPUNIT_ASSERT((data_host < data_host1));
    data_host1.setCheckParams(HM_CHECK_HTTPS, HM_CHECK_PLUGIN_HTTP_CURL, 53,
        HM_DUALSTACK_BOTH, check_info1);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}

void
TESTNAME::test_parseCheckInfo_1()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT(checkInfoHost.empty());
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_2()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT(checkInfoHost.empty());
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_3()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT(checkInfoHost.empty());
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_4()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_5()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_6()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_7()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_8()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_9()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_10()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_11()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_12()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_13()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_14()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_15()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_16()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_17()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_18()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_19()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_20()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:443";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_21()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:443";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_22()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_23()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_24()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_25()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:443";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT(checkInfoHost.empty());
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_26()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:443";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT(checkInfoHost.empty());
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_27()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:443";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT(checkInfoHost.empty());
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_28()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//<host:port>/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: host1:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT(checkInfoHost.empty());
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_29()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:80/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:80";
    string checkhost = "xxx:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_30()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:80/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:80";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_31()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:80");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:80";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_32()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:180/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:180";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)180);
}

void
TESTNAME::test_parseCheckInfo_33()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:80/test");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:80";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_34()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:80");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:80";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_35()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTP,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:80");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:80";
    string checkhost = "xxx:80";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

void
TESTNAME::test_parseCheckInfo_36()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:380/HHHHHH");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:380";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)380);
}

void
TESTNAME::test_parseCheckInfo_37()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:443");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:443";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_38()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_AUX_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            443,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)443);
}

void
TESTNAME::test_parseCheckInfo_39()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx:294/sr");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:294";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)294);
}

void
TESTNAME::test_parseCheckInfo_40()
{
    HMDataHostCheck check;
    check.setCheckParams(HM_CHECK_HTTPS,
            HM_CHECK_PLUGIN_HTTP_CURL,
            80,
            HM_DUALSTACK_IPV4_ONLY,
            "//xxx");

    const string host = "host1";
    uint32_t port;
    string checkInfoHost;
    string hostname = "Host: xxx:80";
    string checkhost = "xxx";

    CPPUNIT_ASSERT(checkInfoHost.empty());
    string result = check.parseCheckInfo(host, port, checkInfoHost);
    CPPUNIT_ASSERT_EQUAL(result, hostname);
    CPPUNIT_ASSERT_EQUAL(checkInfoHost, checkhost);
    CPPUNIT_ASSERT_EQUAL(port, (uint32_t)80);
}

