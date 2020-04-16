// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "common.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fstream>

#include "TestHMState.h"
#include "HMDNSCache.h"
#include "HMState.h"
#include "HMConstants.h"
#include "TestStorageHostGroup.h"

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

void TESTNAME::test_allchecks()
{
    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";
    string hostGroup3 = "hostgroup3";
    string hostGroup4 = "hostgroup4";

    string hostname1_1 = "test1_1.hm.com";
    string hostname1_2 = "test1_2.hm.com";
    string hostname3_1 = "test3_1.hm.com";
    string hostname3_2 = "test3_2.hm.com";

    HMDataHostGroup dataHostGroup1(hostGroup1);
    dataHostGroup1.addHost(hostname1_1);
    dataHostGroup1.addHost(hostname1_2);
    HMDataHostGroup dataHostGroup2(hostGroup2);
    dataHostGroup2.addHost(hostname1_1);
    dataHostGroup2.addHost(hostname1_2);
    HMDataHostGroup dataHostGroup3(hostGroup3);
    dataHostGroup3.addHost(hostname3_1);
    dataHostGroup3.addHost(hostname3_2);
    HMDataHostGroup dataHostGroup4(hostGroup4);
    dataHostGroup4.addHost(hostname3_1);
    dataHostGroup4.addHost(hostname3_2);
    shared_ptr<HMState> oldstate = make_shared<HMState>();
    CPPUNIT_ASSERT(oldstate);
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup2));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup3));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup4));
    vector<HMCheckHeader> allchecks;
    CPPUNIT_ASSERT(oldstate->m_checkList.getAllChecks(allchecks));
    CPPUNIT_ASSERT_EQUAL(4, (int)allchecks.size());
    CPPUNIT_ASSERT(allchecks[0].m_hostname == hostname1_1);
    CPPUNIT_ASSERT(allchecks[0].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[1].m_hostname == hostname1_2);
    CPPUNIT_ASSERT(allchecks[1].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[2].m_hostname == hostname3_1);
    CPPUNIT_ASSERT(allchecks[2].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[3].m_hostname == hostname3_2);
    CPPUNIT_ASSERT(allchecks[3].m_address == HMIPAddress());
}


void TESTNAME::test_allchecks1()
{
    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";
    string hostGroup3 = "hostgroup3";
    string hostGroup4 = "hostgroup4";

    string hostname1_1 = "test1_1.hm.com";
    string hostname1_2 = "test1_2.hm.com";
    string hostname3_1 = "test3_1.hm.com";
    string hostname3_2 = "test3_2.hm.com";

    HMDataHostCheck dataHostCheck1;
    HMDataHostCheck dataHostCheck2;
    HMDataHostCheck dataHostCheck3;
    HMDataHostCheck dataHostCheck4;
    HMDataCheckParams dataCheckParams1;
    HMDataCheckParams dataCheckParams2;
    HMDataCheckParams dataCheckParams3;
    HMDataCheckParams dataCheckParams4;

    HMDataHostGroup dataHostGroup1(hostGroup1);
    dataHostGroup1.addHost(hostname1_1);
    dataHostGroup1.addHost(hostname1_2);
    dataHostGroup1.getHostCheck(dataHostCheck1);
    dataHostGroup1.getCheckParameters(dataCheckParams1);
    HMDataHostGroup dataHostGroup2(hostGroup2);
    dataHostGroup2.setCheckType(HM_CHECK_HTTP);
    dataHostGroup2.addHost(hostname1_1);
    dataHostGroup2.addHost(hostname1_2);
    dataHostGroup2.getHostCheck(dataHostCheck2);
    dataHostGroup2.getCheckParameters(dataCheckParams2);
    HMDataHostGroup dataHostGroup3(hostGroup3);
    dataHostGroup3.addHost(hostname3_1);
    dataHostGroup3.addHost(hostname3_2);
    dataHostGroup3.getHostCheck(dataHostCheck3);
    dataHostGroup3.getCheckParameters(dataCheckParams3);
    HMDataHostGroup dataHostGroup4(hostGroup4);
    dataHostGroup4.setCheckTTL(10000);
    dataHostGroup4.addHost(hostname3_1);
    dataHostGroup4.addHost(hostname3_2);
    dataHostGroup4.getHostCheck(dataHostCheck4);
    dataHostGroup4.getCheckParameters(dataCheckParams4);


    HMIPAddress address1_1_1;
    HMIPAddress address1_1_2;
    HMIPAddress address1_2_1;
    HMIPAddress address1_2_2;
    HMIPAddress address3_1_1;
    HMIPAddress address3_1_2;
    HMIPAddress address3_2_1;
    HMIPAddress address3_2_2;

    address1_1_1.set("192.168.0.1");
    address1_1_2.set("fad0::1");
    address1_2_1.set("192.168.0.2");
    address1_2_2.set("fad0::2");

    address3_1_1.set("192.168.0.5");
    address3_1_2.set("fad0::5");
    address3_2_1.set("192.168.0.6");
    address3_2_2.set("fad0::6");

    shared_ptr<HMState> oldstate = make_shared<HMState>();
    CPPUNIT_ASSERT(oldstate);
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup2));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup3));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup4));

    vector<HMCheckHeader> allchecks;
    CPPUNIT_ASSERT(oldstate->m_checkList.getAllChecks(allchecks));
    CPPUNIT_ASSERT_EQUAL(8, (int)allchecks.size());
    CPPUNIT_ASSERT(allchecks[0].m_hostname == hostname1_1);
    CPPUNIT_ASSERT(allchecks[0].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[0].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[0].m_checkParams == dataCheckParams1);

    CPPUNIT_ASSERT(allchecks[1].m_hostname == hostname1_1);
    CPPUNIT_ASSERT(allchecks[1].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[1].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[1].m_checkParams == dataCheckParams2);

    CPPUNIT_ASSERT(allchecks[2].m_hostname == hostname1_2);
    CPPUNIT_ASSERT(allchecks[2].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[2].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[2].m_checkParams == dataCheckParams1);

    CPPUNIT_ASSERT(allchecks[3].m_hostname == hostname1_2);
    CPPUNIT_ASSERT(allchecks[3].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[3].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[3].m_checkParams == dataCheckParams2);

    CPPUNIT_ASSERT(allchecks[4].m_hostname == hostname3_1);
    CPPUNIT_ASSERT(allchecks[4].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[4].m_hostCheck == dataHostCheck3);
    CPPUNIT_ASSERT(allchecks[4].m_checkParams == dataCheckParams3);

    CPPUNIT_ASSERT(allchecks[5].m_hostname == hostname3_1);
    CPPUNIT_ASSERT(allchecks[5].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[5].m_hostCheck == dataHostCheck4);
    CPPUNIT_ASSERT(allchecks[5].m_checkParams == dataCheckParams4);

    CPPUNIT_ASSERT(allchecks[6].m_hostname == hostname3_2);
    CPPUNIT_ASSERT(allchecks[6].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[6].m_hostCheck == dataHostCheck3);
    CPPUNIT_ASSERT(allchecks[6].m_checkParams == dataCheckParams3);

    CPPUNIT_ASSERT(allchecks[7].m_hostname == hostname3_2);
    CPPUNIT_ASSERT(allchecks[7].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[7].m_hostCheck == dataHostCheck4);
    CPPUNIT_ASSERT(allchecks[7].m_checkParams == dataCheckParams4);


}

