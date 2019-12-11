// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "TestHMStorageHostGroup.h"
#include "common.h"
#include "TestStorageHostGroup.h"
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(TESTNAME);

void
TESTNAME::setUp()
{
    setupCommon();
}

void
TESTNAME::tearDown()
{
    teardownCommon();
}


void
TESTNAME::test_HMStorageHostGroup_ReadOnly()
{
   HMDataHostGroupMap hostGroupMap;

    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";

    HMDataHostGroup dataHostGroup1(hostGroup1);
    HMDataHostGroup dataHostGroup2(hostGroup2);

    hostGroupMap.insert(make_pair(hostGroup1, dataHostGroup1));
    hostGroupMap.insert(make_pair(hostGroup2, dataHostGroup2));

    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams1;
    dataHostGroup1.getCheckParameters(checkParams1);
    checkParams1.addHostGroup(hostGroup1);
    HMDataCheckParams checkParams2;
    dataHostGroup1.getCheckParameters(checkParams2);
    checkParams2.addHostGroup(hostGroup2);
    HMDataCheckParams checkParamsMissing;

    string hostname1_1 = "test1_1.hm.com";
    string hostname1_2 = "test1_2.hm.com";
    string hostname2_1 = "test2_1.hm.com";
    string hostname2_2 = "test2_2.hm.com";

    HMIPAddress address1_1_1;
    HMIPAddress address1_1_2;
    HMIPAddress address1_2_1;
    HMIPAddress address1_2_2;
    HMIPAddress address2_1_1;
    HMIPAddress address2_1_2;
    HMIPAddress address2_2_1;
    HMIPAddress address2_2_2;

    HMIPAddress missingAddress;

    address1_1_1.set("192.168.0.1");
    address1_1_2.set("fad0::1");
    address1_2_1.set("192.168.0.2");
    address1_2_2.set("fad0::2");
    address2_1_1.set("192.168.0.3");
    address2_1_2.set("fad0::3");
    address2_2_1.set("192.168.0.4");
    address2_2_2.set("fad0::4");

    HMDataCheckResult result1_1_1;
    HMDataCheckResult result1_1_2;
    HMDataCheckResult result1_2_1;
    HMDataCheckResult result1_2_2;
    HMDataCheckResult result2_1_1;
    HMDataCheckResult result2_1_2;
    HMDataCheckResult result2_2_1;
    HMDataCheckResult result2_2_2;

    result1_1_1.m_numChecks = 1;
    result1_1_2.m_numChecks = 2;
    result1_2_1.m_numChecks = 3;
    result1_2_2.m_numChecks = 4;
    result2_1_1.m_numChecks = 5;
    result2_1_2.m_numChecks = 6;
    result2_2_1.m_numChecks = 7;
    result2_2_2.m_numChecks = 8;

    HMGroupCheckUpdate update1(hostGroup1, hostname1_1, address1_1_1, result1_1_1);
    HMGroupCheckUpdate update2(hostGroup1, hostname1_1, address1_1_2, result1_1_2);
    HMGroupCheckUpdate update3(hostGroup1, hostname1_2, address1_2_1, result1_2_1);
    HMGroupCheckUpdate update4(hostGroup1, hostname1_2, address1_2_2, result1_2_2);
    HMGroupCheckUpdate update5(hostGroup2, hostname2_1, address2_1_1, result2_1_1);
    HMGroupCheckUpdate update6(hostGroup2, hostname2_1, address2_1_2, result2_1_2);
    HMGroupCheckUpdate update7(hostGroup2, hostname2_2, address2_2_1, result2_2_1);
    HMGroupCheckUpdate update8(hostGroup2, hostname2_2, address2_2_2, result2_2_2);

    HMDataCheckResult testresult;
    HMDNSCache dnsCache;
    TestStorageHostGroup* storageHost = new TestStorageHostGroup(&hostGroupMap, &dnsCache);

    storageHost->test_manual_add(update1);
    storageHost->test_manual_add(update2);
    storageHost->test_manual_add(update3);
    storageHost->test_manual_add(update4);
    storageHost->test_manual_add(update5);
    storageHost->test_manual_add(update6);
    storageHost->test_manual_add(update7);
    storageHost->test_manual_add(update8);

    storageHost->openStore(true);

    // A store request should fail
    CPPUNIT_ASSERT(!storageHost->storeCheckResult(hostname1_1, address1_1_1, hostCheck, checkParams1, testresult));

    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname1_1, address1_1_1, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(testresult == result1_1_1);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname1_1, address1_1_2, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(testresult == result1_1_2);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname1_2, address1_2_1, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(testresult == result1_2_1);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname1_2, address1_2_2, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(testresult == result1_2_2);

    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname2_1, address2_1_1, hostCheck, checkParams2, testresult));
    CPPUNIT_ASSERT(testresult == result2_1_1);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname2_1, address2_1_2, hostCheck, checkParams2, testresult));
    CPPUNIT_ASSERT(testresult == result2_1_2);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname2_2, address2_2_1, hostCheck, checkParams2, testresult));
    CPPUNIT_ASSERT(testresult == result2_2_1);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname2_2, address2_2_2, hostCheck, checkParams2, testresult));
    CPPUNIT_ASSERT(testresult == result2_2_2);

    // Missing entries
    CPPUNIT_ASSERT(!storageHost->getCheckResult("missing.hm.com", address1_1_1, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(!storageHost->getCheckResult(hostname1_1, missingAddress, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(!storageHost->getCheckResult(hostname1_1, address1_1_1, hostCheck, checkParamsMissing, testresult));

    storageHost->closeStore();
    delete storageHost;
}

void
TESTNAME::test_HMStorageHostGroup_ReadWrite()
{
    HMDataHostGroupMap hostGroupMap;

    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";

    HMDataHostGroup dataHostGroup1(hostGroup1);
    HMDataHostGroup dataHostGroup2(hostGroup2);

    string hostname1_1 = "test1_1.hm.com";
    string hostname1_2 = "test1_2.hm.com";
    string hostname2_1 = "test2_1.hm.com";
    string hostname2_2 = "test2_2.hm.com";
    dataHostGroup1.setDualStack(HM_DUALSTACK_BOTH);
    dataHostGroup1.addHost(hostname1_1);
    dataHostGroup1.addHost(hostname1_2);
    dataHostGroup2.setDualStack(HM_DUALSTACK_BOTH);
    dataHostGroup2.addHost(hostname2_1);
    dataHostGroup2.addHost(hostname2_2);

    hostGroupMap.insert(make_pair(hostGroup1, dataHostGroup1));
    hostGroupMap.insert(make_pair(hostGroup2, dataHostGroup2));

    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams1;
    dataHostGroup1.getCheckParameters(checkParams1);
    checkParams1.addHostGroup(hostGroup1);
    HMDataCheckParams checkParams2;
    dataHostGroup1.getCheckParameters(checkParams2);
    checkParams2.addHostGroup(hostGroup2);
    HMDataCheckParams checkParamsMissing;


    HMIPAddress address1_1_1;
    HMIPAddress address1_1_2;
    HMIPAddress address1_2_1;
    HMIPAddress address1_2_2;
    HMIPAddress address2_1_1;
    HMIPAddress address2_1_2;
    HMIPAddress address2_2_1;
    HMIPAddress address2_2_2;

    HMIPAddress missingAddress;

    address1_1_1.set("192.168.0.1");
    address1_1_2.set("fad0::1");
    address1_2_1.set("192.168.0.2");
    address1_2_2.set("fad0::2");
    address2_1_1.set("192.168.0.3");
    address2_1_2.set("fad0::3");
    address2_2_1.set("192.168.0.4");
    address2_2_2.set("fad0::4");

    HMDataCheckResult result1_1_1;
    HMDataCheckResult result1_1_2;
    HMDataCheckResult result1_2_1;
    HMDataCheckResult result1_2_2;
    HMDataCheckResult result2_1_1;
    HMDataCheckResult result2_1_2;
    HMDataCheckResult result2_2_1;
    HMDataCheckResult result2_2_2;

    result1_1_1.m_numChecks = 1;
    result1_1_2.m_numChecks = 2;
    result1_2_1.m_numChecks = 3;
    result1_2_2.m_numChecks = 4;
    result2_1_1.m_numChecks = 5;
    result2_1_2.m_numChecks = 6;
    result2_2_1.m_numChecks = 7;
    result2_2_2.m_numChecks = 8;

    HMDataCheckResult testresult;
    vector<HMGroupCheckResult> results;
    HMDNSCache dnsCache;
    HMDNSLookup dnsHostCheck(HM_DNS_PLUGIN_ARES, false);
    HMDNSLookup dnsHostCheckv6(HM_DNS_PLUGIN_ARES, true);
    set<HMIPAddress> addresses;
    addresses.insert(address1_1_1);
    dnsCache.insertDNSEntry(hostname1_1, dnsHostCheck, 10000, 10000);
    dnsCache.updateDNSEntry(hostname1_1, dnsHostCheck, addresses);
    addresses.clear();
    addresses.insert(address1_1_2);
    dnsCache.insertDNSEntry(hostname1_1, dnsHostCheckv6, 10000, 10000);
    dnsCache.updateDNSEntry(hostname1_1, dnsHostCheckv6, addresses);

    addresses.clear();
    addresses.insert(address1_2_1);
    dnsCache.insertDNSEntry(hostname1_2, dnsHostCheck, 10000, 10000);
    dnsCache.updateDNSEntry(hostname1_2, dnsHostCheck, addresses);
    addresses.clear();
    addresses.insert(address1_2_2);
    dnsCache.insertDNSEntry(hostname1_2, dnsHostCheckv6, 10000, 10000);
    dnsCache.updateDNSEntry(hostname1_2, dnsHostCheckv6, addresses);

    addresses.clear();
    addresses.insert(address2_1_1);
    dnsCache.insertDNSEntry(hostname2_1, dnsHostCheck, 10000, 10000);
    dnsCache.updateDNSEntry(hostname2_1, dnsHostCheck, addresses);
    addresses.clear();
    addresses.insert(address2_1_2);
    dnsCache.insertDNSEntry(hostname2_1, dnsHostCheckv6, 10000, 10000);
    dnsCache.updateDNSEntry(hostname2_1, dnsHostCheckv6, addresses);

    addresses.clear();
    addresses.insert(address2_2_1);
    dnsCache.insertDNSEntry(hostname2_2, dnsHostCheck, 10000, 10000);
    dnsCache.updateDNSEntry(hostname2_2, dnsHostCheck, addresses);
    addresses.clear();
    addresses.insert(address2_2_2);
    dnsCache.insertDNSEntry(hostname2_2, dnsHostCheckv6, 10000, 10000);
    dnsCache.updateDNSEntry(hostname2_2, dnsHostCheckv6, addresses);


    TestStorageHostGroup* storageHost = new TestStorageHostGroup(&hostGroupMap, &dnsCache);

    storageHost->openStore(false);

    // A store request should fail
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname1_1, address1_1_1, hostCheck, checkParams1, result1_1_1));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname1_1, address1_1_2, hostCheck, checkParams1, result1_1_2));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname1_2, address1_2_1, hostCheck, checkParams1, result1_2_1));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname1_2, address1_2_2, hostCheck, checkParams1, result1_2_2));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname2_1, address2_1_1, hostCheck, checkParams2, result2_1_1));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname2_1, address2_1_2, hostCheck, checkParams2, result2_1_2));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname2_2, address2_2_1, hostCheck, checkParams2, result2_2_1));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname2_2, address2_2_2, hostCheck, checkParams2, result2_2_2));

    std::this_thread::sleep_for(10ms);

    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname1_1, address1_1_1, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(testresult == result1_1_1);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname1_1, address1_1_2, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(testresult == result1_1_2);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname1_2, address1_2_1, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(testresult == result1_2_1);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname1_2, address1_2_2, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(testresult == result1_2_2);

    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname2_1, address2_1_1, hostCheck, checkParams2, testresult));
    CPPUNIT_ASSERT(testresult == result2_1_1);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname2_1, address2_1_2, hostCheck, checkParams2, testresult));
    CPPUNIT_ASSERT(testresult == result2_1_2);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname2_2, address2_2_1, hostCheck, checkParams2, testresult));
    CPPUNIT_ASSERT(testresult == result2_2_1);
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname2_2, address2_2_2, hostCheck, checkParams2, testresult));
    CPPUNIT_ASSERT(testresult == result2_2_2);

    storageHost->populateHostGroup(true);
    CPPUNIT_ASSERT(storageHost->getGroupCheckResults(hostGroup1, true, false ,results));
    CPPUNIT_ASSERT_EQUAL(4, (int)results.size());
    // Note in the multimap, we don't have any strong guarentee of the oredering
    bool entry1, entry2, entry3, entry4 = false;
    for(auto it = results.begin(); it != results.end(); ++it)
    {
        if(it->m_hostName == hostname1_1)
        {
            if(it->m_address == address1_1_1)
            {
                CPPUNIT_ASSERT(it->m_result == result1_1_1);
                entry1 = true;
            }
            else if(it->m_address == address1_1_2)
            {
                CPPUNIT_ASSERT(it->m_result == result1_1_2);
                entry2 = true;
            }
        }
        else if(it->m_hostName == hostname1_2)
        {
            if(it->m_address == address1_2_1)
            {
                CPPUNIT_ASSERT(it->m_result == result1_2_1);
                entry3 = true;
            }
            else if(it->m_address == address1_2_2)
            {
                CPPUNIT_ASSERT(it->m_result == result1_2_2);
                entry4 = true;
            }
        }
    }
    CPPUNIT_ASSERT(entry1 && entry2 && entry3 && entry4);

    CPPUNIT_ASSERT(storageHost->getGroupCheckResults(hostGroup2, true, false, results));
    CPPUNIT_ASSERT(results.size() == 4);
    entry1 = entry2 = entry3 = entry4 = false;
    for(auto it = results.begin(); it != results.end(); ++it)
    {
        if(it->m_hostName == hostname2_1)
        {
            if(it->m_address == address2_1_1)
            {
                CPPUNIT_ASSERT(it->m_result == result2_1_1);
                entry1 = true;
            }
            else if(it->m_address == address2_1_2)
            {
                CPPUNIT_ASSERT(it->m_result == result2_1_2);
                entry2 = true;
            }
        }
        else if(it->m_hostName == hostname2_2)
        {
            if(it->m_address == address2_2_1)
            {
                CPPUNIT_ASSERT(it->m_result == result2_2_1);
                entry3 = true;
            }
            else if(it->m_address == address2_2_2)
            {
                CPPUNIT_ASSERT(it->m_result == result2_2_2);
                entry4 = true;
            }
        }
    }
    CPPUNIT_ASSERT(entry1 && entry2 && entry3 && entry4);

    // Missing entries
    CPPUNIT_ASSERT(!storageHost->getCheckResult("missing.hm.com", address1_1_1, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(!storageHost->getCheckResult(hostname1_1, missingAddress, hostCheck, checkParams1, testresult));
    CPPUNIT_ASSERT(!storageHost->getCheckResult(hostname1_1, address1_1_1, hostCheck, checkParamsMissing, testresult));

    CPPUNIT_ASSERT(!storageHost->getGroupCheckResults("missingGroup",true, false, results));

    storageHost->closeStore();

    delete storageHost;
}

void
TESTNAME::test_HMStorageHostGroup_Restore()
{
    HMDataHostGroupMap hostGroupMap;

    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";

    string hostname1_1 = "test1_1.hm.com";
    string hostname1_2 = "test1_2.hm.com";
    string hostname2_1 = "test2_1.hm.com";
    string hostname2_2 = "test2_2.hm.com";


    HMDataHostGroup dataHostGroup1(hostGroup1);
    HMDataHostGroup dataHostGroup2(hostGroup2);

    dataHostGroup1.addHost(hostname1_1);
    dataHostGroup1.addHost(hostname1_2);
    dataHostGroup2.addHost(hostname2_1);
    dataHostGroup2.addHost(hostname2_2);

    hostGroupMap.insert(make_pair(hostGroup1, dataHostGroup1));
    hostGroupMap.insert(make_pair(hostGroup2, dataHostGroup2));

    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams1;
    dataHostGroup1.getCheckParameters(checkParams1);
    checkParams1.addHostGroup(hostGroup1);
    HMDataCheckParams checkParams2;
    dataHostGroup1.getCheckParameters(checkParams2);
    checkParams2.addHostGroup(hostGroup2);
    HMDataCheckParams checkParamsMissing;

    HMIPAddress address1_1_1;
    HMIPAddress address1_1_2;
    HMIPAddress address1_2_1;
    HMIPAddress address1_2_2;
    HMIPAddress address2_1_1;
    HMIPAddress address2_1_2;
    HMIPAddress address2_2_1;
    HMIPAddress address2_2_2;

    address1_1_1.set("192.168.0.1");
    address1_1_2.set("fad0::1");
    address1_2_1.set("192.168.0.2");
    address1_2_2.set("fad0::2");
    address2_1_1.set("192.168.0.3");
    address2_1_2.set("fad0::3");
    address2_2_1.set("192.168.0.4");
    address2_2_2.set("fad0::4");
    set<HMIPAddress> ips1_1;
    ips1_1.insert(address1_1_1);
    ips1_1.insert(address1_1_2);
    set<HMIPAddress> ips1_2;
    ips1_2.insert(address1_2_1);
    ips1_2.insert(address1_2_2);
    set<HMIPAddress> ips2_1;
    ips2_1.insert(address2_1_1);
    ips2_1.insert(address2_1_2);
    set<HMIPAddress> ips2_2;
    ips2_2.insert(address2_2_1);
    ips2_2.insert(address2_2_2);

    HMDataCheckResult result1_1_1;
    HMDataCheckResult result1_1_2;
    HMDataCheckResult result1_2_1;
    HMDataCheckResult result1_2_2;
    HMDataCheckResult result2_1_1;
    HMDataCheckResult result2_1_2;
    HMDataCheckResult result2_2_1;
    HMDataCheckResult result2_2_2;

    result1_1_1.m_numChecks = 1;
    result1_1_2.m_numChecks = 2;
    result1_2_1.m_numChecks = 3;
    result1_2_2.m_numChecks = 4;
    result2_1_1.m_numChecks = 5;
    result2_1_2.m_numChecks = 6;
    result2_2_1.m_numChecks = 7;
    result2_2_2.m_numChecks = 8;

    HMGroupCheckUpdate update1(hostGroup1, hostname1_1, address1_1_1, result1_1_1);
    HMGroupCheckUpdate update2(hostGroup1, hostname1_1, address1_1_2, result1_1_2);
    HMGroupCheckUpdate update3(hostGroup1, hostname1_2, address1_2_1, result1_2_1);
    HMGroupCheckUpdate update4(hostGroup1, hostname1_2, address1_2_2, result1_2_2);
    HMGroupCheckUpdate update5(hostGroup2, hostname2_1, address2_1_1, result2_1_1);
    HMGroupCheckUpdate update6(hostGroup2, hostname2_1, address2_1_2, result2_1_2);
    HMGroupCheckUpdate update7(hostGroup2, hostname2_2, address2_2_1, result2_2_1);
    HMGroupCheckUpdate update8(hostGroup2, hostname2_2, address2_2_2, result2_2_2);

    HMDataCheckResult testresult;
    HMDNSCache* dnsCache = new HMDNSCache();
    TestStorageHostGroup* storageHost = new TestStorageHostGroup(&hostGroupMap, dnsCache);
    HMDataCheckList* checkList = new HMDataCheckList();
    checkList->insertCheck(hostGroup1, hostname1_1, hostCheck, checkParams1, ips1_1);
    checkList->insertCheck(hostGroup1, hostname1_2, hostCheck, checkParams1, ips1_2);
    checkList->insertCheck(hostGroup2, hostname2_1, hostCheck, checkParams2, ips2_1);
    checkList->insertCheck(hostGroup2, hostname2_2, hostCheck, checkParams2, ips2_2);

    HMAuxCache auxCache;
    storageHost->test_manual_add(update1);
    storageHost->test_manual_add(update2);
    storageHost->test_manual_add(update3);
    storageHost->test_manual_add(update4);
    storageHost->test_manual_add(update5);
    storageHost->test_manual_add(update6);
    storageHost->test_manual_add(update7);
    storageHost->test_manual_add(update8);

    storageHost->openStore(false);
    HMDNSLookup dnsHostCheck(HM_DNS_PLUGIN_ARES);
    storageHost->initResultsFromBackend(*checkList, *dnsCache, auxCache);
    set<HMIPAddress> vip_ret;
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname1_1, HM_DUALSTACK_IPV4_ONLY, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address1_1_1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname1_1, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address1_1_1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname1_1, HM_DUALSTACK_IPV6_ONLY, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address1_1_2) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname1_1, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address1_1_2) != vip_ret.end());
    vip_ret.clear();

    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname1_2, HM_DUALSTACK_IPV4_ONLY, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address1_2_1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname1_2, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address1_2_1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname1_2, HM_DUALSTACK_IPV6_ONLY, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address1_2_2) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname1_2, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address1_2_2) != vip_ret.end());
    vip_ret.clear();

    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname2_1, HM_DUALSTACK_IPV4_ONLY, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2_1_1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname2_1, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2_1_1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname2_1, HM_DUALSTACK_IPV6_ONLY, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2_1_2) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname2_1, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2_1_2) != vip_ret.end());
    vip_ret.clear();

    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname2_2, HM_DUALSTACK_IPV4_ONLY, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2_2_1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname2_2, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2_2_1) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname2_2, HM_DUALSTACK_IPV6_ONLY, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2_2_2) != vip_ret.end());
    vip_ret.clear();
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname2_2, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2_2_2) != vip_ret.end());;

    std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> results;
    CPPUNIT_ASSERT(checkList->getCheckResults(hostname1_1, hostCheck, address1_1_1, results));
    CPPUNIT_ASSERT(results.size() == 1);
    CPPUNIT_ASSERT(results[0].first == checkParams1);
    CPPUNIT_ASSERT(results[0].second == result1_1_1);

    CPPUNIT_ASSERT(checkList->getCheckResults(hostname1_1, hostCheck, address1_1_2, results));
    CPPUNIT_ASSERT(results.size() == 1);
    CPPUNIT_ASSERT(results[0].first == checkParams1);
    CPPUNIT_ASSERT(results[0].second == result1_1_2);

    CPPUNIT_ASSERT(checkList->getCheckResults(hostname1_2, hostCheck, address1_2_1, results));
    CPPUNIT_ASSERT(results.size() == 1);
    CPPUNIT_ASSERT(results[0].first == checkParams1);
    CPPUNIT_ASSERT(results[0].second == result1_2_1);

    CPPUNIT_ASSERT(checkList->getCheckResults(hostname1_2, hostCheck, address1_2_2, results));
    CPPUNIT_ASSERT(results.size() == 1);
    CPPUNIT_ASSERT(results[0].first == checkParams1);
    CPPUNIT_ASSERT(results[0].second == result1_2_2);

    CPPUNIT_ASSERT(checkList->getCheckResults(hostname2_1, hostCheck, address2_1_1, results));
    CPPUNIT_ASSERT(results.size() == 1);
    CPPUNIT_ASSERT(results[0].first == checkParams1);
    CPPUNIT_ASSERT(results[0].second == result2_1_1);

    CPPUNIT_ASSERT(checkList->getCheckResults(hostname2_1, hostCheck, address2_1_2, results));
    CPPUNIT_ASSERT(results.size() == 1);
    CPPUNIT_ASSERT(results[0].first == checkParams1);
    CPPUNIT_ASSERT(results[0].second == result2_1_2);

    CPPUNIT_ASSERT(checkList->getCheckResults(hostname2_2, hostCheck, address2_2_1, results));
    CPPUNIT_ASSERT(results.size() == 1);
    CPPUNIT_ASSERT(results[0].first == checkParams1);
    CPPUNIT_ASSERT(results[0].second == result2_2_1);

    CPPUNIT_ASSERT(checkList->getCheckResults(hostname2_2, hostCheck, address2_2_2, results));
    CPPUNIT_ASSERT(results.size() == 1);
    CPPUNIT_ASSERT(results[0].first == checkParams1);
    CPPUNIT_ASSERT(results[0].second == result2_2_2);

    storageHost->closeStore();
    delete storageHost;
    delete checkList;
    delete dnsCache;

    // Test an empty hostGroup list
    HMDataHostGroupMap emptyHostGroupMap;
    storageHost = new TestStorageHostGroup(&emptyHostGroupMap, dnsCache);
    checkList = new HMDataCheckList();
    dnsCache = new HMDNSCache();

    storageHost->test_manual_add(update1);
    storageHost->test_manual_add(update2);
    storageHost->test_manual_add(update3);
    storageHost->test_manual_add(update4);
    storageHost->test_manual_add(update5);
    storageHost->test_manual_add(update6);
    storageHost->test_manual_add(update7);
    storageHost->test_manual_add(update8);

    storageHost->openStore(false);

    // TODO fix the initResultsFromBackend
    //CPPUNIT_ASSERT(storageHost->restoreResults(*checkList, *dnsCache));

    // Now nothing should be restored
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname1_1, HM_DUALSTACK_IPV4_ONLY, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname1_1, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname1_1, HM_DUALSTACK_IPV6_ONLY, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname1_1, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname1_2, HM_DUALSTACK_IPV4_ONLY, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname1_2, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname1_2, HM_DUALSTACK_IPV6_ONLY, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname1_2, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname2_1, HM_DUALSTACK_IPV4_ONLY, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname2_1, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname2_1, HM_DUALSTACK_IPV6_ONLY, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname2_1, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname2_2, HM_DUALSTACK_IPV4_ONLY, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname2_2, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname2_2, HM_DUALSTACK_IPV6_ONLY, dnsHostCheck, vip_ret));
    vip_ret.clear();
    CPPUNIT_ASSERT(!dnsCache->getAddresses(hostname2_2, HM_DUALSTACK_BOTH, dnsHostCheck, vip_ret));

    CPPUNIT_ASSERT(!checkList->getCheckResults(hostname1_1, hostCheck, address1_1_1, results));
    CPPUNIT_ASSERT(!checkList->getCheckResults(hostname1_1, hostCheck, address1_1_2, results));
    CPPUNIT_ASSERT(!checkList->getCheckResults(hostname1_2, hostCheck, address1_2_1, results));
    CPPUNIT_ASSERT(!checkList->getCheckResults(hostname1_2, hostCheck, address1_2_2, results));
    CPPUNIT_ASSERT(!checkList->getCheckResults(hostname2_1, hostCheck, address2_1_1, results));
    CPPUNIT_ASSERT(!checkList->getCheckResults(hostname2_1, hostCheck, address2_1_2, results));
    CPPUNIT_ASSERT(!checkList->getCheckResults(hostname2_2, hostCheck, address2_2_1, results));
    CPPUNIT_ASSERT(!checkList->getCheckResults(hostname2_2, hostCheck, address2_2_2, results));

    storageHost->closeStore();
    delete storageHost;
    delete checkList;
    delete dnsCache;

    storageHost = new TestStorageHostGroup(&hostGroupMap, dnsCache);
    checkList = new HMDataCheckList();
    dnsCache = new HMDNSCache();

    storageHost->test_manual_add(update1);
    storageHost->test_manual_add(update2);
    storageHost->test_manual_add(update3);
    storageHost->test_manual_add(update4);
    storageHost->test_manual_add(update5);
    storageHost->test_manual_add(update6);
    storageHost->test_manual_add(update7);
    storageHost->test_manual_add(update8);

    storageHost->openStore(true);

    // TODO fix initResultsFromBackend
    //CPPUNIT_ASSERT(!storageHost->restoreResults(*checkList, *dnsCache));
    storageHost->closeStore();
    delete storageHost;
    delete checkList;
    delete dnsCache;
}

