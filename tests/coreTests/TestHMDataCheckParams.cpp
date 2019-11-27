// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMDataCheckParams.h"

#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMConstants.h"
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

void TESTNAME::test_basic_datacheckparams()
{
    HMDataCheckParams params;
    vector<std::string> hostGroups;
    CPPUNIT_ASSERT_EQUAL(30000, (int )params.getTTL());
    CPPUNIT_ASSERT_EQUAL(10000, (int )params.getTimeout());
    params.setCheckParameters(0, 0, 0, 0, 0, 0, 0, 6000, 3000, HM_DEFAULT_FLAP_THRESHOLD, 0);
    CPPUNIT_ASSERT_EQUAL(3000, (int )params.getTTL());
    CPPUNIT_ASSERT_EQUAL(6000, (int )params.getTimeout());
    params.addHostGroup("Group1");
    params.addHostGroup("Group2");
    params.getHostGroups(hostGroups);
    CPPUNIT_ASSERT_EQUAL(2, (int )hostGroups.size());
    CPPUNIT_ASSERT(!hostGroups[0].compare("Group1"));
    CPPUNIT_ASSERT(!hostGroups[1].compare("Group2"));
    CPPUNIT_ASSERT(!params.printHostGroups().compare("Group1,Group2"));
}

void TESTNAME::test_basic_query()
{
    HMDataCheckParams params;
    HMIPAddress ip;
    params.emptyQuery(ip);
    params.startQuery(ip);
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_IN_PROGRESS, params.getQueryState(ip));
    CPPUNIT_ASSERT(params.getCheckTime(ip) <= HMTimeStamp::now());
    CPPUNIT_ASSERT(!params.checkNeeded(ip));
    CPPUNIT_ASSERT(params.nextCheckTime(ip) > HMTimeStamp::now());
}

void TESTNAME::test_basic_result()
{
    HMDataCheckParams params;
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);
    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED, HM_REASON_DNS_NOTFOUND,
            start, end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, params.getQueryState(ip));
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_DNS_NOTFOUND, (uint8_t )result.m_reason);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, result.m_numFailures);
    CPPUNIT_ASSERT(
            !params.printEntry().compare(
                    "Check Timeout: 10000\nCheck TTL:30000\nNumber Check Retries: 0\nCheck Retry Delay: 0\nMeasurement Options: connect\nSmoothing Window: 10\nGroup Threshold: 20\nSlow Threshold: 20\nMax Flaps: 4\n"));
}


void TESTNAME::test_dns_failed_result()
{
    HMDataCheckParams params;
    HMIPAddress ip;
    ip.set("0.0.0.0");
    HMIPAddress ip1;
    ip1.set("192.168.1.0");
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = start;
    params.emptyQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_DNS_FAILED, HM_REASON_DNS_NOTFOUND,
            start, end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(true, params.getCheckResult(ip, result));
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_RESPONSE_DNS_FAILED, (uint8_t )result.m_response);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_DNS_NOTFOUND, (uint8_t )result.m_reason);
    CPPUNIT_ASSERT(start == result.m_start);
    CPPUNIT_ASSERT(start == result.m_checkTime);
    CPPUNIT_ASSERT(start == result.m_end);
    CPPUNIT_ASSERT(!result.m_address.toString().compare("0.0.0.0"));
    CPPUNIT_ASSERT_EQUAL((uint32_t )0, result.m_numChecks);
    CPPUNIT_ASSERT(
            !params.printEntry().compare(
                    "Check Timeout: 10000\nCheck TTL:30000\nNumber Check Retries: 0\nCheck Retry Delay: 0\nMeasurement Options: connect\nSmoothing Window: 10\nGroup Threshold: 20\nSlow Threshold: 20\nMax Flaps: 4\n"));
}


void TESTNAME::test_basic_result1()
{
    HMDataCheckParams params;
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED, HM_REASON_CONNECT_TIMEOUT,
            start, end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, params.getQueryState(ip));
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_CONNECT_TIMEOUT,
            (uint8_t )result.m_reason);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, result.m_numTimeouts);
}

void TESTNAME::test_basic_result2()
{
    HMDataCheckParams params;
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED, HM_REASON_RESPONSE_FAILURE,
            start, end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, params.getQueryState(ip));
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_RESPONSE_FAILURE,
            (uint8_t )result.m_reason);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, result.m_numFailures);
}