void TESTNAME::test_allcheckResults()
{
    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";

    string hostname1 = "test1.hm.com";
    string hostname2 = "test2.hm.com";

    HMDataHostCheck dataHostCheck1;
    HMDataHostCheck dataHostCheck2;
    HMDataCheckParams dataCheckParams1;
    HMDataCheckParams dataCheckParams2;

    HMDataHostGroup dataHostGroup1(hostGroup1);
    dataHostGroup1.addHost(hostname1);
    dataHostGroup1.addHost(hostname2);
    dataHostGroup1.getHostCheck(dataHostCheck1);
    dataHostGroup1.getCheckParameters(dataCheckParams1);
    HMDataHostGroup dataHostGroup2(hostGroup2);
    dataHostGroup2.addHost(hostname1);
    dataHostGroup2.addHost(hostname2);
    dataHostGroup2.getHostCheck(dataHostCheck2);
    dataHostGroup2.getCheckParameters(dataCheckParams2);


    HMIPAddress address1_1;
    HMIPAddress address1_2;
    HMIPAddress address2_1;
    HMIPAddress address2_2;

    address1_1.set("192.168.0.1");
    address1_2.set("fad0::1");
    address2_1.set("192.168.0.2");
    address2_2.set("fad0::2");

    shared_ptr<HMState> oldstate = make_shared<HMState>();
    CPPUNIT_ASSERT(oldstate);
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup2));
    HMDNSLookup lookup4(HM_DNS_TYPE_LOOKUP, false);
    HMDNSLookup lookup6(HM_DNS_TYPE_LOOKUP, true);
    {
        set<HMIPAddress> ips;
        ips.insert(address1_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address1_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup6, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup6, ips);
    }


    HMDataCheckResult result1_1;
    result1_1.m_address = address1_1;
    result1_1.m_reason = HM_REASON_DNS_TIMEOUT;
    HMDataCheckResult result1_2;
    result1_2.m_address = address1_2;
    result1_2.m_reason = HM_REASON_CONNECT_TIMEOUT;
    HMDataCheckResult result2_1;
    result2_1.m_address = address2_1;
    result2_1.m_reason = HM_REASON_CONNECT_FAILURE;
    HMDataCheckResult result2_2;
    result2_2.m_address = address2_2;
    result2_2.m_reason = HM_REASON_RESPONSE_3XX;


    vector<HMCheckHeader> allchecks;
    CPPUNIT_ASSERT(oldstate->m_checkList.getAllChecks(allchecks));
    CPPUNIT_ASSERT_EQUAL(2, (int)allchecks.size());
    CPPUNIT_ASSERT(allchecks[0].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[0].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[0].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[0].m_checkParams == dataCheckParams1);
    CPPUNIT_ASSERT(allchecks[1].m_hostname == hostname2);
    CPPUNIT_ASSERT(allchecks[1].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[1].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[1].m_checkParams == dataCheckParams1);

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_1);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_2);
    }

    {
        HMCheckHeader header(hostname2, address2_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_1);
    }

    {
        HMCheckHeader header(hostname2, address2_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_2);
    }
    shared_ptr<HMState> newstate = make_shared<HMState>();
    CPPUNIT_ASSERT(newstate);
    newstate->m_datastore = make_unique<TestStorageHostGroup>(&newstate->m_hostGroups, &newstate->m_dnsCache);
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(dataHostGroup2));

    newstate->restoreRunningCheckState(oldstate);
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
    }
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, dataHostCheck1, dataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, dataHostCheck1, dataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_1, dataHostCheck1, dataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_2, dataHostCheck1, dataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_2);
    }

    // hostgroupresults
    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup1, results));
        CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2);
        CPPUNIT_ASSERT(results[2].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[2].m_address == address2_1);
        CPPUNIT_ASSERT(results[2].m_result == result2_1);
        CPPUNIT_ASSERT(results[3].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[3].m_address == address2_2);
        CPPUNIT_ASSERT(results[3].m_result == result2_2);
    }
    // hostgroupresults
    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup2, results));
        CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2);
        CPPUNIT_ASSERT(results[2].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[2].m_address == address2_1);
        CPPUNIT_ASSERT(results[2].m_result == result2_1);
        CPPUNIT_ASSERT(results[3].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[3].m_address == address2_2);
        CPPUNIT_ASSERT(results[3].m_result == result2_2);
    }
}

void TESTNAME::test_allcheckResults1()
{
    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";

    string hostname1 = "test1.hm.com";
    string hostname2 = "test2.hm.com";

    HMDataHostCheck dataHostCheck1;
    HMDataHostCheck dataHostCheck2;
    HMDataCheckParams dataCheckParams1;
    HMDataCheckParams dataCheckParams2;

    HMDataHostGroup dataHostGroup1(hostGroup1);
    dataHostGroup1.addHost(hostname1);
    dataHostGroup1.addHost(hostname2);
    dataHostGroup1.getHostCheck(dataHostCheck1);
    dataHostGroup1.getCheckParameters(dataCheckParams1);
    HMDataHostGroup dataHostGroup2(hostGroup2);
    dataHostGroup2.addHost(hostname1);
    dataHostGroup2.addHost(hostname2);
    dataHostGroup2.setCheckTTL(10000);
    dataHostGroup2.getHostCheck(dataHostCheck2);
    dataHostGroup2.getCheckParameters(dataCheckParams2);


    HMIPAddress address1_1;
    HMIPAddress address1_2;
    HMIPAddress address2_1;
    HMIPAddress address2_2;

    address1_1.set("192.168.0.1");
    address1_2.set("fad0::1");
    address2_1.set("192.168.0.2");
    address2_2.set("fad0::2");

    shared_ptr<HMState> oldstate = make_shared<HMState>();
    CPPUNIT_ASSERT(oldstate);
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup2));
    HMDNSLookup lookup4(HM_DNS_TYPE_LOOKUP, false);
    HMDNSLookup lookup6(HM_DNS_TYPE_LOOKUP, true);
    {
        set<HMIPAddress> ips;
        ips.insert(address1_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address1_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup6, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup6, ips);
    }


    HMDataCheckResult result1_1;
    result1_1.m_address = address1_1;
    result1_1.m_reason = HM_REASON_DNS_TIMEOUT;
    HMDataCheckResult result1_2;
    result1_2.m_address = address1_2;
    result1_2.m_reason = HM_REASON_CONNECT_TIMEOUT;
    HMDataCheckResult result2_1;
    result2_1.m_address = address2_1;
    result2_1.m_reason = HM_REASON_CONNECT_FAILURE;
    HMDataCheckResult result2_2;
    result2_2.m_address = address2_2;
    result2_2.m_reason = HM_REASON_RESPONSE_3XX;

    HMDataCheckResult result1_1_1;
    result1_1_1.m_address = address1_1;
    result1_1_1.m_reason = HM_REASON_RESPONSE_403;
    HMDataCheckResult result1_2_1;
    result1_2_1.m_address = address1_2;
    result1_2_1.m_reason = HM_REASON_RESPONSE_404;
    HMDataCheckResult result2_1_1;
    result2_1_1.m_address = address2_1;
    result2_1_1.m_reason = HM_REASON_RESPONSE_DOWN;
    HMDataCheckResult result2_2_1;
    result2_2_1.m_address = address2_2;
    result2_2_1.m_reason = HM_REASON_RESPONSE_5XX;


    vector<HMCheckHeader> allchecks;
    CPPUNIT_ASSERT(oldstate->m_checkList.getAllChecks(allchecks));
    CPPUNIT_ASSERT_EQUAL(4, (int)allchecks.size());
    CPPUNIT_ASSERT(allchecks[0].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[0].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[0].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[0].m_checkParams == dataCheckParams1);
    CPPUNIT_ASSERT(allchecks[1].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[1].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[1].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[1].m_checkParams == dataCheckParams2);
    CPPUNIT_ASSERT(allchecks[2].m_hostname == hostname2);
    CPPUNIT_ASSERT(allchecks[2].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[2].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[2].m_checkParams == dataCheckParams1);
    CPPUNIT_ASSERT(allchecks[3].m_hostname == hostname2);
    CPPUNIT_ASSERT(allchecks[3].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[3].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[3].m_checkParams == dataCheckParams2);

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_1);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_2);
    }

    {
        HMCheckHeader header(hostname2, address2_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_1);
    }

    {
        HMCheckHeader header(hostname2, address2_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_2);
    }

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result1_1_1);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result1_2_1);
    }

    {
        HMCheckHeader header(hostname2, address2_1, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result2_1_1);
    }

    {
        HMCheckHeader header(hostname2, address2_2, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result2_2_1);
    }

    shared_ptr<HMState> newstate = make_shared<HMState>();
    CPPUNIT_ASSERT(newstate);
    newstate->m_datastore = make_unique<TestStorageHostGroup>(&newstate->m_hostGroups, &newstate->m_dnsCache);
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(dataHostGroup2));

    newstate->restoreRunningCheckState(oldstate);
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
    }
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, dataHostCheck1, dataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, dataHostCheck1, dataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_1, dataHostCheck1, dataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_2, dataHostCheck1, dataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, dataHostCheck2, dataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, dataHostCheck2, dataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_1, dataHostCheck2, dataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_2, dataHostCheck2, dataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_2_1);
    }

    // hostgroupresults
    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup1, results));
        CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2);
        CPPUNIT_ASSERT(results[2].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[2].m_address == address2_1);
        CPPUNIT_ASSERT(results[2].m_result == result2_1);
        CPPUNIT_ASSERT(results[3].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[3].m_address == address2_2);
        CPPUNIT_ASSERT(results[3].m_result == result2_2);
    }
    // hostgroupresults
    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup2, results));
        CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2_1);
        CPPUNIT_ASSERT(results[2].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[2].m_address == address2_1);
        CPPUNIT_ASSERT(results[2].m_result == result2_1_1);
        CPPUNIT_ASSERT(results[3].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[3].m_address == address2_2);
        CPPUNIT_ASSERT(results[3].m_result == result2_2_1);
    }
}

