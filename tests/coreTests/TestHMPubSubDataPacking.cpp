// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMPubSubDataPacking.h"

#include "HMDNSCache.h"
#include "HMStateManager.h"
#include "HMWorkDNSLookupAres.h"
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

void TESTNAME::test_protobuf_packing()
{
    vector<string> hostgroups = {"hostgroup1", "hostgroup2", "hostgroup3"};
    HMTimeStamp now = HMTimeStamp::now();
    HMIPAddress address;
    address.set("192.168.1.3");
    HMDataCheckResult checkResult;
    checkResult.m_checkTime = now;
    checkResult.m_response = HM_RESPONSE_CONNECTED;
    checkResult.m_reason = HM_REASON_SUCCESS;
    checkResult.m_responseTime = 33;
    checkResult.m_totalResponseTime = 66;
    checkResult.m_smoothedResponseTime = 44;
    checkResult.m_address = address;
    string hostname = "host1.nc.com";
    set<string> hostgroupset;
    for(const string& hg: hostgroups)
    {
        hostgroupset.insert(hg);
    }
    HMPubSubDataPacking datapacking;
    uint64_t buflen;
    int mark = 10;
    unique_ptr<char[]> data = datapacking.packPublishResults(hostname, mark, hostgroupset, checkResult, buflen);
    CPPUNIT_ASSERT(data);
    uint8_t pubversion;
    string rhostname;
    set<string> rhostgroupset;
    HMDataCheckResult rcheckResult;
    int rMark = 0;
    CPPUNIT_ASSERT(datapacking.unpackPublishResults(data, buflen, pubversion, rhostname, rMark, rhostgroupset, rcheckResult));
    CPPUNIT_ASSERT_EQUAL(hostname, rhostname);
    CPPUNIT_ASSERT_EQUAL(1, (int)pubversion);
    CPPUNIT_ASSERT_EQUAL(10, (int)rMark);
    CPPUNIT_ASSERT(checkResult == rcheckResult);
    for (const string& hg : hostgroups)
    {
        CPPUNIT_ASSERT(rhostgroupset.find(hg) != rhostgroupset.end());
    }
}