void TESTNAME::test_basic_result3()
{
    HMDataCheckParams params;
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED, HM_REASON_CONNECT_FAILURE,
            start, end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, params.getQueryState(ip));
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_CONNECT_FAILURE,
            (uint8_t )result.m_reason);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, result.m_numConnectFailures);
}

void TESTNAME::test_basic_result4()
{

    HMDataCheckParams params;
    params.setCheckParameters(0, 0, HM_RT_CONNECT,
    HM_DEFAULT_SMOOTHING_WINDOW, HM_DEFAULT_GROUP_THRESHOLD,
    HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS, 10000, 0, HM_DEFAULT_FLAP_THRESHOLD, 0);
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED, HM_REASON_SUCCESS, start,
            end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, params.getQueryState(ip));
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_SUCCESS, (uint8_t )result.m_reason);
    CPPUNIT_ASSERT_EQUAL((uint32_t )(end - start), result.m_responseTime);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, result.m_numResponses);
    CPPUNIT_ASSERT_EQUAL((uint32_t )(end - start),
            result.m_smoothedResponseTime);
    CPPUNIT_ASSERT(
            (uint32_t )(HMTimeStamp::now() - start)
                    >= result.m_totalResponseTime);
}

void TESTNAME::test_basic_result5()
{

    HMDataCheckParams params;
    params.setCheckParameters(0, 0, HM_RT_TOTAL,
    HM_DEFAULT_SMOOTHING_WINDOW, HM_DEFAULT_GROUP_THRESHOLD,
    HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS, 10000, 0, HM_DEFAULT_FLAP_THRESHOLD, 0);
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED, HM_REASON_SUCCESS, start,
            end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, params.getQueryState(ip));
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_SUCCESS, (uint8_t )result.m_reason);
    CPPUNIT_ASSERT_EQUAL((uint32_t )(end - start), result.m_responseTime);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, result.m_numResponses);
    CPPUNIT_ASSERT(
            (uint32_t )(HMTimeStamp::now() - start)
                    >= result.m_smoothedResponseTime);
    CPPUNIT_ASSERT(
            (uint32_t )(HMTimeStamp::now() - start)
                    >= result.m_totalResponseTime);
}


void TESTNAME::test_basic_result6()
{
    HMDataCheckParams params;
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test" );
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED, HM_REASON_INTERNAL_ERROR,
            start, end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, params.getQueryState(ip));
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_INTERNAL_ERROR, (uint8_t )result.m_reason);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, result.m_numConnectFailures);
}


void TESTNAME::test_basic_retry()
{
    HMDataCheckParams params;
    params.setCheckParameters(3, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
            HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS,
            10000, 0, HM_DEFAULT_FLAP_THRESHOLD, 0);
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
            HM_REASON_SUCCESS, start, end, dataHost.getPort());
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(result.m_status & HM_HOST_STATUS_UP);
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL(2, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(2, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(result.m_status & HM_HOST_STATUS_UP);
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL(3, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(3, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(result.m_status & HM_HOST_STATUS_UP);
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint16_t )801, (uint16_t )result.m_port);
    CPPUNIT_ASSERT_EQUAL(4, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(0, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(!(result.m_status & HM_HOST_STATUS_UP));
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));

}


void TESTNAME::test_basic_flaps()
{
    HMDataCheckParams params;
    params.setCheckParameters(3, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
            HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS,
            10000, 0, 1000, 0);
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
            HM_REASON_SUCCESS, start, end, dataHost.getPort());
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
                HM_REASON_SUCCESS, start, end, dataHost.getPort());
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
                HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(3, (int )result.m_numFlaps);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
            HM_REASON_SUCCESS, start, end, dataHost.getPort());
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(5, (int )result.m_numFlaps);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
            HM_REASON_SUCCESS, start, end, dataHost.getPort());
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(7, (int )result.m_numFlaps);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
            HM_REASON_SUCCESS, start +1001, end+1001, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(0, (int )result.m_numFlaps);

}