void TESTNAME::test_allcheckResults2()
{
    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";

    string hostname1 = "test1.hm.com";
    string hostname2 = "test2.hm.com";

    HMDataHostCheck dataHostCheck1;
    HMDataHostCheck dataHostCheck2;
    HMDataCheckParams dataCheckParams1;
    HMDataCheckParams dataCheckParams2;

    HMDataHostGroup dataHostGroup1(hostGroup1);
    dataHostGroup1.addHost(hostname1);
    dataHostGroup1.addHost(hostname2);
    dataHostGroup1.getHostCheck(dataHostCheck1);
    dataHostGroup1.getCheckParameters(dataCheckParams1);
    HMDataHostGroup dataHostGroup2(hostGroup2);
    dataHostGroup2.addHost(hostname1);
    dataHostGroup2.addHost(hostname2);
    dataHostGroup2.setCheckTTL(10000);
    dataHostGroup2.getHostCheck(dataHostCheck2);
    dataHostGroup2.getCheckParameters(dataCheckParams2);


    HMIPAddress address1_1;
    HMIPAddress address1_2;
    HMIPAddress address2_1;
    HMIPAddress address2_2;

    address1_1.set("192.168.0.1");
    address1_2.set("fad0::1");
    address2_1.set("192.168.0.2");
    address2_2.set("fad0::2");

    shared_ptr<HMState> oldstate = make_shared<HMState>();
    CPPUNIT_ASSERT(oldstate);
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup2));
    HMDNSLookup lookup4(HM_DNS_TYPE_LOOKUP, false);
    HMDNSLookup lookup6(HM_DNS_TYPE_LOOKUP, true);
    {
        set<HMIPAddress> ips;
        ips.insert(address1_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address1_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup6, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup6, ips);
    }


    HMDataCheckResult result1_1;
    result1_1.m_address = address1_1;
    result1_1.m_reason = HM_REASON_DNS_TIMEOUT;
    HMDataCheckResult result1_2;
    result1_2.m_address = address1_2;
    result1_2.m_reason = HM_REASON_CONNECT_TIMEOUT;
    HMDataCheckResult result2_1;
    result2_1.m_address = address2_1;
    result2_1.m_reason = HM_REASON_CONNECT_FAILURE;
    HMDataCheckResult result2_2;
    result2_2.m_address = address2_2;
    result2_2.m_reason = HM_REASON_RESPONSE_3XX;

    HMDataCheckResult result1_1_1;
    result1_1_1.m_address = address1_1;
    result1_1_1.m_reason = HM_REASON_RESPONSE_403;
    HMDataCheckResult result1_2_1;
    result1_2_1.m_address = address1_2;
    result1_2_1.m_reason = HM_REASON_RESPONSE_404;
    HMDataCheckResult result2_1_1;
    result2_1_1.m_address = address2_1;
    result2_1_1.m_reason = HM_REASON_RESPONSE_DOWN;
    HMDataCheckResult result2_2_1;
    result2_2_1.m_address = address2_2;
    result2_2_1.m_reason = HM_REASON_RESPONSE_5XX;


    vector<HMCheckHeader> allchecks;
    CPPUNIT_ASSERT(oldstate->m_checkList.getAllChecks(allchecks));
    CPPUNIT_ASSERT_EQUAL(4, (int)allchecks.size());
    CPPUNIT_ASSERT(allchecks[0].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[0].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[0].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[0].m_checkParams == dataCheckParams1);
    CPPUNIT_ASSERT(allchecks[1].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[1].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[1].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[1].m_checkParams == dataCheckParams2);
    CPPUNIT_ASSERT(allchecks[2].m_hostname == hostname2);
    CPPUNIT_ASSERT(allchecks[2].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[2].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[2].m_checkParams == dataCheckParams1);
    CPPUNIT_ASSERT(allchecks[3].m_hostname == hostname2);
    CPPUNIT_ASSERT(allchecks[3].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[3].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[3].m_checkParams == dataCheckParams2);

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_1);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_2);
    }

    {
        HMCheckHeader header(hostname2, address2_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_1);
    }

    {
        HMCheckHeader header(hostname2, address2_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_2);
    }

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result1_1_1);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result1_2_1);
    }

    {
        HMCheckHeader header(hostname2, address2_1, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result2_1_1);
    }

    {
        HMCheckHeader header(hostname2, address2_2, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result2_2_1);
    }

    // New state details
    string hostGroup3 = "hostgroup3";
    string hostGroup4 = "hostgroup4";

    HMDataHostCheck nDataHostCheck1;
    HMDataHostCheck nDataHostCheck2;
    HMDataHostCheck nDataHostCheck3;
    HMDataHostCheck nDataHostCheck4;
    HMDataCheckParams nDataCheckParams1;
    HMDataCheckParams nDataCheckParams2;
    HMDataCheckParams nDataCheckParams3;
    HMDataCheckParams nDataCheckParams4;

    HMDataHostGroup nDataHostGroup1(hostGroup1);
    nDataHostGroup1.addHost(hostname1);
    nDataHostGroup1.setCheckTTL(10000);
    nDataHostGroup1.getHostCheck(nDataHostCheck1);
    nDataHostGroup1.getCheckParameters(nDataCheckParams1);

    HMDataHostGroup nDataHostGroup2(hostGroup2);
    nDataHostGroup2.addHost(hostname1);
    nDataHostGroup2.addHost(hostname2);
    nDataHostGroup2.setCheckTTL(100000);
    nDataHostGroup2.getHostCheck(nDataHostCheck2);
    nDataHostGroup2.getCheckParameters(nDataCheckParams2);

    HMDataHostGroup nDataHostGroup3(hostGroup3);
    nDataHostGroup3.addHost(hostname2);
    nDataHostGroup3.setCheckTTL(10000);
    nDataHostGroup3.getHostCheck(nDataHostCheck3);
    nDataHostGroup3.getCheckParameters(nDataCheckParams3);

    HMDataHostGroup nDataHostGroup4(hostGroup4);
    nDataHostGroup4.addHost(hostname1);
    nDataHostGroup4.addHost(hostname2);
    nDataHostGroup4.getHostCheck(nDataHostCheck4);
    nDataHostGroup4.getCheckParameters(nDataCheckParams4);

    shared_ptr<HMState> newstate = make_shared<HMState>();
    CPPUNIT_ASSERT(newstate);
    newstate->m_datastore = make_unique<TestStorageHostGroup>(&newstate->m_hostGroups, &newstate->m_dnsCache);
    CPPUNIT_ASSERT_EQUAL(1, (int)newstate->m_checkList.addHostGroup(nDataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(nDataHostGroup2));
    CPPUNIT_ASSERT_EQUAL(1, (int)newstate->m_checkList.addHostGroup(nDataHostGroup3));
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(nDataHostGroup4));

    newstate->restoreRunningCheckState(oldstate);
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
    }
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, nDataHostCheck1, nDataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, nDataHostCheck1, nDataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_1, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_2, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_2_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, nDataHostCheck4, nDataCheckParams4);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, nDataHostCheck4, nDataCheckParams4);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_1, nDataHostCheck4, nDataCheckParams4);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_2, nDataHostCheck4, nDataCheckParams4);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_2);
    }

    // hostgroupresults
    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup1, results));
        CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2_1);
    }
    // hostgroupresults
    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup2, results));
        CPPUNIT_ASSERT_EQUAL(0, (int)results.size());
    }

    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup3, results));
        CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[0].m_address == address2_1);
        CPPUNIT_ASSERT(results[0].m_result == result2_1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[1].m_address == address2_2);
        CPPUNIT_ASSERT(results[1].m_result == result2_2_1);
    }

    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup4, results));
        CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2);
        CPPUNIT_ASSERT(results[2].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[2].m_address == address2_1);
        CPPUNIT_ASSERT(results[2].m_result == result2_1);
        CPPUNIT_ASSERT(results[3].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[3].m_address == address2_2);
        CPPUNIT_ASSERT(results[3].m_result == result2_2);
    }
}

