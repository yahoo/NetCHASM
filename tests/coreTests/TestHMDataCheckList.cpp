// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMDataCheckList.h"
#include <arpa/inet.h>
#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMStorage.h"
#include "HMWorkHealthCheckRemote.h"
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
    check_list.initDNSCache(cache, waitlist);
    HMDNSLookup dnsHostCheckT(HM_DNS_PLUGIN_STATIC, true);
    cache.getAddresses(host_name, HM_DUALSTACK_BOTH, dnsHostCheckT, vip_ret);
    check_list.checkNeeded(host_name, ip, data_host);
    cout<<check_list.printChecks(true);
    CPPUNIT_ASSERT_EQUAL(HM_SCHEDULE_IGNORE,
            check_list.checkNeeded(host_name, ip, data_host));
    cout<<check_list.printChecks(false)<<endl;
    CPPUNIT_ASSERT(
            !check_list.printChecks(false).compare(
                    "Host1\nCheck Type: https\tCheck Info: DummyCheckInfo\tPort: 53\tDual Stack: both\n\n"));
    cout<<check_list.printChecks(true).length()<<endl;
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
    check_list.initDNSCache(cache, waitlist);
    HMDNSLookup dnsHostCheckT(HM_DNS_PLUGIN_STATIC, true);
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

void TESTNAME::test_basic_healthPlugins_remoteDisabled()
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
	hostGroup.setCheckType(HM_CHECK_FTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_FTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_IPV6_ONLY);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("RemoteRotation");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_TCP);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue, false);
    CPPUNIT_ASSERT_EQUAL((uint32_t )2, queue.queueSize());
    CPPUNIT_ASSERT_EQUAL(true, queue.getWork(work, threadStatus));
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )work->m_hostCheck.getCheckType());
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_PLUGIN_FTP_CURL,
            (uint8_t )work->m_hostCheck.getCheckPlugin());
    CPPUNIT_ASSERT_EQUAL(true, queue.getWork(work, threadStatus));
    HMWorkHealthCheckRemote* test = dynamic_cast<HMWorkHealthCheckRemote*>(work.get());
    CPPUNIT_ASSERT(!test);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )work->m_hostCheck.getCheckType());
    check_list.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )allChecks[0].m_hostCheck.getCheckType());
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )allChecks[1].m_hostCheck.getCheckType());

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


void TESTNAME::test_basic_healthPlugins_remoteEnabled()
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
    check_list.queueCheck(host_name, ip, data_host, queue, true);
	hostGroup.setCheckType(HM_CHECK_FTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_FTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_IPV6_ONLY);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck("RemoteRotation");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_TCP);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    check_list.insertCheck(host_group, host_name, data_host, params, ips);
    check_list.startCheck(host_name, ip, data_host);
    check_list.queueCheck(host_name, ip, data_host, queue);
    CPPUNIT_ASSERT_EQUAL((uint32_t )2, queue.queueSize());
    CPPUNIT_ASSERT_EQUAL(true, queue.getWork(work, threadStatus));
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )work->m_hostCheck.getCheckType());
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_PLUGIN_FTP_CURL,
            (uint8_t )work->m_hostCheck.getCheckPlugin());
    CPPUNIT_ASSERT_EQUAL(true, queue.getWork(work, threadStatus));
    HMWorkHealthCheckRemote* test = dynamic_cast<HMWorkHealthCheckRemote*>(work.get());
    CPPUNIT_ASSERT(test);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )work->m_hostCheck.getCheckType());
    check_list.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
            (uint8_t )allChecks[0].m_hostCheck.getCheckType());
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_FTP,
                (uint8_t )allChecks[1].m_hostCheck.getCheckType());

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
