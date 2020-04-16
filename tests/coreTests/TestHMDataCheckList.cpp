// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMDataCheckList.h"
#include <arpa/inet.h>
#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMStorage.h"
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

void TESTNAME::test_basic_datachecklist()
{
    HMDataHostCheck data_host;
    HMDataCheckParams params;
    HMDataCheckList check_list;
    HMIPAddress ip, ip_res;
    HMDNSCache cache;
    HMWorkQueue queue;
    HMWaitList waitlist;
    string host_name = "Host1";
    string host_group = "HostGroup1";
    string check_info = "DummyCheckInfo";
    ip.set("2001:0db8::0370:7334");
    set<HMIPAddress> ips, vip_ret;
    ips.insert(ip);
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.initDNSCache(cache, waitlist, HM_DNS_PLUGIN_ARES, HM_DNS_PLUGIN_STATIC);
    HMDNSLookup dnsHostCheckT(HM_DNS_TYPE_STATIC, true);
    cache.getAddresses(host_name, HM_DUALSTACK_BOTH, dnsHostCheckT, vip_ret);
    check_list.checkNeeded(host_name, ip, data_host);
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            check_list.checkNeeded(host_name, ip, data_host));
    CPPUNIT_ASSERT(
            !check_list.printChecks(false).compare(
                    "Host1\nCheck Type: https\tCheck Info: DummyCheckInfo\tPort: 53\tDual Stack: both\n\n"));
    CPPUNIT_ASSERT(
            !check_list.printChecks(true).compare(
                    "Host1\nCheck Type: https\tCheck Info: DummyCheckInfo\tPort: 53\tDual Stack: both\nCheck Timeout: 10000\nCheck TTL:30000\nNumber Check Retries: 0\nCheck Retry Delay: 0\nMeasurement Options: connect\nSmoothing Window: 10\nGroup Threshold: 20\nSlow Threshold: 20\nMax Flaps: 4\n\n\n"));

    //neg cases HM_CHECK_AUX_HTTP
	hostGroup.setCheckType(HM_CHECK_AUX_HTTP);
	hostGroup.setCheckPlugin((HM_CHECK_PLUGIN_CLASS)10);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )0, queue.queueSize());    
}


void TESTNAME::test_ip_dns_failed()
{
    HMDataHostCheck data_host;
    HMDataCheckParams params;
    HMDataCheckList check_list;
    HMIPAddress ip(AF_INET6), ip_res;
    set<HMIPAddress> ips, vip_ret;
    ips.insert(ip);
    HMDNSCache cache;
    HMWorkQueue queue;
    HMWaitList waitlist;
    string host_name = "Host1";
    string host_group = "HostGroup1";
    string check_info = "DummyCheckInfo";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.initDNSCache(cache, waitlist, HM_DNS_PLUGIN_ARES, HM_DNS_PLUGIN_STATIC);
    HMDNSLookup dnsHostCheckT(HM_DNS_TYPE_STATIC, true);
    cache.getAddresses(host_name, HM_DUALSTACK_BOTH, dnsHostCheckT, vip_ret);
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
    check_list.checkNeeded(host_name, ip, data_host));
}

void TESTNAME::test_basic_healthPlugins_tcp()
{
    HMDataHostCheck data_host;
    HMDataCheckParams params;
    HMDataCheckList check_list;
    HMIPAddress ip;
    set<HMIPAddress> ips;
    ips.insert(ip);
    HMWorkQueue queue;
    HMDataCheckResult result;
    vector<HMCheckHeader> allChecks;
    unique_ptr<HMWork> work;
    string host_name = "Host1";
    string host_group = "HostGroup1";
    string check_info = "DummyCheckInfo";
    bool threadStatus = false;
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_TCP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_TCP_RAW);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, queue.queueSize());
    CPPUNIT_ASSERT_EQUAL(true, queue.getWork(work, threadStatus));
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_TCP,
            (uint8_t )work->m_hostCheck.getCheckType());
    check_list.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_TCP,
            (uint8_t )allChecks[0].m_hostCheck.getCheckType());
    CPPUNIT_ASSERT(check_list.getCheckResult(allChecks[0], result));

    //neg cases
	hostGroup.setCheckType(HM_CHECK_TCP);
	hostGroup.setCheckPlugin((HM_CHECK_PLUGIN_CLASS)10);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )0, queue.queueSize());    
}