void
TESTNAME::test_HMStorageHostGroup_InternalFunctions()
{
    HMDataHostGroupMap hostGroupMap;

    string hostGroup1 = "hostgroup1";
    string hostGroup2 = "hostgroup2";

    HMDataHostGroup dataHostGroup1(hostGroup1);
    HMDataHostGroup dataHostGroup2(hostGroup2);

    hostGroupMap.insert(make_pair(hostGroup1, dataHostGroup1));
    hostGroupMap.insert(make_pair(hostGroup2, dataHostGroup2));

    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams1;
    dataHostGroup1.getCheckParameters(checkParams1);
    checkParams1.addHostGroup(hostGroup1);
    HMDataCheckParams checkParams2;
    dataHostGroup1.getCheckParameters(checkParams2);
    checkParams2.addHostGroup(hostGroup2);
    HMDataCheckParams checkParamsMissing;

    string hostname1_1 = "test1_1.hm.com";
    string hostname1_2 = "test1_2.hm.com";
    string hostname2_1 = "test2_1.hm.com";
    string hostname2_2 = "test2_2.hm.com";

    HMIPAddress address1_1_1;
    HMIPAddress address1_1_2;
    HMIPAddress address1_2_1;
    HMIPAddress address1_2_2;
    HMIPAddress address2_1_1;
    HMIPAddress address2_1_2;
    HMIPAddress address2_2_1;
    HMIPAddress address2_2_2;

    HMIPAddress testAddress;

    address1_1_1.set("192.168.0.1");
    address1_1_2.set("fad0::1");
    address1_2_1.set("192.168.0.2");
    address1_2_2.set("fad0::2");
    address2_1_1.set("192.168.0.3");
    address2_1_2.set("fad0::3");
    address2_2_1.set("192.168.0.4");
    address2_2_2.set("fad0::4");

    HMDataCheckResult result1_1_1;
    HMDataCheckResult result1_1_2;
    HMDataCheckResult result1_2_1;
    HMDataCheckResult result1_2_2;
    HMDataCheckResult result2_1_1;
    HMDataCheckResult result2_1_2;
    HMDataCheckResult result2_2_1;
    HMDataCheckResult result2_2_2;

    result1_1_1.m_numChecks = 1;
    result1_1_2.m_numChecks = 2;
    result1_2_1.m_numChecks = 3;
    result1_2_2.m_numChecks = 4;
    result2_1_1.m_numChecks = 5;
    result2_1_2.m_numChecks = 6;
    result2_2_1.m_numChecks = 7;
    result2_2_2.m_numChecks = 8;


    HMGroupCheckUpdate update1(hostGroup1, hostname1_1, address1_1_1, result1_1_1);
    HMGroupCheckUpdate update2(hostGroup1, hostname1_1, address1_1_2, result1_1_2);
    HMGroupCheckUpdate update3(hostGroup1, hostname1_2, address1_2_1, result1_2_1);
    HMGroupCheckUpdate update4(hostGroup1, hostname1_2, address1_2_2, result1_2_2);
    HMGroupCheckUpdate update5(hostGroup2, hostname2_1, address2_1_1, result2_1_1);
    HMGroupCheckUpdate update6(hostGroup2, hostname2_1, address2_1_2, result2_1_2);
    HMGroupCheckUpdate update7(hostGroup2, hostname2_2, address2_2_1, result2_2_1);
    HMGroupCheckUpdate update8(hostGroup2, hostname2_2, address2_2_2, result2_2_2);

    HMDataCheckResult testresult;
    HMDNSCache dnsCache;
    TestStorageHostGroup* storageHost = new TestStorageHostGroup(&hostGroupMap, &dnsCache);

    storageHost->test_manual_add(update1);
    storageHost->test_manual_add(update2);
    storageHost->test_manual_add(update3);
    storageHost->test_manual_add(update4);
    storageHost->test_manual_add(update5);
    storageHost->test_manual_add(update6);
    storageHost->test_manual_add(update7);
    storageHost->test_manual_add(update8);

    std::vector<HMGroupCheckResult> results;
    HMDataHostGroup updateHostGroupTest1(hostGroup1);
    HMDataHostGroup updateHostGroupTest2(hostGroup2);
    const HMDataHostGroup* hostGroupTest1;
    const HMDataHostGroup* hostGroupTest2; 
    string missingGroup = "missingGroup";


    // Test retrieving group names
    CPPUNIT_ASSERT(!storageHost->test_getInternalHostGroupInfo(missingGroup, hostGroupTest1));
    CPPUNIT_ASSERT(storageHost->test_getInternalHostGroupInfo(hostGroup1, hostGroupTest1));
    CPPUNIT_ASSERT(hostGroupTest1->getName() == hostGroup1);
    CPPUNIT_ASSERT(storageHost->test_getInternalHostGroupInfo(hostGroup2, hostGroupTest2));
    CPPUNIT_ASSERT(hostGroupTest2->getName() == hostGroup2);

    updateHostGroupTest1 = *hostGroupTest1;
    updateHostGroupTest2 = *hostGroupTest2;

    storageHost->populateHostGroup(false);
    // Test retrieving the health check results
    CPPUNIT_ASSERT(storageHost->test_getInternalHostGroupResults(hostGroup1, results) == 4);
    CPPUNIT_ASSERT(results.size() == 4);
    bool entry1, entry2, entry3, entry4 = false;
    for(auto it = results.begin(); it != results.end(); ++it)
    {
        if(it->m_hostName == hostname1_1)
        {
            if(it->m_address == address1_1_1)
            {
                CPPUNIT_ASSERT(it->m_result == result1_1_1);
                entry1 = true;
            }
            else if(it->m_address == address1_1_2)
            {
                CPPUNIT_ASSERT(it->m_result == result1_1_2);
                entry2 = true;
            }
        }
        else if(it->m_hostName == hostname1_2)
        {
            if(it->m_address == address1_2_1)
            {
                CPPUNIT_ASSERT(it->m_result == result1_2_1);
                entry3 = true;
            }
            else if(it->m_address == address1_2_2)
            {
                CPPUNIT_ASSERT(it->m_result == result1_2_2);
                entry4 = true;
            }
        }
    }
    CPPUNIT_ASSERT(entry1 && entry2 && entry3 && entry4);

    // Check for missing health check results
    CPPUNIT_ASSERT(storageHost->test_getInternalHostGroupResults(missingGroup, results) == 0);
    CPPUNIT_ASSERT(results.size() == 0);

    CPPUNIT_ASSERT(storageHost->test_getInternalHostGroupResults(hostGroup2, results) == 4);
    CPPUNIT_ASSERT(results.size() == 4);
    entry1 = entry2 = entry3 = entry4 = false;
    for(auto it = results.begin(); it != results.end(); ++it)
    {
        if(it->m_hostName == hostname2_1)
        {
            if(it->m_address == address2_1_1)
            {
                CPPUNIT_ASSERT(it->m_result == result2_1_1);
                entry1 = true;
            }
            else if(it->m_address == address2_1_2)
            {
                CPPUNIT_ASSERT(it->m_result == result2_1_2);
                entry2 = true;
            }
        }
        else if(it->m_hostName == hostname2_2)
        {
            if(it->m_address == address2_2_1)
            {
                CPPUNIT_ASSERT(it->m_result == result2_2_1);
                entry3 = true;
            }
            else if(it->m_address == address2_2_2)
            {
                CPPUNIT_ASSERT(it->m_result == result2_2_2);
                entry4 = true;
            }
        }
    }
    CPPUNIT_ASSERT(entry1 && entry2 && entry3 && entry4);


    CPPUNIT_ASSERT(storageHost->test_getInternalHostGroupResults(hostGroup1, results) == 4);
    CPPUNIT_ASSERT(results.size() == 4);

    for(auto it = results.begin(); it != results.end(); ++it)
    {
        if(it->m_hostName == hostname2_1)
        {
            if(it->m_address == address2_1_1)
            {
                CPPUNIT_ASSERT(it->m_result == result2_1_1);
                entry1 = true;
            }
            else if(it->m_address == address2_1_2)
            {
                CPPUNIT_ASSERT(it->m_result == result2_1_2);
                entry2 = true;
            }
        }
        else if(it->m_hostName == hostname2_2)
        {
            if(it->m_address == address2_2_1)
            {
                CPPUNIT_ASSERT(it->m_result == result2_2_1);
                entry3 = true;
            }
            else if(it->m_address == address2_2_2)
            {
                CPPUNIT_ASSERT(it->m_result == result2_2_2);
                entry4 = true;
            }
        }
    }
    CPPUNIT_ASSERT(entry1 && entry2 && entry3 && entry4);
    delete storageHost;
}