void TESTNAME::test_allcheckResults3()
{
    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";
    string hostGroup3 = "hostgroup3";
    string hostGroup4 = "hostgroup4";
    string hostGroup5 = "hostgroup5";
    string hostGroup6 = "hostgroup6";

    string hostname1 = "test1.hm.com";
    string hostname2 = "test2.hm.com";
    string hostname3 = "test3.hm.com";
    string hostname4 = "test4.hm.com";

    HMDataHostCheck dataHostCheck1;
    HMDataHostCheck dataHostCheck2;
    HMDataHostCheck dataHostCheck3;
    HMDataHostCheck dataHostCheck4;
    HMDataHostCheck dataHostCheck5;
    HMDataHostCheck dataHostCheck6;
    HMDataCheckParams dataCheckParams1;
    HMDataCheckParams dataCheckParams2;
    HMDataCheckParams dataCheckParams3;
    HMDataCheckParams dataCheckParams4;
    HMDataCheckParams dataCheckParams5;
    HMDataCheckParams dataCheckParams6;

    HMDataHostGroup dataHostGroup1(hostGroup1);
    dataHostGroup1.addHost(hostname1);
    dataHostGroup1.getHostCheck(dataHostCheck1);
    dataHostGroup1.getCheckParameters(dataCheckParams1);

    HMDataHostGroup dataHostGroup2(hostGroup2);
    dataHostGroup2.addHost(hostname2);
    dataHostGroup2.getHostCheck(dataHostCheck2);
    dataHostGroup2.getCheckParameters(dataCheckParams2);

    HMDataHostGroup dataHostGroup3(hostGroup3);
    dataHostGroup3.addHost(hostname3);
    dataHostGroup3.setDualStack(HM_DUALSTACK_BOTH);
    dataHostGroup3.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    dataHostGroup3.getHostCheck(dataHostCheck3);
    dataHostGroup3.getCheckParameters(dataCheckParams3);

    HMDataHostGroup dataHostGroup4(hostGroup4);
    dataHostGroup4.addHost(hostname4);
    dataHostGroup4.setDualStack(HM_DUALSTACK_BOTH);
    dataHostGroup4.setFlowType(HM_FLOW_REMOTE_HOSTGROUP_TYPE);
    dataHostGroup4.getHostCheck(dataHostCheck4);
    dataHostGroup4.getCheckParameters(dataCheckParams4);

    HMDataHostGroup dataHostGroup5(hostGroup5);
    dataHostGroup5.addHost(hostname1);
    dataHostGroup5.setDualStack(HM_DUALSTACK_BOTH);
    dataHostGroup5.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    dataHostGroup5.getHostCheck(dataHostCheck5);
    dataHostGroup5.getCheckParameters(dataCheckParams5);

    HMDataHostGroup dataHostGroup6(hostGroup6);
    dataHostGroup6.addHost(hostname1);
    dataHostGroup6.setDualStack(HM_DUALSTACK_BOTH);
    dataHostGroup6.setFlowType(HM_FLOW_REMOTE_HOSTGROUP_TYPE);
    dataHostGroup6.getHostCheck(dataHostCheck6);
    dataHostGroup6.getCheckParameters(dataCheckParams6);

    HMIPAddress address1_1;
    HMIPAddress address1_2;
    HMIPAddress address2_1;
    HMIPAddress address2_2;
    HMIPAddress address3_1;
    HMIPAddress address3_2;
    HMIPAddress address4_1;
    HMIPAddress address4_2;

    address1_1.set("192.168.0.1");
    address1_2.set("fad0::1");
    address2_1.set("192.168.0.2");
    address2_2.set("fad0::2");
    address3_1.set("192.168.0.3");
    address3_2.set("fad0::3");
    address4_1.set("192.168.0.4");
    address4_2.set("fad0::4");

    shared_ptr<HMState> oldstate = make_shared<HMState>();
    CPPUNIT_ASSERT(oldstate);
    HMTimeStamp time;
    time.setTime(100);
    oldstate->m_remoteCache.insertRemoteEntry(hostGroup4, dataHostGroup4.getCheckTTL(), dataHostGroup4.getCheckTimeout());
    oldstate->m_remoteCache.updateResultTime(hostGroup4, time);
    time.setTime(200);
    oldstate->m_remoteCache.insertRemoteEntry(hostGroup6, dataHostGroup6.getCheckTTL(), dataHostGroup6.getCheckTimeout());
    oldstate->m_remoteCache.updateResultTime(hostGroup6, time);
    time.setTime(1000);
    oldstate->m_remoteHostCache.insertRemoteEntry(hostname1, dataHostCheck5, dataHostGroup5.getCheckTTL(), dataHostGroup5.getCheckTimeout());
    oldstate->m_remoteHostCache.updateResultTime(hostname1, dataHostCheck5, time);
    time.setTime(2000);
    oldstate->m_remoteHostCache.insertRemoteEntry(hostname3, dataHostCheck3, dataHostGroup3.getCheckTTL(), dataHostGroup3.getCheckTimeout());
    oldstate->m_remoteHostCache.updateResultTime(hostname3, dataHostCheck3, time);

    CPPUNIT_ASSERT_EQUAL(1, (int)oldstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(1, (int)oldstate->m_checkList.addHostGroup(dataHostGroup2));
    CPPUNIT_ASSERT_EQUAL(1, (int)oldstate->m_checkList.addHostGroup(dataHostGroup3));
    CPPUNIT_ASSERT_EQUAL(1, (int)oldstate->m_checkList.addHostGroup(dataHostGroup4));
    CPPUNIT_ASSERT_EQUAL(1, (int)oldstate->m_checkList.addHostGroup(dataHostGroup5));
    CPPUNIT_ASSERT_EQUAL(1, (int)oldstate->m_checkList.addHostGroup(dataHostGroup6));

    HMDNSLookup lookup4(HM_DNS_TYPE_LOOKUP, false);
    HMDNSLookup lookup6(HM_DNS_TYPE_LOOKUP, true);
    {
        set<HMIPAddress> ips;
        ips.insert(address1_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address1_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup6, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup6, ips);
    }


    HMDataCheckResult result1_1;
    result1_1.m_address = address1_1;
    result1_1.m_reason = HM_REASON_DNS_TIMEOUT;
    HMDataCheckResult result1_2;
    result1_2.m_address = address1_2;
    result1_2.m_reason = HM_REASON_CONNECT_TIMEOUT;
    HMDataCheckResult result1_1_host;
    result1_1_host.m_address = address1_1;
    result1_1_host.m_reason = HM_REASON_INTERNAL_ERROR;
    HMDataCheckResult result1_1_hostgroup;
    result1_1_hostgroup.m_address = address1_1;
    result1_1_hostgroup.m_reason = HM_REASON_YNET_NOTFOUND;
    HMDataCheckResult result1_2_hostgroup;
    result1_2_hostgroup.m_address = address1_2;
    result1_2_hostgroup.m_reason = HM_REASON_CONNECT_TIMEOUT;
    result1_2_hostgroup.m_checkTime.setTime(2000);
    HMDataCheckResult result1_2_host;
    result1_2_host.m_address = address1_2;
    result1_2_host.m_reason = HM_REASON_CONNECT_TIMEOUT;
    result1_2_host.m_checkTime.setTime(200);
    HMDataCheckResult result2_1;
    result2_1.m_address = address2_1;
    result2_1.m_reason = HM_REASON_CONNECT_FAILURE;
    HMDataCheckResult result2_2;
    result2_2.m_address = address2_2;
    result2_2.m_reason = HM_REASON_RESPONSE_3XX;

    HMDataCheckResult result3_1;
    result3_1.m_address = address3_1;
    result3_1.m_reason = HM_REASON_RESPONSE_403;
    HMDataCheckResult result3_2;
    result3_2.m_address = address3_2;
    result3_2.m_reason = HM_REASON_RESPONSE_404;
    HMDataCheckResult result4_1;
    result4_1.m_address = address4_1;
    result4_1.m_reason = HM_REASON_RESPONSE_DOWN;
    HMDataCheckResult result4_2;
    result4_2.m_address = address4_2;
    result4_2.m_reason = HM_REASON_RESPONSE_5XX;


    vector<HMCheckHeader> allchecks;
    CPPUNIT_ASSERT(oldstate->m_checkList.getAllChecks(allchecks));
    CPPUNIT_ASSERT_EQUAL(6, (int)allchecks.size());
    CPPUNIT_ASSERT(allchecks[0].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[0].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[0].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[0].m_checkParams == dataCheckParams1);
    CPPUNIT_ASSERT(allchecks[1].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[1].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[1].m_hostCheck == dataHostCheck6);
    CPPUNIT_ASSERT(allchecks[1].m_checkParams == dataCheckParams6);
    CPPUNIT_ASSERT(allchecks[2].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[2].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[2].m_hostCheck == dataHostCheck5);
    CPPUNIT_ASSERT(allchecks[2].m_checkParams == dataCheckParams5);
    CPPUNIT_ASSERT(allchecks[3].m_hostname == hostname2);
    CPPUNIT_ASSERT(allchecks[3].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[3].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[3].m_checkParams == dataCheckParams2);
    CPPUNIT_ASSERT(allchecks[4].m_hostname == hostname3);
    CPPUNIT_ASSERT(allchecks[4].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[4].m_hostCheck == dataHostCheck3);
    CPPUNIT_ASSERT(allchecks[4].m_checkParams == dataCheckParams3);
    CPPUNIT_ASSERT(allchecks[5].m_hostname == hostname4);
    CPPUNIT_ASSERT(allchecks[5].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[5].m_hostCheck == dataHostCheck4);
    CPPUNIT_ASSERT(allchecks[5].m_checkParams == dataCheckParams4);

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_1);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_2);
    }

    {
        HMCheckHeader header(hostname2, address2_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_1);
    }

    {
        HMCheckHeader header(hostname2, address2_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_2);
    }

    {
        HMCheckHeader header(hostname3, address3_1, dataHostCheck3, dataCheckParams3);
        oldstate->m_checkList.updateCheck(header, result3_1);
    }

    {
        HMCheckHeader header(hostname3, address3_2, dataHostCheck3, dataCheckParams3);
        oldstate->m_checkList.updateCheck(header, result3_2);
    }

    {
        HMCheckHeader header(hostname4, address4_1, dataHostCheck4, dataCheckParams4);
        oldstate->m_checkList.updateCheck(header, result4_1);
    }

    {
        HMCheckHeader header(hostname4, address4_2, dataHostCheck4, dataCheckParams4);
        oldstate->m_checkList.updateCheck(header, result4_2);
    }

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck5, dataCheckParams5);
        oldstate->m_checkList.updateCheck(header, result1_1_host);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck5, dataCheckParams5);
        oldstate->m_checkList.updateCheck(header, result1_2_host);
    }

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck6, dataCheckParams6);
        oldstate->m_checkList.updateCheck(header, result1_1_hostgroup);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck6, dataCheckParams6);
        oldstate->m_checkList.updateCheck(header, result1_2_hostgroup);
    }





    // New state details

    HMDataHostCheck nDataHostCheck1;
    HMDataHostCheck nDataHostCheck2;
    HMDataHostCheck nDataHostCheck3;
    HMDataHostCheck nDataHostCheck4;

    HMDataCheckParams nDataCheckParams1;
    HMDataCheckParams nDataCheckParams2;
    HMDataCheckParams nDataCheckParams3;
    HMDataCheckParams nDataCheckParams4;

    HMDataHostGroup nDataHostGroup1(hostGroup1);
    nDataHostGroup1.addHost(hostname1);
    nDataHostGroup1.getHostCheck(nDataHostCheck1);
    nDataHostGroup1.getCheckParameters(nDataCheckParams1);

    HMDataHostGroup nDataHostGroup2(hostGroup2);
    nDataHostGroup2.addHost(hostname2);
    nDataHostGroup2.getHostCheck(nDataHostCheck2);
    nDataHostGroup2.getCheckParameters(nDataCheckParams2);

    HMDataHostGroup nDataHostGroup3(hostGroup3);
    nDataHostGroup3.addHost(hostname1);
    nDataHostGroup3.addHost(hostname3);
    nDataHostGroup3.setDualStack(HM_DUALSTACK_BOTH);
    nDataHostGroup3.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    nDataHostGroup3.getHostCheck(nDataHostCheck3);
    nDataHostGroup3.getCheckParameters(nDataCheckParams3);

    HMDataHostGroup nDataHostGroup4(hostGroup4);
    nDataHostGroup4.addHost(hostname1);
    nDataHostGroup4.addHost(hostname4);
    nDataHostGroup4.setDualStack(HM_DUALSTACK_BOTH);
    nDataHostGroup4.setFlowType(HM_FLOW_REMOTE_HOSTGROUP_TYPE);
    nDataHostGroup4.getHostCheck(nDataHostCheck4);
    nDataHostGroup4.getCheckParameters(nDataCheckParams4);

    shared_ptr<HMState> newstate = make_shared<HMState>();
    CPPUNIT_ASSERT(newstate);
    newstate->m_remoteCache.insertRemoteEntry(hostGroup4, nDataHostGroup4.getCheckTTL(), nDataHostGroup4.getCheckTimeout());
    newstate->m_remoteHostCache.insertRemoteEntry(hostname1, nDataHostCheck3, nDataHostGroup3.getCheckTTL(), nDataHostGroup3.getCheckTimeout());
    newstate->m_remoteHostCache.insertRemoteEntry(hostname3, nDataHostCheck3, nDataHostGroup3.getCheckTTL(), nDataHostGroup3.getCheckTimeout());

    newstate->m_datastore = make_unique<TestStorageHostGroup>(&newstate->m_hostGroups, &newstate->m_dnsCache);
    CPPUNIT_ASSERT_EQUAL(1, (int)newstate->m_checkList.addHostGroup(nDataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(1, (int)newstate->m_checkList.addHostGroup(nDataHostGroup2));
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(nDataHostGroup3));
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(nDataHostGroup4));

    newstate->restoreRunningCheckState(oldstate);
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
    }
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname3, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname3, HM_DUALSTACK_IPV4_ONLY, lookup6, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname3, HM_DUALSTACK_IPV6_ONLY, lookup4, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname4, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname4, HM_DUALSTACK_IPV4_ONLY, lookup6, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname4, HM_DUALSTACK_IPV6_ONLY, lookup4, ips));
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, nDataHostCheck1, nDataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, nDataHostCheck1, nDataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_1, nDataHostCheck2, nDataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_2, nDataHostCheck2, nDataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1_host);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2_host);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname3, address3_1, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result3_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname3, address3_2, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result3_2);
    }


    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, nDataHostCheck4, nDataCheckParams4);
        CPPUNIT_ASSERT(!newstate->m_checkList.getCheckResult(header, result));
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address2_1, nDataHostCheck4, nDataCheckParams4);
        CPPUNIT_ASSERT(!newstate->m_checkList.getCheckResult(header, result));
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname4, address4_1, nDataHostCheck4, nDataCheckParams4);
        CPPUNIT_ASSERT(!newstate->m_checkList.getCheckResult(header, result));
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname4, address4_2, nDataHostCheck4, nDataCheckParams4);
        CPPUNIT_ASSERT(!newstate->m_checkList.getCheckResult(header, result));
    }

    // hostgroupresults
    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup1, results));
        CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2);
    }

    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup2, results));
        CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[0].m_address == address2_1);
        CPPUNIT_ASSERT(results[0].m_result == result2_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[1].m_address == address2_2);
        CPPUNIT_ASSERT(results[1].m_result == result2_2);
    }

    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup3, results));
        CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1_host);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2_host);
        CPPUNIT_ASSERT(results[2].m_hostName == hostname3);
        CPPUNIT_ASSERT(results[2].m_address == address3_1);
        CPPUNIT_ASSERT(results[2].m_result == result3_1);
        CPPUNIT_ASSERT(results[3].m_hostName == hostname3);
        CPPUNIT_ASSERT(results[3].m_address == address3_2);
        CPPUNIT_ASSERT(results[3].m_result == result3_2);
    }

    {
        //The below test would contain empty results for hostgroup4
        // remote hostgroup fetch would contain data only in m_hostGroupResults
        // The the test we have only fed the result to m_checklist
        // Data should not leak from m_checklist to m_hostgroupresults map
        // The next test would contain the tests to check the copying of result in remote hostgroup type
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup4, results));
        CPPUNIT_ASSERT_EQUAL(0, (int)results.size());
    }
    {
        map<string,HMRemoteResult>::const_iterator result;
        newstate->m_remoteCache.getRemoteResult(hostGroup4, result);
        CPPUNIT_ASSERT_EQUAL(0, (int)result->second.getResultTime().getTimeSinceEpoch());
    }
    {
        map<pair<string, HMDataHostCheck>,HMRemoteResult>::const_iterator result;
        newstate->m_remoteHostCache.getRemoteResult(hostname1, nDataHostCheck3, result);
        CPPUNIT_ASSERT_EQUAL(1000, (int)result->second.getResultTime().getTimeSinceEpoch());
    }
    {
        map<pair<string, HMDataHostCheck>,HMRemoteResult>::const_iterator result;
        newstate->m_remoteHostCache.getRemoteResult(hostname3, nDataHostCheck3, result);
        CPPUNIT_ASSERT_EQUAL(2000, (int)result->second.getResultTime().getTimeSinceEpoch());
    }
}