void TESTNAME::test_nexttime_retry()
{
    //Tests next check time is after check retry delay
    HMDataCheckParams params;
    HMIPAddress ip;
    HMDataCheckResult result;
    // both prev is up and current status is down should return check_time+ retry_ttl

    result.m_numFailedChecks = 1;
    result.m_status = HM_HOST_STATUS_UP;
    result.m_softStatus = HM_HOST_STATUS_NONE;

    result.m_queryState = HM_CHECK_INACTIVE;
    result.m_checkTime = HMTimeStamp::now();

    params.setCheckParameters(3, 100, 0, 0, 0, 0, 0, 0, 900000, 0, 0);

    params.updateCheck(ip, result, true);
    HMTimeStamp nexttime = HMTimeStamp::now();
    HMTimeStamp ret = params.nextCheckTime(ip);
    CPPUNIT_ASSERT( ret > nexttime);
    CPPUNIT_ASSERT( ret <= (nexttime + 100));

    // both prev and current status is up but m_numSlowResponses > 0 should return check_time+ retry_ttl
    result.m_numSlowResponses = 1;
    result.m_status = HM_HOST_STATUS_UP;
    result.m_softStatus = HM_HOST_STATUS_UP;
    params.updateCheck(ip, result, true);
    nexttime = HMTimeStamp::now();
    ret = params.nextCheckTime(ip);
    CPPUNIT_ASSERT( ret > nexttime);
    CPPUNIT_ASSERT( ret <= (nexttime + 100));

    // both prev and current status is up should return check_time+ check_ttl
    result.m_numSlowResponses = 0;
    result.m_status = HM_HOST_STATUS_UP;
    result.m_softStatus = HM_HOST_STATUS_UP;
    params.updateCheck(ip, result, true);
    nexttime = HMTimeStamp::now();
    ret = params.nextCheckTime(ip);
    CPPUNIT_ASSERT(ret > nexttime + 100);
    CPPUNIT_ASSERT(ret <= (nexttime + 900000));

}

void TESTNAME::test_nexttime_ttl()
{

    //Tests next check time is after ttl
    HMDataCheckParams params;
    HMIPAddress ip;
    HMDataCheckResult result , result1;

    //   numFailedChecks reached max retries
    result.m_numFailedChecks = 4;
    result.m_status = HM_HOST_STATUS_UP;
    result.m_softStatus = HM_HOST_STATUS_NONE;
    result.m_queryState = HM_CHECK_INACTIVE;
    result.m_checkTime = HMTimeStamp::now();

    params.setCheckParameters(3, 10000, 0, 0, 0, 0, 0, 0, 50000, 0, 0);

    params.updateCheck(ip, result, true);
    HMTimeStamp nexttime = HMTimeStamp::now();
    HMTimeStamp ret = params.nextCheckTime(ip);
    CPPUNIT_ASSERT( ret > (nexttime + 20000));
    CPPUNIT_ASSERT( ret <= (nexttime + 50000));

    //  no failed check
    result1.m_numFailedChecks = 0;
    result1.m_status = HM_HOST_STATUS_UP ;
    result1.m_softStatus = result1.m_status;
    result1.m_queryState = HM_CHECK_INACTIVE;
    result1.m_checkTime = HMTimeStamp::now();

    params.setCheckParameters(3, 10000, 0, 0, 0, 0, 0, 0, 500000, 0, 0);

    params.updateCheck(ip, result1, true);
    nexttime = HMTimeStamp::now();
    ret = params.nextCheckTime(ip);
    CPPUNIT_ASSERT( ret > nexttime + 30000);
    CPPUNIT_ASSERT( ret <= nexttime + 500000);
}

void TESTNAME::test_nexttime_immediate()
{

    //   needs a immediate check
    HMDataCheckParams params;
    HMIPAddress ip;
    HMDataCheckResult result;

    result.m_numFailedChecks = 2;
    result.m_status = HM_HOST_STATUS_UP;
    result.m_status = HM_HOST_STATUS_NONE;
    result.m_queryState = HM_CHECK_INACTIVE;
    result.m_checkTime = HMTimeStamp::now()-50000;

    params.setCheckParameters(3, 100, 0, 0, 0, 0, 0, 0, 10000, 0, 0);

    params.updateCheck(ip, result, true);
    HMTimeStamp nexttime = HMTimeStamp::now();
    HMTimeStamp ret = params.nextCheckTime(ip);
    CPPUNIT_ASSERT( ret <= (nexttime + 100));

}