void TESTNAME::test_basic_healthPlugins_ftp()
{
    HMDataHostCheck data_host;
    HMDataCheckParams params;
    HMDataCheckList check_list;
    HMIPAddress ip;
    set<HMIPAddress> ips;
    ips.insert(ip);
    HMWorkQueue queue;
    vector<HMCheckHeader> allChecks;
    unique_ptr<HMWork> work;
    string host_name = "Host1";
    string host_group = "HostGroup1";
    string check_info = "DummyCheckInfo";
    bool threadStatus = false;
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_FTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_FTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_IPV6_ONLY);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, queue.queueSize());
    CPPUNIT_ASSERT_EQUAL(true, queue.getWork(work, threadStatus));
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )work->m_hostCheck.getCheckType());
    check_list.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )allChecks[0].m_hostCheck.getCheckType());
    //neg cases
	hostGroup.setCheckType(HM_CHECK_FTP);
	hostGroup.setCheckPlugin((HM_CHECK_PLUGIN_CLASS)10);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )0, queue.queueSize());
}

void TESTNAME::test_basic_healthPlugins_none()
{
    HMDataHostCheck data_host;
    HMDataCheckParams params;
    HMDataCheckList check_list;
    HMIPAddress ip;
    set<HMIPAddress> ips;
    ips.insert(ip);
    HMWorkQueue queue;
    vector<HMCheckHeader> allChecks;
    unique_ptr<HMWork> work;
    string host_name = "Host1";
    string host_group = "HostGroup1";
    string check_info = "DummyCheckInfo";
    bool threadStatus = false;
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_NONE);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_DEFAULT);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, queue.queueSize());
    CPPUNIT_ASSERT_EQUAL(true, queue.getWork(work, threadStatus));
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_NONE,
            (uint8_t )work->m_hostCheck.getCheckType());
    check_list.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_NONE,
            (uint8_t )allChecks[0].m_hostCheck.getCheckType());

    //neg cases
	hostGroup.setCheckType(HM_CHECK_NONE);
	hostGroup.setCheckPlugin((HM_CHECK_PLUGIN_CLASS)10);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )0, queue.queueSize());
}

void TESTNAME::test_basic_healthPlugins_dns()
{
    HMDataHostCheck data_host;
    HMDataCheckParams params;
    HMDataCheckList check_list;
    HMIPAddress ip;
    set<HMIPAddress> ips;
    ips.insert(ip);
    HMWorkQueue queue;
    vector<HMCheckHeader> allChecks;
    unique_ptr<HMWork> work;
    string host_name = "Host1";
    string host_group = "HostGroup1";
    string check_info = "DummyCheckInfo";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_DEFAULT);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_DEFAULT);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, queue.queueSize());
}

void TESTNAME::test_add_hostgroup()
{
    HMDataHostCheck data_host;
    HMDataCheckParams params;
    HMDataCheckList check_list,neg_check_list;
    string host1 = "test1";
    string host2 = "test2";
    HMDataHostGroup group1("Group1");
    HMDataHostGroup group2("Group2");
    CPPUNIT_ASSERT_EQUAL((uint32_t )0, check_list.addHostGroup(group1));
    group1.addHost(host1);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, check_list.addHostGroup(group1));
    group1.addHost(host2);
    CPPUNIT_ASSERT_EQUAL((uint32_t )2, check_list.addHostGroup(group1));
    //neg test
    group2.setCheckType((HM_CHECK_TYPE)(-1));
    group2.addHost(host1);
    CPPUNIT_ASSERT_EQUAL((uint32_t )0, neg_check_list.addHostGroup(group2));
    CPPUNIT_ASSERT_EQUAL((group1 < group2), true);
    CPPUNIT_ASSERT_EQUAL((group2 < group2), false);
}

void TESTNAME::test_basic_healthPlugins_mtls()
{
    HMDataHostCheck data_host;
    HMDataCheckParams params;
    HMDataCheckList check_list;
    HMIPAddress ip;
    set<HMIPAddress> ips;
    ips.insert(ip);
    HMWorkQueue queue;
    HMDataCheckResult result;
    vector<HMCheckHeader> allChecks;
    unique_ptr<HMWork> work;
    string host_name = "Host1";
    string host_group = "HostGroup1";
    string check_info = "DummyCheckInfo";
    bool threadStatus = false;
    HMDataHostGroup hostGroup(host_group);
    hostGroup.setCheckType(HM_CHECK_MTLS_HTTPS);
    hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
    hostGroup.setPort(53);
    hostGroup.setDualStack(HM_DUALSTACK_BOTH);
    hostGroup.setCheckInfo(check_info);
    hostGroup.setRemoteCheck("");
    hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
    hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, queue.queueSize());
    CPPUNIT_ASSERT_EQUAL(true, queue.getWork(work, threadStatus));
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_MTLS_HTTPS,
            (uint8_t )work->m_hostCheck.getCheckType());
    check_list.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_MTLS_HTTPS,
            (uint8_t )allChecks[0].m_hostCheck.getCheckType());
    CPPUNIT_ASSERT(check_list.getCheckResult(allChecks[0], result));

}