void TESTNAME::test_allcheckResults4()
{
    // Tests to check the copying of result remote hostgroup type
    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";
    string hostGroup3 = "hostgroup3";

    string hostname1 = "test1.hm.com";
    string hostname2 = "test2.hm.com";
    string hostname3 = "test3.hm.com";
    string hostname4 = "test4.hm.com";

    HMDataHostCheck dataHostCheck1;
    HMDataHostCheck dataHostCheck2;
    HMDataHostCheck dataHostCheck3;
    HMDataCheckParams dataCheckParams1;
    HMDataCheckParams dataCheckParams2;
    HMDataCheckParams dataCheckParams3;

    HMDataHostGroup dataHostGroup1(hostGroup1);
    dataHostGroup1.addHost(hostname1);
    dataHostGroup1.addHost(hostname2);
    dataHostGroup1.getHostCheck(dataHostCheck1);
    dataHostGroup1.getCheckParameters(dataCheckParams1);

    HMDataHostGroup dataHostGroup2(hostGroup2);
    dataHostGroup2.addHost(hostname1);
    dataHostGroup2.addHost(hostname3);
    dataHostGroup2.setDualStack(HM_DUALSTACK_BOTH);
    dataHostGroup2.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    dataHostGroup2.getHostCheck(dataHostCheck2);
    dataHostGroup2.getCheckParameters(dataCheckParams2);

    HMDataHostGroup dataHostGroup3(hostGroup3);
    dataHostGroup3.addHost(hostname1);
    dataHostGroup3.addHost(hostname4);
    dataHostGroup3.setDualStack(HM_DUALSTACK_BOTH);
    dataHostGroup3.setFlowType(HM_FLOW_REMOTE_HOSTGROUP_TYPE);
    dataHostGroup3.getHostCheck(dataHostCheck3);
    dataHostGroup3.getCheckParameters(dataCheckParams3);

    HMIPAddress address1_1;
    HMIPAddress address1_2;
    HMIPAddress address2_1;
    HMIPAddress address2_2;
    HMIPAddress address3_1;
    HMIPAddress address3_2;
    HMIPAddress address4_1;
    HMIPAddress address4_2;

    address1_1.set("192.168.0.1");
    address1_2.set("fad0::1");
    address2_1.set("192.168.0.2");
    address2_2.set("fad0::2");
    address3_1.set("192.168.0.3");
    address3_2.set("fad0::3");
    address4_1.set("192.168.0.4");
    address4_2.set("fad0::4");

    shared_ptr<HMState> oldstate = make_shared<HMState>();
    CPPUNIT_ASSERT(oldstate);
    HMTimeStamp time;
    time.setTime(100);
    oldstate->m_remoteCache.insertRemoteEntry(hostGroup3, dataHostGroup3.getCheckTTL(), dataHostGroup3.getCheckTimeout());
    oldstate->m_remoteCache.updateResultTime(hostGroup3, time);
    time.setTime(1000);
    oldstate->m_remoteHostCache.insertRemoteEntry(hostname1, dataHostCheck2, dataHostGroup2.getCheckTTL(), dataHostGroup2.getCheckTimeout());
    oldstate->m_remoteHostCache.updateResultTime(hostname1, dataHostCheck2, time);
    time.setTime(2000);
    oldstate->m_remoteHostCache.insertRemoteEntry(hostname3, dataHostCheck2, dataHostGroup2.getCheckTTL(), dataHostGroup2.getCheckTimeout());
    oldstate->m_remoteHostCache.updateResultTime(hostname3, dataHostCheck2, time);

    oldstate->m_hostGroups.insert(make_pair(hostGroup1, dataHostGroup1));
    oldstate->m_hostGroups.insert(make_pair(hostGroup2, dataHostGroup2));
    oldstate->m_hostGroups.insert(make_pair(hostGroup3, dataHostGroup3));
    oldstate->m_datastore = make_unique<TestStorageHostGroup>(&oldstate->m_hostGroups, &oldstate->m_dnsCache);
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup2));
    CPPUNIT_ASSERT_EQUAL(2, (int)oldstate->m_checkList.addHostGroup(dataHostGroup3));

    HMDNSLookup lookup4(HM_DNS_TYPE_LOOKUP, false);
    HMDNSLookup lookup6(HM_DNS_TYPE_LOOKUP, true);
    {
        set<HMIPAddress> ips;
        ips.insert(address1_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address1_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname1, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname1, lookup6, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_1);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup4, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup4, ips);
    }

    {
        set<HMIPAddress> ips;
        ips.insert(address2_2);
        oldstate->m_dnsCache.insertDNSEntry(hostname2, lookup6, dataHostGroup1.getCheckTimeout(), dataHostGroup1.getCheckTTL());
        oldstate->m_dnsCache.updateDNSEntry(hostname2, lookup6, ips);
    }


    HMDataCheckResult result1_1;
    result1_1.m_address = address1_1;
    result1_1.m_reason = HM_REASON_DNS_TIMEOUT;
    HMDataCheckResult result1_2;
    result1_2.m_address = address1_2;
    result1_2.m_reason = HM_REASON_CONNECT_TIMEOUT;
    HMDataCheckResult result1_1_host;
    result1_1_host.m_address = address1_1;
    result1_1_host.m_reason = HM_REASON_INTERNAL_ERROR;
    HMDataCheckResult result1_1_hostgroup;
    result1_1_hostgroup.m_address = address1_1;
    result1_1_hostgroup.m_reason = HM_REASON_YNET_NOTFOUND;
    HMDataCheckResult result1_2_hostgroup;
    result1_2_hostgroup.m_address = address1_2;
    result1_2_hostgroup.m_reason = HM_REASON_CONNECT_TIMEOUT;
    HMDataCheckResult result1_2_host;
    result1_2_host.m_address = address1_2;
    result1_2_host.m_reason = HM_REASON_CONNECT_TIMEOUT;
    HMDataCheckResult result2_1;
    result2_1.m_address = address2_1;
    result2_1.m_reason = HM_REASON_CONNECT_FAILURE;
    HMDataCheckResult result2_2;
    result2_2.m_address = address2_2;
    result2_2.m_reason = HM_REASON_RESPONSE_3XX;

    HMDataCheckResult result3_1;
    result3_1.m_address = address3_1;
    result3_1.m_reason = HM_REASON_RESPONSE_403;
    HMDataCheckResult result3_2;
    result3_2.m_address = address3_2;
    result3_2.m_reason = HM_REASON_RESPONSE_404;
    HMDataCheckResult result4_1;
    result4_1.m_address = address4_1;
    result4_1.m_reason = HM_REASON_RESPONSE_DOWN;
    HMDataCheckResult result4_2;
    result4_2.m_address = address4_2;
    result4_2.m_reason = HM_REASON_RESPONSE_5XX;


    vector<HMCheckHeader> allchecks;
    CPPUNIT_ASSERT(oldstate->m_checkList.getAllChecks(allchecks));
    CPPUNIT_ASSERT_EQUAL(6, (int)allchecks.size());
    CPPUNIT_ASSERT(allchecks[0].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[0].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[0].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[0].m_checkParams == dataCheckParams1);
    CPPUNIT_ASSERT(allchecks[1].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[1].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[1].m_hostCheck == dataHostCheck3);
    CPPUNIT_ASSERT(allchecks[1].m_checkParams == dataCheckParams3);
    CPPUNIT_ASSERT(allchecks[2].m_hostname == hostname1);
    CPPUNIT_ASSERT(allchecks[2].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[2].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[2].m_checkParams == dataCheckParams2);
    CPPUNIT_ASSERT(allchecks[3].m_hostname == hostname2);
    CPPUNIT_ASSERT(allchecks[3].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[3].m_hostCheck == dataHostCheck1);
    CPPUNIT_ASSERT(allchecks[3].m_checkParams == dataCheckParams1);
    CPPUNIT_ASSERT(allchecks[4].m_hostname == hostname3);
    CPPUNIT_ASSERT(allchecks[4].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[4].m_hostCheck == dataHostCheck2);
    CPPUNIT_ASSERT(allchecks[4].m_checkParams == dataCheckParams2);
    CPPUNIT_ASSERT(allchecks[5].m_hostname == hostname4);
    CPPUNIT_ASSERT(allchecks[5].m_address == HMIPAddress());
    CPPUNIT_ASSERT(allchecks[5].m_hostCheck == dataHostCheck3);
    CPPUNIT_ASSERT(allchecks[5].m_checkParams == dataCheckParams3);

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_1);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result1_2);
    }

    {
        HMCheckHeader header(hostname2, address2_1, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_1);
    }

    {
        HMCheckHeader header(hostname2, address2_2, dataHostCheck1, dataCheckParams1);
        oldstate->m_checkList.updateCheck(header, result2_2);
    }

    {
        HMCheckHeader header(hostname3, address3_1, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result3_1);
    }

    {
        HMCheckHeader header(hostname3, address3_2, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result3_2);
    }

    {
        HMCheckHeader header(hostname1, address1_1, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result1_1_host);
    }

    {
        HMCheckHeader header(hostname1, address1_2, dataHostCheck2, dataCheckParams2);
        oldstate->m_checkList.updateCheck(header, result1_2_host);
    }

    vector<HMGroupCheckResult> groupresults1;
    HMGroupCheckResult group_result_1_1_1;
    group_result_1_1_1.m_hostName = hostname1;
    group_result_1_1_1.m_address = address1_1;
    group_result_1_1_1.m_result = result1_1;
    HMGroupCheckResult group_result_1_1_2;
    group_result_1_1_1.m_hostName = hostname1;
    group_result_1_1_1.m_address = address1_2;
    group_result_1_1_1.m_result = result1_2;
    HMGroupCheckResult group_result_1_2_1;
    group_result_1_2_1.m_hostName = hostname2;
    group_result_1_2_1.m_address = address2_1;
    group_result_1_2_1.m_result = result2_1;
    HMGroupCheckResult group_result_1_2_2;
    group_result_1_2_2.m_hostName = hostname1;
    group_result_1_2_2.m_address = address2_2;
    group_result_1_2_2.m_result = result2_2;
    groupresults1.push_back(group_result_1_1_1);
    groupresults1.push_back(group_result_1_1_2);
    groupresults1.push_back(group_result_1_2_1);
    groupresults1.push_back(group_result_1_2_2);

    vector<HMGroupCheckResult> groupresults2;
    HMGroupCheckResult group_result_2_1_1;
    group_result_2_1_1.m_hostName = hostname1;
    group_result_2_1_1.m_address = address1_1;
    group_result_2_1_1.m_result = result1_1_host;
    HMGroupCheckResult group_result_2_1_2;
    group_result_2_1_1.m_hostName = hostname1;
    group_result_2_1_1.m_address = address1_2;
    group_result_2_1_1.m_result = result1_2_host;
    HMGroupCheckResult group_result_2_3_1;
    group_result_2_3_1.m_hostName = hostname3;
    group_result_2_3_1.m_address = address3_1;
    group_result_2_3_1.m_result = result3_1;
    HMGroupCheckResult group_result_2_3_2;
    group_result_2_3_2.m_hostName = hostname3;
    group_result_2_3_2.m_address = address3_2;
    group_result_2_3_2.m_result = result3_2;
    groupresults2.push_back(group_result_2_1_1);
    groupresults2.push_back(group_result_2_1_2);
    groupresults2.push_back(group_result_2_3_1);
    groupresults2.push_back(group_result_2_3_2);

    vector<HMGroupCheckResult> groupresults3;
    HMGroupCheckResult group_result_3_1_1;
    group_result_3_1_1.m_hostName = hostname1;
    group_result_3_1_1.m_address = address1_1;
    group_result_3_1_1.m_result = result1_1_hostgroup;
    HMGroupCheckResult group_result_3_1_2;
    group_result_3_1_1.m_hostName = hostname1;
    group_result_3_1_1.m_address = address1_2;
    group_result_3_1_1.m_result = result1_2_hostgroup;
    HMGroupCheckResult group_result_3_4_1;
    group_result_3_4_1.m_hostName = hostname4;
    group_result_3_4_1.m_address = address4_1;
    group_result_3_4_1.m_result = result4_1;
    HMGroupCheckResult group_result_3_4_2;
    group_result_3_4_2.m_hostName = hostname4;
    group_result_3_4_2.m_address = address4_2;
    group_result_3_4_2.m_result = result4_2;
    groupresults3.push_back(group_result_3_1_1);
    groupresults3.push_back(group_result_3_1_2);
    groupresults3.push_back(group_result_3_4_1);
    groupresults3.push_back(group_result_3_4_2);

    CPPUNIT_ASSERT(oldstate->m_datastore->storeHostGroupCheckResult(hostGroup1, groupresults1));
    CPPUNIT_ASSERT(oldstate->m_datastore->storeHostGroupCheckResult(hostGroup2, groupresults2));
    CPPUNIT_ASSERT(oldstate->m_datastore->storeHostGroupCheckResult(hostGroup3, groupresults3));

    // New state details

    HMDataHostCheck nDataHostCheck1;
    HMDataHostCheck nDataHostCheck2;
    HMDataHostCheck nDataHostCheck3;

    HMDataCheckParams nDataCheckParams1;
    HMDataCheckParams nDataCheckParams2;
    HMDataCheckParams nDataCheckParams3;

    HMDataHostGroup nDataHostGroup1(hostGroup1);
    nDataHostGroup1.addHost(hostname1);
    nDataHostGroup1.addHost(hostname2);
    nDataHostGroup1.getHostCheck(nDataHostCheck1);
    nDataHostGroup1.getCheckParameters(nDataCheckParams1);

    HMDataHostGroup nDataHostGroup2(hostGroup2);
    nDataHostGroup2.addHost(hostname1);
    nDataHostGroup2.addHost(hostname3);
    nDataHostGroup2.setDualStack(HM_DUALSTACK_BOTH);
    nDataHostGroup2.setFlowType(HM_FLOW_REMOTE_HOST_TYPE);
    nDataHostGroup2.getHostCheck(nDataHostCheck2);
    nDataHostGroup2.getCheckParameters(nDataCheckParams2);

    HMDataHostGroup nDataHostGroup3(hostGroup3);
    nDataHostGroup3.addHost(hostname4);
    nDataHostGroup3.setDualStack(HM_DUALSTACK_BOTH);
    nDataHostGroup3.setFlowType(HM_FLOW_REMOTE_HOSTGROUP_TYPE);
    nDataHostGroup3.getHostCheck(nDataHostCheck3);
    nDataHostGroup3.getCheckParameters(nDataCheckParams3);

    shared_ptr<HMState> newstate = make_shared<HMState>();
    CPPUNIT_ASSERT(newstate);
    newstate->m_remoteCache.insertRemoteEntry(hostGroup3, nDataHostGroup3.getCheckTTL(), nDataHostGroup3.getCheckTimeout());
    newstate->m_remoteHostCache.insertRemoteEntry(hostname1, nDataHostCheck2, nDataHostGroup2.getCheckTTL(), nDataHostGroup2.getCheckTimeout());
    newstate->m_remoteHostCache.insertRemoteEntry(hostname3, nDataHostCheck2, nDataHostGroup2.getCheckTTL(), nDataHostGroup2.getCheckTimeout());
    newstate->m_hostGroups.insert(make_pair(hostGroup1, nDataHostGroup1));
    newstate->m_hostGroups.insert(make_pair(hostGroup2, nDataHostGroup2));
    newstate->m_hostGroups.insert(make_pair(hostGroup3, nDataHostGroup3));
    newstate->m_datastore = make_unique<TestStorageHostGroup>(&newstate->m_hostGroups, &newstate->m_dnsCache);
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(nDataHostGroup1));
    CPPUNIT_ASSERT_EQUAL(2, (int)newstate->m_checkList.addHostGroup(nDataHostGroup2));
    CPPUNIT_ASSERT_EQUAL(1, (int)newstate->m_checkList.addHostGroup(nDataHostGroup3));

    newstate->restoreRunningCheckState(oldstate);
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname1, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address1_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address1_2) != ips.end());
    }
    {
        set<HMIPAddress> ips;
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV4_ONLY, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_IPV6_ONLY, lookup6, ips));
        CPPUNIT_ASSERT_EQUAL(1, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(newstate->m_dnsCache.getAddresses(hostname2, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT_EQUAL(2, (int)ips.size());
        CPPUNIT_ASSERT(ips.find(address2_1) != ips.end());
        CPPUNIT_ASSERT(ips.find(address2_2) != ips.end());
        ips.clear();
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname3, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname3, HM_DUALSTACK_IPV4_ONLY, lookup6, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname3, HM_DUALSTACK_IPV6_ONLY, lookup4, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname4, HM_DUALSTACK_BOTH, lookup4, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname4, HM_DUALSTACK_IPV4_ONLY, lookup6, ips));
        CPPUNIT_ASSERT(!newstate->m_dnsCache.getAddresses(hostname4, HM_DUALSTACK_IPV6_ONLY, lookup4, ips));
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, nDataHostCheck1, nDataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, nDataHostCheck1, nDataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_1, nDataHostCheck1, nDataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname2, address2_2, nDataHostCheck1, nDataCheckParams1);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result2_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, nDataHostCheck2, nDataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_1_host);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_2, nDataHostCheck2, nDataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result1_2_host);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname3, address3_1, nDataHostCheck2, nDataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result3_1);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname3, address3_2, nDataHostCheck2, nDataCheckParams2);
        CPPUNIT_ASSERT(newstate->m_checkList.getCheckResult(header, result));
        CPPUNIT_ASSERT(result == result3_2);
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address1_1, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(!newstate->m_checkList.getCheckResult(header, result));
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname1, address2_1, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(!newstate->m_checkList.getCheckResult(header, result));
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname4, address4_1, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(!newstate->m_checkList.getCheckResult(header, result));
    }

    {
        HMDataCheckResult result;
        HMCheckHeader header(hostname4, address4_2, nDataHostCheck3, nDataCheckParams3);
        CPPUNIT_ASSERT(!newstate->m_checkList.getCheckResult(header, result));
    }

    // hostgroupresults
    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup1, results));
        CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2);
        CPPUNIT_ASSERT(results[2].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[2].m_address == address2_1);
        CPPUNIT_ASSERT(results[2].m_result == result2_1);
        CPPUNIT_ASSERT(results[3].m_hostName == hostname2);
        CPPUNIT_ASSERT(results[3].m_address == address2_2);
        CPPUNIT_ASSERT(results[3].m_result == result2_2);
    }

    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup2, results));
        CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[0].m_address == address1_1);
        CPPUNIT_ASSERT(results[0].m_result == result1_1_host);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname1);
        CPPUNIT_ASSERT(results[1].m_address == address1_2);
        CPPUNIT_ASSERT(results[1].m_result == result1_2_host);
        CPPUNIT_ASSERT(results[2].m_hostName == hostname3);
        CPPUNIT_ASSERT(results[2].m_address == address3_1);
        CPPUNIT_ASSERT(results[2].m_result == result3_1);
        CPPUNIT_ASSERT(results[3].m_hostName == hostname3);
        CPPUNIT_ASSERT(results[3].m_address == address3_2);
        CPPUNIT_ASSERT(results[3].m_result == result3_2);
    }

    {
        std::vector<HMGroupCheckResult> results;
        CPPUNIT_ASSERT(newstate->m_datastore->getGroupCheckResults(hostGroup3, results));
        CPPUNIT_ASSERT_EQUAL(2, (int)results.size());
        CPPUNIT_ASSERT(results[0].m_hostName == hostname4);
        CPPUNIT_ASSERT(results[0].m_address == address4_1);
        CPPUNIT_ASSERT(results[0].m_result == result4_1);
        CPPUNIT_ASSERT(results[1].m_hostName == hostname4);
        CPPUNIT_ASSERT(results[1].m_address == address4_2);
        CPPUNIT_ASSERT(results[1].m_result == result4_2);
    }
    {
        map<string,HMRemoteResult>::const_iterator result;
        newstate->m_remoteCache.getRemoteResult(hostGroup3, result);
        CPPUNIT_ASSERT_EQUAL(100, (int)result->second.getResultTime().getTimeSinceEpoch());
    }
    {
        map<pair<string, HMDataHostCheck>,HMRemoteResult>::const_iterator result;
        newstate->m_remoteHostCache.getRemoteResult(hostname1, nDataHostCheck2, result);
        CPPUNIT_ASSERT_EQUAL(1000, (int)result->second.getResultTime().getTimeSinceEpoch());
    }
    {
        map<pair<string, HMDataHostCheck>,HMRemoteResult>::const_iterator result;
        newstate->m_remoteHostCache.getRemoteResult(hostname3, nDataHostCheck2, result);
        CPPUNIT_ASSERT_EQUAL(2000, (int)result->second.getResultTime().getTimeSinceEpoch());
    }
}