void TESTNAME::test_response_200()
{

    HMDataCheckParams params;
    params.setCheckParameters(0, 0, HM_RT_TOTAL,
    HM_DEFAULT_SMOOTHING_WINDOW, HM_DEFAULT_GROUP_THRESHOLD,
    HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS, 10000, 0, HM_DEFAULT_FLAP_THRESHOLD, 0);
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 11000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED, HM_REASON_SUCCESS, start,
            end, dataHost.getPort());
    CPPUNIT_ASSERT_EQUAL(HM_CHECK_INACTIVE, params.getQueryState(ip));
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL((uint8_t )HM_REASON_SUCCESS, (uint8_t)result.m_reason);
    CPPUNIT_ASSERT_EQUAL((uint32_t )(end - start), result.m_responseTime);
    CPPUNIT_ASSERT_EQUAL((uint32_t )1, result.m_numResponses);
    CPPUNIT_ASSERT(
            (uint32_t )(10000)
                    >= result.m_smoothedResponseTime);
    CPPUNIT_ASSERT(
            (uint32_t )(10000)
                    >= result.m_totalResponseTime);
}

void TESTNAME::test_retry_200()
{

    HMDataCheckParams params;
    params.setCheckParameters(3, 0, 0, HM_DEFAULT_SMOOTHING_WINDOW,
            HM_DEFAULT_GROUP_THRESHOLD, HM_DEFAULT_SLOW_THRESHOLD, HM_DEFAULT_MAX_FLAPS,
            2000, 0, HM_DEFAULT_FLAP_THRESHOLD, 0);
    HMIPAddress ip;
    string host_name = "dummy.hm.com";
    HMDataCheckResult result;
    HMDataHostCheck dataHost;
    string host_group = "dummy";
    HMDataHostGroup hostGroup(host_group);
	hostGroup.setCheckType(HM_CHECK_HTTP);
	hostGroup.setCheckPlugin(HM_CHECK_PLUGIN_HTTP_CURL);
	hostGroup.setPort(801);
	hostGroup.setDualStack(HM_DUALSTACK_IPV4_ONLY);
	hostGroup.setCheckInfo("test");
	hostGroup.setRemoteCheck("");
	hostGroup.setRemoteCheckType(HM_REMOTE_CHECK_NONE);
	hostGroup.setDistributedFallback(HM_DISTRIBUTED_FALLBACK_NONE);
	dataHost.setCheckParams(hostGroup);

    HMTimeStamp start = HMTimeStamp::now();
    HMTimeStamp end = HMTimeStamp::now() + 1000;
    params.emptyQuery(ip);
    params.startQuery(ip);
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
            HM_REASON_SUCCESS, start, end, dataHost.getPort());
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
            HM_REASON_SUCCESS, start, end+1200, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFailures);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(result.m_status & HM_HOST_STATUS_UP);
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(2, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(result.m_status & HM_HOST_STATUS_UP);
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(2, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(3, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(result.m_status & HM_HOST_STATUS_UP);
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(3, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(0, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(!(result.m_status & HM_HOST_STATUS_UP));
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(4, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(0, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(!(result.m_status & HM_HOST_STATUS_UP));
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_FAILED,
            HM_REASON_CONNECT_FAILURE, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(5, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(0, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(1, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(!(result.m_status & HM_HOST_STATUS_UP));
    CPPUNIT_ASSERT(!(result.m_softStatus & HM_HOST_STATUS_UP));
    params.updateCheck(host_name, ip, HM_RESPONSE_CONNECTED,
            HM_REASON_SUCCESS, start, end, dataHost.getPort());
    params.getCheckResult(ip, result);
    CPPUNIT_ASSERT_EQUAL(5, (int )result.m_numConnectFailures);
    CPPUNIT_ASSERT_EQUAL(0, (int )result.m_numFailedChecks);
    CPPUNIT_ASSERT_EQUAL(2, (int )result.m_numFlaps);
    CPPUNIT_ASSERT(result.m_status & HM_HOST_STATUS_UP);
    CPPUNIT_ASSERT((result.m_softStatus & HM_HOST_STATUS_UP));
}

