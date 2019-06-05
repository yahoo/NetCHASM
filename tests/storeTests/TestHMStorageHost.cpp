// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#include "common.h"
#include "TestHMStorageHost.h"
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

void
TESTNAME::test_HMStorageHost_ReadOnly()
{
    string hostname = "test.hm.com";
    HMIPAddress address;
    address.set("192.168.0.1");
    HMIPAddress address2;
    address2.set("fad0::1");

    HMConfigInfo configInfo;
    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams;
    HMDataCheckResult result;
    result.m_numTimeouts = 10;
    HMDataCheckResult testresult;
    vector<HMIPAddress> dnsResult;
    HMDataHostGroupMap groupMap;
    HMCheckData checkData(hostname, address, hostCheck, checkParams, result);

    TestStorageHost* storageHost = new TestStorageHost(&groupMap);
    storageHost->openStore(true);

    storageHost->dns.insert(make_pair(hostname, address));
    storageHost->dns.insert(make_pair(hostname, address2));
    storageHost->results.insert(make_pair(hostname, checkData));

    // A store request should fail
    CPPUNIT_ASSERT(!storageHost->storeCheckResult(hostname, address, hostCheck, checkParams, result));
    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname, address, hostCheck, checkParams, testresult));
    CPPUNIT_ASSERT(result == testresult);

    CPPUNIT_ASSERT(!storageHost->getCheckResult("missing.hm.com", address, hostCheck, checkParams, testresult));
    storageHost->closeStore();
    delete storageHost;
}

void
TESTNAME::test_HMStorageHost_ReadWrite()
{
    string hostname = "test.hm.com";
    HMIPAddress address;
    address.set("192.168.0.1");
    HMIPAddress address2;
    address2.set("fad0::1");

    HMConfigInfo configInfo;
    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams;
    HMDataCheckResult result;
    result.m_numTimeouts = 10;
    HMDataCheckResult testresult;
    vector<HMIPAddress> dnsResult;

    HMCheckData checkData(hostname, address, hostCheck, checkParams, result);
    HMDataHostGroupMap groupMap;
    TestStorageHost* storageHost = new TestStorageHost(&groupMap);
    storageHost->openStore(false);

    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname, address2, hostCheck, checkParams, result));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname, address, hostCheck, checkParams, result));

    std::this_thread::sleep_for(10ms);

    CPPUNIT_ASSERT(storageHost->getCheckResult(hostname, address, hostCheck, checkParams, testresult));
    CPPUNIT_ASSERT(result == testresult);

    CPPUNIT_ASSERT(!storageHost->getCheckResult("missing.hm.com", address, hostCheck, checkParams, testresult));
    storageHost->closeStore();
    delete storageHost;
}

void
TESTNAME::test_HMStorageHost_Restore()
{
    string hostname = "test.hm.com";
    HMIPAddress address;
    address.set("192.168.0.1");
    HMIPAddress address2;
    address2.set("fad0::1");
    set<HMIPAddress> ips, vip_ret;
    ips.insert(address);

    set<HMIPAddress> ips2;
    ips2.insert(address2);

    HMIPAddress testAddress;

    HMConfigInfo configInfo;
    HMDataHostCheck hostCheck;
    HMDataCheckParams checkParams;
    HMDataCheckResult result;
    result.m_numTimeouts = 10;

    HMDataCheckList* checkList = new HMDataCheckList();
    HMDNSCache* dnsCache = new HMDNSCache();

    checkList->insertCheck("group1", hostname, hostCheck, checkParams, ips);

    HMCheckData checkData(hostname, address, hostCheck, checkParams, result);
    HMDataHostGroupMap groupMap;
    TestStorageHost* storageHost = new TestStorageHost(&groupMap);
    storageHost->openStore(false);

    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname, address, hostCheck, checkParams, result));
    CPPUNIT_ASSERT(storageHost->storeCheckResult(hostname, address2, hostCheck, checkParams, result));

    std::this_thread::sleep_for(10ms);

    // TODO fix function
    //CPPUNIT_ASSERT(storageHost->restoreResults(*checkList, *dnsCache));

    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname, HM_DUALSTACK_IPV4_ONLY, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address) != vip_ret.end());
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address) != vip_ret.end());
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname, HM_DUALSTACK_IPV6_ONLY, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2) != vip_ret.end());
    CPPUNIT_ASSERT(dnsCache->getAddresses(hostname, HM_DUALSTACK_BOTH, vip_ret));
    CPPUNIT_ASSERT(vip_ret.find(address2) != vip_ret.end());

    std::vector<std::pair<HMDataCheckParams, HMDataCheckResult>> results;

    storageHost->closeStore();
    delete storageHost;
    delete checkList;
    delete dnsCache;
}