void TESTNAME::test_hostgroup_distributed_fallback()
{
    HMDataHostGroup group1("Group1");
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_NONE);
    group1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_LOCAL);
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_LOCAL);
    group1.unsetDistributedFallback(HM_DISTRIBUTED_FALLBACK_LOCAL);
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_NONE);
    group1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_REMOTE);
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_REMOTE);
    group1.unsetDistributedFallback(HM_DISTRIBUTED_FALLBACK_REMOTE);
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_NONE);
    group1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_LOCAL);
    group1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_REMOTE);
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_BOTH);
    group1.unsetDistributedFallback(HM_DISTRIBUTED_FALLBACK_REMOTE);
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_LOCAL);
    group1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_REMOTE);
    group1.unsetDistributedFallback(HM_DISTRIBUTED_FALLBACK_LOCAL);
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_REMOTE);
    group1.unsetDistributedFallback(HM_DISTRIBUTED_FALLBACK_REMOTE);
    CPPUNIT_ASSERT_EQUAL(group1.getDistributedFallback(), HM_DISTRIBUTED_FALLBACK_NONE);
}

void TESTNAME::test_basic_healthPlugins_checkIpAddress()
{
    HMDataHostCheck data_host;
    HMDataHostCheck data_host1;
    HMDataCheckParams params;
    HMDataCheckParams params1;
    HMDataCheckList check_list;
    HMIPAddress ip, ip1, ip2, ip3, ip4, ip5;
    string host_name = "Host";
    string host_name1 = "Host1";
    string host_group = "HostGroup";
    string host_group1 = "HostGroup1";
    string check_info = "DummyCheckInfo";

    HMDataHostGroup hostGroup(host_group);
    hostGroup.setCheckType(HM_CHECK_DEFAULT);
    hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_DEFAULT);
    hostGroup.setPort(53);
    hostGroup.setDualStack(HM_DUALSTACK_BOTH);
    hostGroup.setCheckInfo(check_info);
    hostGroup.setRemoteCheck("");
    hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
    hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    data_host.setCheckParams(hostGroup);

    HMDataHostGroup hostGroup1(host_group1);
    hostGroup1.setCheckType(HM_CHECK_DNS);
    hostGroup1.setCheckPlugin(HM_CHECK_PLUGIN_DEFAULT);
    hostGroup1.setPort(53);
    hostGroup1.setDualStack(HM_DUALSTACK_BOTH);
    hostGroup1.setCheckInfo(check_info);
    hostGroup1.setRemoteCheck("");
    hostGroup1.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
    hostGroup1.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
    data_host1.setCheckParams(hostGroup1);

    ip.set("192.168.1.1");
    set<HMIPAddress> ips;
    ips.insert(ip);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    ips.clear();
    ip1.set("::1");
    ips.insert(ip1);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    ips.clear();
    ip2.set("::2");
    ips.insert(ip2);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    ips.clear();
    ip3.set("192.168.1.2");
    ips.clear();
    ips.insert(ip3);
    check_list.insertCheck(host_group, host_name1, data_host1, params1, ips);
    ips.clear();
    ip4.set("192.168.1.3");
    ips.insert(ip4);
    check_list.insertCheck(host_group, host_name1, data_host1, params1, ips);
    ips.clear();
    ip5.set("::3");
    ips.insert(ip5);
    check_list.insertCheck(host_group, host_name1, data_host1, params1, ips);

    check_list.getCheckResultsAddress(host_name, data_host, HM_DUALSTACK_IPV4_ONLY, ips);
    CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
    CPPUNIT_ASSERT(ips.find(ip) != ips.end());

    check_list.getCheckResultsAddress(host_name, data_host, HM_DUALSTACK_IPV6_ONLY, ips);
    CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
    CPPUNIT_ASSERT(ips.find(ip1) != ips.end());
    CPPUNIT_ASSERT(ips.find(ip2) != ips.end());

    check_list.getCheckResultsAddress(host_name, data_host, HM_DUALSTACK_BOTH, ips);
    CPPUNIT_ASSERT_EQUAL(3, (int)ips.size());
    CPPUNIT_ASSERT(ips.find(ip) != ips.end());
    CPPUNIT_ASSERT(ips.find(ip1) != ips.end());
    CPPUNIT_ASSERT(ips.find(ip2) != ips.end());

}
