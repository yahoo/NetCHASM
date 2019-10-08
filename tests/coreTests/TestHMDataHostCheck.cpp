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
    string remote_check = "remote.hm.com";
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getPort());
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getCheckType());
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getCheckPlugin());
    CPPUNIT_ASSERT_EQUAL((int )HM_DUALSTACK_IPV4_ONLY,
            (int )data_host.getDualStack());
    CPPUNIT_ASSERT(data_host.getCheckInfo().empty());
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT_EQUAL(53, (int )data_host.getPort());
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_HTTPS,
            (int )data_host.getCheckType());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_HTTP_CURL, (int )data_host.getCheckPlugin());
    CPPUNIT_ASSERT_EQUAL((int )HM_DUALSTACK_BOTH,
            (int )data_host.getDualStack());
    string test = data_host.getCheckInfo();
    CPPUNIT_ASSERT(!data_host.getCheckInfo().compare(check_info));
    CPPUNIT_ASSERT(!data_host.getRemoteCheck().compare(remote_check));
}

void TESTNAME::test_operations_equal()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "remote.hm.com";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(!(data_host != data_host1));
}

void TESTNAME::test_operations_nequal_checktype()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_operations_nequal_checkinfo()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
    string check_info1 = "DummyCheckInfo1";
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info1);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_operations_nequal_checkport()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(54);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_operations_nequal_dualstack()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_operations_equal_checkplugin()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_DNS_ARES);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(!(data_host != data_host1));
}

void TESTNAME::test_operations_nequal_remote_check()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "remote.hm.com";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
    string remote_check1 = "remote1.hm.com";
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_DNS_ARES);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check1);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host != data_host1));
}

void TESTNAME::test_print_entry()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: https:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
	hostGroup.setCheckType(HM_CHECK_NONE);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', false).compare(
                    "none:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: http:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));

	hostGroup.setCheckType(HM_CHECK_TCP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: tcp:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));

	hostGroup.setCheckType(HM_CHECK_FTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: ftp:Check Info: DummyCheckInfo:Port: 53:Dual Stack: ipv4-only"));

	hostGroup.setCheckType(HM_CHECK_DNS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_IPV6_ONLY);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: dns:Check Info: DummyCheckInfo:Port: 53:Dual Stack: ipv6-only"));
	hostGroup.setCheckType(HM_CHECK_DNSVC);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: dnsvc:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
	hostGroup.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: https-no-peer-check:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
	hostGroup.setCheckType(HM_CHECK_FTPS_EXPLICIT);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_FTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: ftps-explicit:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
	hostGroup.setCheckType(HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_FTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: ftps-explicit-no-peer-check:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
	hostGroup.setCheckType(HM_CHECK_AUX_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: aux http:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: aux https:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(
            !data_host.printEntry(':', true).compare(
                    "Check Type: aux https-no-peer-check:Check Info: DummyCheckInfo:Port: 53:Dual Stack: both"));

}


void TESTNAME::test_operations_less_checktype()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host < data_host1));
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}


void TESTNAME::test_operations_less_port()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(52);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host < data_host1));
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_DEFAULT);
	hostGroup.setPort(52);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}

void TESTNAME::test_operations_less_dualstack()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host < data_host1));
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_DEFAULT);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}

void TESTNAME::test_operations_less_checkinfo()
{
    HMDataHostCheck data_host;
    string check_info1 = "DummyCheckInfo1";
    string check_info2 = "DummyCheckInfo2";
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info1);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info2);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host < data_host1));
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info1);
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}

void TESTNAME::test_operations_less_remote_check()
{
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo1";
    string remote_check1 = "remoteRotation1";
    string remote_check2 = "remoteRotation2";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check1);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host.setCheckParams(hostGroup);
    HMDataHostCheck data_host1;
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check2);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT((data_host < data_host1));
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(53);
	hostGroup.setDualStack(HM_DUALSTACK_BOTH);
	hostGroup.setCheckInfo(check_info);
	hostGroup.setRemoteCheck(remote_check1);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	data_host1.setCheckParams(hostGroup);
    CPPUNIT_ASSERT(!(data_host < data_host1));
}

void
TESTNAME::test_parseCheckInfo_1()
{
    HMDataHostCheck check;
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//<host:port>/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:80/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:80/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:80");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:180/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:80/test");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS_NO_PEER_CHECK);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:80");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:80");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:380/HHHHHH");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:443");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_AUX_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(443);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx:294/sr");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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
    string remote_check = "";
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTPS);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(80);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("//xxx");
	hostGroup.setRemoteCheck(remote_check);
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	check.setCheckParams(hostGroup);

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

void TESTNAME::test_basic_data_host_check2()
{   
    HMDataHostCheck data_host;
    string check_info = "DummyCheckInfo";
    string remote_check = "remote.hm.com";
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getPort());
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getCheckType());
    CPPUNIT_ASSERT_EQUAL(0, (int )data_host.getCheckPlugin());
    CPPUNIT_ASSERT_EQUAL((int )HM_DUALSTACK_IPV4_ONLY,
            (int )data_host.getDualStack());
    CPPUNIT_ASSERT(data_host.getCheckInfo().empty());
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
        hostGroup.setCheckType(HM_CHECK_MARK_HTTPS);
        hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_MARK_CURL);
        hostGroup.setPort(53);
        hostGroup.setDualStack(HM_DUALSTACK_BOTH);
        hostGroup.setCheckInfo(check_info);
        hostGroup.setRemoteCheck(remote_check);
        hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
        hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
        data_host.setCheckParams(hostGroup);
    CPPUNIT_ASSERT_EQUAL(53, (int )data_host.getPort());
    CPPUNIT_ASSERT_EQUAL((int )HM_CHECK_MARK_HTTPS,
            (int )data_host.getCheckType());
    CPPUNIT_ASSERT_EQUAL((int)HM_CHECK_PLUGIN_MARK_CURL, (int )data_host.getCheckPlugin());
    CPPUNIT_ASSERT_EQUAL((int )HM_DUALSTACK_BOTH,
            (int )data_host.getDualStack());
    string test = data_host.getCheckInfo();
    CPPUNIT_ASSERT(!data_host.getCheckInfo().compare(check_info));
    CPPUNIT_ASSERT(!data_host.getRemoteCheck().compare(remote_check));
}

