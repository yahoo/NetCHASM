// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMDataCheckListAres.h"
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

void TESTNAME::test_basic_healthPlugins_dnsvc()
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
	hostGroup.setCheckType(HM_CHECK_DNSVC);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_DNS_ARES);
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
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_DNSVC,
            (uint8_t )work->m_hostCheck.getCheckType());
    check_list.getAllChecks(allChecks);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_CHECK_DNSVC,
            (uint8_t )allChecks[0].m_hostCheck.getCheckType());

    //neg cases
	hostGroup.setCheckType(HM_CHECK_DNSVC);
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

